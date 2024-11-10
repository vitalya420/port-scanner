#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/select.h>

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

int main(int argc, char** argv) {
    if (getuid() != 0) {
        fprintf(stderr, "This program requires root privileges to create raw sockets\n");
        return EXIT_FAILURE;
    }

    int* sockets = create_raw_sockets(10);
    if (sockets == NULL) {
        return EXIT_FAILURE;
    }

    struct event stop_event = {0};

    printf("Starting event loop. Press Ctrl+C to stop.\n");
    event_loop(10, sockets, &stop_event);

    close_sockets(sockets, 10);
    free(sockets);
    return EXIT_SUCCESS;
}