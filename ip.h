#ifndef IP_H
#define IP_H

#include <stdint.h>
#include <arpa/inet.h>
#include <string.h>

// Standard IP header structure
struct ip_header {
    uint8_t version_ihl;        // Version (4 bits) + Internet header length (4 bits)
    uint8_t tos;               // Type of service
    uint16_t total_length;     // Total length
    uint16_t id;               // Identification
    uint16_t frag_offset;      // Fragment offset
    uint8_t ttl;               // Time to live
    uint8_t protocol;          // Protocol
    uint16_t checksum;         // Header checksum
    uint32_t src_addr;         // Source address
    uint32_t dst_addr;         // Destination address
};

// IP packet creation function
struct ip_header create_ip_header(
    uint16_t total_length,
    uint8_t protocol,
    const char* src_ip,
    const char* dst_ip
);

// Calculate IP checksum
uint16_t calculate_ip_checksum(void* vdata, size_t length);

#endif // IP_H