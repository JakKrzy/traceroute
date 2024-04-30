// Jakub Krzyżowski 331931

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <regex>
#include <unistd.h>
#include "Receiver.hpp"
#include "Sender.hpp"

using namespace std;

uint16_t get_id()
{
	const int n_of_bits = 16;
	pid_t pid = getpid();
	uint32_t mask = (1 << n_of_bits) - 1;
	return pid & mask;
}

bool validate_args(int argc, const char* argv[])
{
	if (argc != 2) {
		cout << "Usage: ./Traceroute destination_IPv4_address\n";
		return false;
	}
	regex ipv4_addr_regex = regex(R"(^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$)");
	if (not regex_match(argv[1], ipv4_addr_regex)) {
		cerr << "Invalid IPv4 address\n";
		return false;
	}
	return true;
}

int main(int argc, const char* argv[])
{
	if (not validate_args(argc, argv)) { return -1; }
	const char* dest_ip_addr = argv[1];

	int sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sock_fd < 0) {
		cerr << "socket error: %s\n" <<  strerror(errno); 
		return EXIT_FAILURE;
	}

	uint16_t id = get_id();

	Receiver receiver(sock_fd, id, dest_ip_addr);
	Sender sender(sock_fd, dest_ip_addr, id);

	try {
		bool found = false;
		int ttl = 1;
		do {
			auto [sn, sent_tp] = sender.sendPings(ttl);
			found = receiver.receive(sn, sent_tp);
			ttl++;
		} while (not found);
	} catch (const char* e) {
		cerr << e << '\n';
		return -1;
	}

	return 0;
}
