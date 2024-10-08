//
// Created by Daniel Ben Naim on 13/06/2024.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 11325
#define WARM_UP_FACTOR 500
#define BUFFER_SIZE 1048576 // 1 MB buffer, adjust if needed

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void measure_throughput(int sock, int message_size, int num_messages) {
    char *message = malloc(message_size);
    char buffer[BUFFER_SIZE];
    memset(message, 'A', message_size);
    struct timespec start, end;
    long total_bytes = 0;


    /*
    We chose warm up factor of 1000 by trying different numbers, and take the minimal number
    where the rest of the throughput seems to stabilize.
     */
    if(message_size == 1) {
        for (int i = 0; i < WARM_UP_FACTOR; i++) {
            send(sock, message, message_size, 0);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < num_messages; i++) {
        send(sock, message, message_size, 0);
        total_bytes += message_size;
    }

    ssize_t bytes_received = 0;
    while(1) {
        bytes_received = recv(sock, buffer, sizeof(buffer), 0);
        if(bytes_received > 0) {
            break;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double time_taken = (end.tv_sec - start.tv_sec) * 1e9;
    time_taken = (time_taken + (end.tv_nsec - start.tv_nsec)) * 1e-9;

    double throughput = (total_bytes / time_taken) / (1024 * 1024); // MB/s
    printf("%d\t%f\tGb/s\n", message_size, throughput / 1000 * 8);

    free(message);
}

int main(int argc, char const *argv[]) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char *server_ip = "127.0.0.1";

    if (argc > 1) {
        server_ip = (char *)argv[1]; // Use user-provided server IP if available
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        error("Socket creation error");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0)
        error("Invalid address/ Address not supported");

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("Connection Failed");

    int message_sizes[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576}; // Exponential series
    int num_messages = 1000; // Number of messages to send

    for (int i = 0; i < sizeof(message_sizes) / sizeof(message_sizes[0]); i++) {
        measure_throughput(sock, message_sizes[i], num_messages);
    }

    close(sock);
    return 0;
}
