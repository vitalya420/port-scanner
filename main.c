#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/select.h>
#include <time.h>



#include "ip.h"
#include "tcp.h"

// Forward declare the struct first
struct event {
    unsigned short int value;
};

// Function prototypes after struct definition
void set_event(struct event* ev);
unsigned short int is_event_set(struct event* ev);
void clear_event(struct event* ev);
int create_raw_sock();
void close_sockets(int* sockfds, size_t amount);
int* create_raw_sockets(size_t amount);
void event_loop(int socks_amount, int sockets[], struct event* stop_event);

// Function implementations
void set_event(struct event* ev) {
    ev->value = 1;
}

unsigned short int is_event_set(struct event* ev) {
    return ev->value;
}

void clear_event(struct event* ev) {
    ev->value = 0;
}

int create_raw_sock() {
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sockfd < 0) {
        perror("Socket creation failure");
        return -1;
    }

    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl get");
        close(sockfd);
        return -1;
    }

    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl set");
        close(sockfd);
        return -1;
    }

    // Enable IP header include
    int one = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0) {
        perror("setsockopt IP_HDRINCL");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

void close_sockets(int* sockfds, size_t amount) {
    for (size_t i = 0; i < amount; i++) {
        if (sockfds[i] != -1) {
            close(sockfds[i]);
        }
    }
}

int* create_raw_sockets(size_t amount) {
    int* sockfds = malloc(sizeof(int) * amount);
    if (sockfds == NULL) {
        perror("Memory allocation failure");
        return NULL;
    }

    for (size_t i = 0; i < amount; i++) {
        sockfds[i] = create_raw_sock();
        if (sockfds[i] == -1) {
            printf("Failed to create socket %zu\n", i);
            close_sockets(sockfds, i);
            free(sockfds);
            return NULL;
        }
    }
    return sockfds;
}

void event_loop(int socks_amount, int sockets[], struct event* stop_event) {
    fd_set read_fds;
    struct timeval tv;
    int max_fd = -1;
    char buffer[4096];

    while (!is_event_set(stop_event)) {
        FD_ZERO(&read_fds);
        for (int i = 0; i < socks_amount; i++) {
            if (sockets[i] > max_fd) {
                max_fd = sockets[i];
            }
            FD_SET(sockets[i], &read_fds);
        }

        tv.tv_sec = 1;  // 1 second timeout
        tv.tv_usec = 0;

        int ready = select(max_fd + 1, &read_fds, NULL, NULL, &tv);
        if (ready < 0) {
            if (errno == EINTR) {
                continue;  // Interrupted by signal, continue loop
            }
            perror("select");
            break;
        }

        if (ready == 0) {
            continue;  // Timeout, check stop_event and continue
        }

        for (int i = 0; i < socks_amount; i++) {
            if (FD_ISSET(sockets[i], &read_fds)) {
                ssize_t bytes_read = recv(sockets[i], buffer, sizeof(buffer), 0);
                if (bytes_read < 0) {
                    if (errno != EAGAIN && errno != EWOULDBLOCK) {
                        perror("recv");
                    }
                    continue;
                }

                if (bytes_read > 0) {
                    printf("Received %zd bytes on socket %d\n", bytes_read, i);
                    // Process the received data here
                }
            }
        }
    }
}

// int main(int argc, char** argv) {
//     if (getuid() != 0) {
//         fprintf(stderr, "This program requires root privileges to create raw sockets\n");
//         return EXIT_FAILURE;
//     }

//     int* sockets = create_raw_sockets(10);
//     if (sockets == NULL) {
//         return EXIT_FAILURE;
//     }

//     struct event stop_event = {0};

//     printf("Starting event loop. Press Ctrl+C to stop.\n");
//     event_loop(10, sockets, &stop_event);

//     close_sockets(sockets, 10);
//     free(sockets);
//     return EXIT_SUCCESS;
// }

int main() {
    const char* src_ip = "192.168.0.117";
    const char* dst_ip = "8.8.8.8";
    uint16_t src_port = 12345;
    uint32_t seq_num = 1000;
    uint32_t ack_num = 0;
    uint8_t flags = TCP_SYN;

    // Create a raw socket
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sockfd < 0) {
        perror("Failed to create raw socket");
        exit(EXIT_FAILURE);
    }

    // Increase socket send buffer size
    int buffer_size = 512 * 1024; // Set to 512 KB
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &buffer_size, sizeof(buffer_size)) < 0) {
        perror("Failed to set socket send buffer size");
    }

    // Timing
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Send packets to each port in range
    for (uint16_t dst_port = 1; dst_port < 65535; dst_port++) {
        // Create TCP packet for the current destination port
        struct tcp_packet packet = create_tcp_packet(src_ip, dst_ip, src_port, dst_port, seq_num, ack_num, flags, NULL, 0);
        size_t packet_size;
        uint8_t* packet_bytes = tcp_packet_to_bytes(&packet, &packet_size);

        if (!packet_bytes) {
            fprintf(stderr, "Failed to convert packet to bytes\n");
            close(sockfd);
            exit(EXIT_FAILURE);
        }

        // Prepare destination address
        struct sockaddr_in dest_addr;
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(dst_port);
        inet_pton(AF_INET, dst_ip, &dest_addr.sin_addr);

        // Send the packet
        if (sendto(sockfd, packet_bytes, packet_size, 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) {
            perror("Failed to send packet");
        }

        free(packet_bytes);
    }

    // End timing
    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Elapsed time for sending packets to 65535 ports: %.3f seconds\n", elapsed_time);

    // Cleanup
    close(sockfd);

    return 0;
}