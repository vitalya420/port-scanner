#include "utils.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

// Default hexdump configuration
const struct hexdump_config HEXDUMP_DEFAULT = {
    .bytes_per_line = 16,
    .group_bytes = 2,
    .show_ascii = 1,
    .show_offset = 1,
    .offset_format = "%04x: "
};

void format_hex_byte(char* output, uint8_t byte) {
    static const char hex_chars[] = "0123456789ABCDEF";
    output[0] = hex_chars[(byte >> 4) & 0x0F];
    output[1] = hex_chars[byte & 0x0F];
}

static void print_ascii_segment(const uint8_t* data, size_t start, size_t end) {
    printf(" |");
    for (size_t i = start; i < end; i++) {
        printf("%c", isprint(data[i]) ? data[i] : '.');
    }
    printf("|");
}

void hexdump_with_config(const void* data, size_t size, const struct hexdump_config* config) {
    const uint8_t* bytes = (const uint8_t*)data;
    char hex_buf[3] = {0};  // 2 hex chars + null terminator
    size_t offset = 0;

    while (offset < size) {
        // Print offset if enabled
        if (config->show_offset) {
            printf(config->offset_format, (unsigned int)offset);
        }

        // Calculate end of current line
        size_t line_end = offset + config->bytes_per_line;
        if (line_end > size) {
            line_end = size;
        }

        // Print hex values
        for (size_t i = offset; i < line_end; i++) {
            format_hex_byte(hex_buf, bytes[i]);
            printf("%s", hex_buf);

            // Add space after each group
            if (((i + 1) % config->group_bytes) == 0) {
                printf(" ");
            }
        }

        // Pad remaining space if line is incomplete
        if (line_end - offset < config->bytes_per_line) {
            size_t remaining = config->bytes_per_line - (line_end - offset);
            // Each byte takes 2 chars, plus potential spaces for grouping
            size_t padding = (remaining * 2) + (remaining / config->group_bytes);
            for (size_t i = 0; i < padding; i++) {
                printf(" ");
            }
        }

        // Print ASCII representation if enabled
        if (config->show_ascii) {
            print_ascii_segment(bytes, offset, line_end);
        }

        printf("\n");
        offset = line_end;
    }
}

void hexdump(const void* data, size_t size) {
    hexdump_with_config(data, size, &HEXDUMP_DEFAULT);
}

void print_packet_info(const void* packet, size_t size, const char* protocol) {
    printf("\n=== %s Packet Dump (%zu bytes) ===\n", protocol, size);
    hexdump(packet, size);
    printf("=== End of Packet ===\n\n");
}

