#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

// Hexdump configuration struct
struct hexdump_config {
    int bytes_per_line;    // Number of bytes to display per line
    int group_bytes;       // Number of bytes to group together
    int show_ascii;        // Whether to show ASCII representation
    int show_offset;       // Whether to show offset
    char offset_format[8]; // Format for offset (e.g., "%04x: " or "%08x: ")
};

// Default hexdump configuration
extern const struct hexdump_config HEXDUMP_DEFAULT;

// Hexdump functions
void hexdump(const void* data, size_t size);
void hexdump_with_config(const void* data, size_t size, const struct hexdump_config* config);

// Format a single byte as hex
void format_hex_byte(char* output, uint8_t byte);

// Print formatted packet information
void print_packet_info(const void* packet, size_t size, const char* protocol);

#endif // UTILS_H