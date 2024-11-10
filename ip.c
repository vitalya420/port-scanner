#include "ip.h"

struct ip_header create_ip_header(
    uint16_t total_length,
    uint8_t protocol,
    const char* src_ip,
    const char* dst_ip
) {
    struct ip_header iph = {0};
    
    // IPv4, header length 5 * 32bit words = 20 bytes
    iph.version_ihl = 0x45;    // Version 4, IHL 5
    iph.tos = 0;
    iph.total_length = htons(total_length);
    iph.id = htons(rand() & 0xFFFF);  // Random ID
    iph.frag_offset = 0;
    iph.ttl = 64;             // Standard TTL value
    iph.protocol = protocol;
    iph.checksum = 0;         // Will be calculated later
    
    // Convert IP addresses from string to binary
    inet_pton(AF_INET, src_ip, &iph.src_addr);
    inet_pton(AF_INET, dst_ip, &iph.dst_addr);
    
    // Calculate checksum
    iph.checksum = calculate_ip_checksum(&iph, sizeof(struct ip_header));
    
    return iph;
}

uint16_t calculate_ip_checksum(void* vdata, size_t length) {
    // Cast the data to 16-bit unsigned integers
    uint16_t* data = (uint16_t*)vdata;
    uint32_t sum = 0;
    
    // Sum up all 16-bit words
    while (length > 1) {
        sum += *data++;
        length -= 2;
    }
    
    // Add left-over byte, if any
    if (length > 0) {
        sum += *(uint8_t*)data;
    }
    
    // Add upper 16 bits to lower 16 bits
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    // Take one's complement
    return ~sum;
}