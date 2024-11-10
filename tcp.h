#ifndef TCP_H
#define TCP_H

#include <stdint.h>
#include "ip.h"

// TCP flags
#define TCP_FIN  0x01
#define TCP_SYN  0x02
#define TCP_RST  0x04
#define TCP_PSH  0x08
#define TCP_ACK  0x10
#define TCP_URG  0x20

// Standard TCP header structure
struct tcp_header {
    uint16_t src_port;        // Source port
    uint16_t dst_port;        // Destination port
    uint32_t seq_num;         // Sequence number
    uint32_t ack_num;         // Acknowledgment number
    uint8_t data_offset;      // Data offset and reserved bits
    uint8_t flags;            // TCP flags
    uint16_t window;          // Window size
    uint16_t checksum;        // Checksum
    uint16_t urgent_ptr;      // Urgent pointer
};

// TCP pseudo header for checksum calculation
struct tcp_pseudo_header {
    uint32_t src_addr;
    uint32_t dst_addr;
    uint8_t zero;
    uint8_t protocol;
    uint16_t tcp_length;
};

// TCP packet creation function
struct tcp_header create_tcp_header(
    uint16_t src_port,
    uint16_t dst_port,
    uint32_t seq_num,
    uint32_t ack_num,
    uint8_t flags
);

// Calculate TCP checksum including pseudo-header
uint16_t calculate_tcp_checksum(
    struct tcp_header* tcp,
    struct ip_header* ip,
    uint8_t* payload,
    size_t payload_len
);

#endif // TCP_H