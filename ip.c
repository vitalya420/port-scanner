#include "ip.h"

struct ip_header create_ip_header(
    uint16_t total_length,
    uint8_t protocol,
    const char* src_ip,
    const char* dst_ip
) {
    struct ip_header iph = {0};
    
    iph.version_ihl = 0x45;
    iph.tos = 0;
    iph.total_length = htons(total_length);
    iph.id = htons(0x001);
    iph.frag_offset = 0;
    iph.ttl = 64;
    iph.protocol = protocol;
    iph.checksum = 0;
    
    inet_pton(AF_INET, src_ip, &iph.src_addr);
    inet_pton(AF_INET, dst_ip, &iph.dst_addr);
    
    iph.checksum = calculate_ip_checksum(&iph, sizeof(struct ip_header));
    
    return iph;
}

uint16_t calculate_ip_checksum(void* vdata, size_t length) {
    uint16_t* data = (uint16_t*)vdata;
    uint32_t sum = 0;
    
    while (length > 1) {
        sum += *data++;
        length -= 2;
    }
    
    if (length > 0) {
        sum += *(uint8_t*)data;
    }
    
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    
    return ~sum;
}