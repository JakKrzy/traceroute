// Jakub Krzyżowski 331931

#include "Receiver.hpp"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/ip.h>
#include <poll.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <set>

bool Receiver::receive(uint32_t sn, tp sent_tp)
{
    struct pollfd ps;
	ps.fd = sock_fd;
	ps.events = POLLIN;
	ps.revents = 0;

	uint32_t wait_for_ms = 1000;

	struct sockaddr_in sender;
	socklen_t sender_len = sizeof(sender);
	uint8_t buffer[IP_MAXPACKET];

	int found = 0;
	auto poll_start = system_clock::now();
	set<string> found_addresses;
	int latency_sum = 0;
	auto time_since_start = [&poll_start](){ return chrono::duration_cast<ms>(system_clock::now() - poll_start); };
		
	do {
		int ready = poll(&ps, 1, wait_for_ms);
		wait_for_ms = 1000 - time_since_start().count();

		if (ready < 0) {
			throw "Polling error: " + string(strerror(errno));
		}

		if (ready == 0) { break; }

		if (ps.revents != POLLIN) { continue; }

		ssize_t packet_len = recvfrom(
			sock_fd,
			buffer,
			IP_MAXPACKET,
			MSG_DONTWAIT,
			(struct sockaddr*)&sender,
			&sender_len);

		if (packet_len < 0) {
			throw "recvfrom error: " + string(strerror(errno));
		}

		char sender_ip_str[20];
        if (inet_ntop(AF_INET, &(sender.sin_addr), sender_ip_str, sizeof(sender_ip_str)) == NULL) {
            throw "inet_ntop error: " + string(strerror(errno));
        }

		auto update_stats = [&found, &sender_ip_str, &found_addresses, &latency_sum, &sent_tp](){
			found++;
			char sender_ip[20];
			strcpy(sender_ip, sender_ip_str);
			found_addresses.insert(string(sender_ip));
			latency_sum += chrono::duration_cast<ms>(system_clock::now() - sent_tp).count();
		};

		struct icmp* icmp_header = get_icmp_h_from_ip_h(buffer);
		if (icmp_header->icmp_type == ICMP_TIMXCEED) {
			struct icmp* ins_icmp_header = get_icmp_h_from_ip_h(buffer + get_ip_h_len(buffer) + icmp_header_len);
			if (validate_icmp_header(ins_icmp_header, sn)) {
				update_stats();
			}
		} else if (icmp_header->icmp_type == ICMP_ECHOREPLY) {
			if (validate_icmp_header(icmp_header, sn) and dest_addr == string(sender_ip_str)) {
				update_stats();
			}
		}
	} while (found < 3 and time_since_start() < ms(1000));
		
	if (found == 0) {
		cout << "*\n";
		return false;
	}

	for (auto& addr : found_addresses) {
		cout << addr << " ";
	}

    if (found == 3) {
    	double avg_latency = (double) latency_sum / 3.0;
    	cout << avg_latency << " ms\n";
    } else {
		cout << "???\n";
	}

	return found_addresses.contains(dest_addr);
}

bool Receiver::validate_icmp_header(const struct icmp* icmp_header, uint32_t first_sn)
{
	uint16_t sn = ntohs(icmp_header->icmp_hun.ih_idseq.icd_seq);
	uint16_t _id = ntohs(icmp_header->icmp_hun.ih_idseq.icd_id);
	return _id == id and first_sn <= sn and sn < first_sn + 3;
}

ssize_t Receiver::get_ip_h_len(const uint8_t* buffer)
{
    struct ip* ip_header = (struct ip*) buffer;
	return 4 * (ssize_t)(ip_header->ip_hl);
}

struct icmp* Receiver::get_icmp_h_from_ip_h(const uint8_t* buffer)
{
	return (struct icmp*)(buffer + get_ip_h_len(buffer));
}
