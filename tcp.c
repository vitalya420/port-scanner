#include "tcp.h"
#include <string.h>

struct tcp_header create_tcp_header(
    uint16_t src_port,
    uint16_t dst_port,
    uint32_t seq_num,
    uint32_t ack_num,
    uint8_t flags
) {
    struct tcp_header tcph = {0};
    
    tcph.src_port = htons(src_port);
    tcph.dst_port = htons(dst_port);
    tcph.seq_num = htonl(seq_num);
    tcph.ack_num = htonl(ack_num);
    tcph.data_offset = (5 << 4);  // 5 * 32bit words = 20 bytes
    tcph.flags = flags;
    tcph.window = htons(65535);   // Maximum window size
    tcph.checksum = 0;            // Will be calculated later
    tcph.urgent_ptr = 0;
    
    return tcph;
}

uint16_t calculate_tcp_checksum(
    struct tcp_header* tcp,
    struct ip_header* ip,
    uint8_t* payload,
    size_t payload_len
) {
    struct tcp_pseudo_header pseudo_header = {0};
    pseudo_header.src_addr = ip->src_addr;
    pseudo_header.dst_addr = ip->dst_addr;
    pseudo_header.zero = 0;
    pseudo_header.protocol = IPPROTO_TCP;
    pseudo_header.tcp_length = htons(sizeof(struct tcp_header) + payload_len);
    
    size_t total_len = sizeof(struct tcp_pseudo_header) + 
                       sizeof(struct tcp_header) + 
                       payload_len;
    
    uint8_t* packet = malloc(total_len);
    if (!packet) return 0;
    
    memcpy(packet, &pseudo_header, sizeof(struct tcp_pseudo_header));
    memcpy(packet + sizeof(struct tcp_pseudo_header), tcp, sizeof(struct tcp_header));
    if (payload && payload_len > 0) {
        memcpy(packet + sizeof(struct tcp_pseudo_header) + sizeof(struct tcp_header),
               payload, payload_len);
    }
    
    // Calculate checksum
    uint16_t checksum = calculate_ip_checksum(packet, total_len);
    
    // Free the temporary buffer
    free(packet);
    
    return checksum;
}

struct tcp_packet create_tcp_packet(
    const char* src_ip,
    const char* dst_ip,
    uint16_t src_port,
    uint16_t dst_port,
    uint32_t seq_num,
    uint32_t ack_num,
    uint8_t flags,
    uint8_t* payload,
    size_t payload_len
) {
    struct tcp_packet packet;
    
    uint16_t total_length = sizeof(struct ip_header) + sizeof(struct tcp_header) + payload_len;
    packet.ip = create_ip_header(total_length, IPPROTO_TCP, src_ip, dst_ip);
    packet.tcp = create_tcp_header(src_port, dst_port, seq_num, ack_num, flags);
    
    if (payload && payload_len > 0) {
        packet.payload = malloc(payload_len);
        if (packet.payload) {
            memcpy(packet.payload, payload, payload_len);
        }
    } else {
        packet.payload = NULL;
    }
    
    packet.payload_len = payload_len;

    packet.tcp.checksum = calculate_tcp_checksum(&packet.tcp, &packet.ip, payload, payload_len);

    return packet;
}

uint8_t* tcp_packet_to_bytes(struct tcp_packet* packet, size_t* out_size) {
    if (!packet || !out_size) {
        return NULL;
    }

    *out_size = sizeof(struct ip_header) + sizeof(struct tcp_header) + packet->payload_len;

    uint8_t* buffer = malloc(*out_size);
    if (!buffer) {
        return NULL;
    }

    memcpy(buffer, &packet->ip, sizeof(struct ip_header));

    memcpy(buffer + sizeof(struct ip_header), &packet->tcp, sizeof(struct tcp_header));

    if (packet->payload && packet->payload_len > 0) {
        memcpy(buffer + sizeof(struct ip_header) + sizeof(struct tcp_header), packet->payload, packet->payload_len);
    }

    return buffer;
}