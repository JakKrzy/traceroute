// Jakub Krzyżowski 331931

#pragma once

#include <chrono>
#include <iostream>

using namespace std;
using chrono::system_clock;
using ms = chrono::milliseconds;
using tp = system_clock::time_point;

class Receiver
{
public:
    Receiver(int& sock_fd, const uint16_t id, string dest_addr) : sock_fd(sock_fd), id(id), dest_addr(dest_addr) {}

    bool receive(uint32_t sn, tp sent_tp);

private:
	bool validate_icmp_header(const struct icmp* icmp_header, uint32_t first_sn);
	ssize_t get_ip_h_len(const uint8_t* buffer);
	struct icmp* get_icmp_h_from_ip_h(const uint8_t* buffer);

    int& sock_fd;
	uint16_t id;
	string dest_addr;
	const ssize_t icmp_header_len{8};
};
