// Jakub Krzyżowski 331931

#include "Sender.hpp"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <utility>

pair<uint32_t, tp> Sender::sendPings(const uint16_t ttl)
{
    uint32_t sn_to_return = curr_sn;
    struct sockaddr_in recipient;
    bzero(&recipient, sizeof(recipient));
    recipient.sin_family = AF_INET;
    if (int res = inet_pton(AF_INET, dest_addr, &recipient.sin_addr) == 0) {
        throw "Invalid destination address";
    } else if (res == -1) {
        throw "inet_pton error: " + string(strerror(errno));
    }

    if (setsockopt(sock_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int)) == -1) {
        throw "setsockopt error: " + string(strerror(errno));
    }

    for (auto i{0u}; i < 3; i++) {
        struct icmp header = get_icmp_header();
        curr_sn++;

        ssize_t bytes_sent = sendto(
            sock_fd,
            &header,
            sizeof(header),
            0,
            (struct sockaddr*) &recipient,
            sizeof(recipient));

        if (bytes_sent <= 0)
            throw "Error sending packet: " + string(strerror(errno));
    }
    
    return make_pair(sn_to_return, chrono::system_clock::now());
}

uint16_t Sender::compute_icmp_checksum (const uint16_t *buff, int length)
{
    const uint16_t* ptr = buff;
    uint32_t sum = 0;
    assert(length % 2 == 0);
    for (; length > 0; length -= 2)
        sum += *ptr++;
    sum = (sum >> 16U) + (sum & 0xffffU);
    return (uint16_t)(~(sum + (sum >> 16U)));
}

struct icmp Sender::get_icmp_header()
{
    struct icmp header;
    header.icmp_type = ICMP_ECHO;
    header.icmp_code = 0;
    header.icmp_hun.ih_idseq.icd_id = htons(id);
    header.icmp_hun.ih_idseq.icd_seq = htons(curr_sn);
    header.icmp_cksum = 0;
    header.icmp_cksum = compute_icmp_checksum((uint16_t*) &header, sizeof(header));
    return header;
}
