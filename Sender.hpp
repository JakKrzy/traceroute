// Jakub Krzyżowski 331931

#pragma once

#include <chrono>
#include <iostream>

using namespace std;
using chrono::time_point;
using chrono::system_clock;
using tp = system_clock::time_point;

class Sender
{
public:
    Sender(int& sock_fd, const char* dest_addr, const uint16_t id)
        : id(id), sock_fd(sock_fd), dest_addr(dest_addr) {}

    pair<uint32_t, tp> sendPings(const uint16_t ttl);

private:
    uint16_t compute_icmp_checksum (const uint16_t *buff, int length);
    struct icmp get_icmp_header();

    const uint16_t id;
    int& sock_fd;
    const char* dest_addr;
    uint32_t curr_sn{1u};
};
