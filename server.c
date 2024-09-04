//
// Created by Daniel Ben Naim on 13/06/2024.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 11325

void error(const char *msg) {
    perror(msg);
    exit(1);
}

// int main() {
//     int server_fd, new_socket;
//     struct sockaddr_in address;
//     int opt = 1;
//     int addrlen = sizeof(address);
//     char buffer[1048576] = {0};  // 1MB buffer
//
//     if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
//         error("socket failed");
//
//     if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
//         error("setsockopt");
//
//     address.sin_family = AF_INET;
//     address.sin_addr.s_addr = INADDR_ANY;
//     address.sin_port = htons(PORT);
//
//     if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
//         error("bind failed");
//
//     if (listen(server_fd, 3) < 0)
//         error("listen");
//
//     if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
//         error("accept");
//
//     while (1) {
//         int valread = read(new_socket, buffer, sizeof(buffer));
//         if (valread <= 0) break;
//     }
//
//     close(new_socket);
//     close(server_fd);
//     return 0;
// }





#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1048576 // 1 MB buffer, adjust if needed
#define WARM_UP_FACTOR 500 // Same as client

void handle_client(int client_sock, int message_size, int num_messages) {
    char buffer[BUFFER_SIZE];
    long total_bytes_received = 0;
    int messages_received = 0;

    if(message_size == 1) {
        while(total_bytes_received < WARM_UP_FACTOR) {
            ssize_t bytes_received = recv(client_sock, buffer, sizeof(buffer), 0);
            if (bytes_received <= 0) {
                perror("Error receiving warm-up data");
                return;
            }
            total_bytes_received += bytes_received;
        }
        total_bytes_received = total_bytes_received - WARM_UP_FACTOR;
    }

    // Receive actual messages
    while (total_bytes_received < num_messages * message_size) {
        ssize_t bytes_received = recv(client_sock, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            perror("Error receiving data");
            return;
        }
        total_bytes_received += bytes_received;
    }

    // Send reply
    const char *reply = "All messages received";
    send(client_sock, reply, strlen(reply), 0);
}

int main(int argc, char *argv[]) {
    int server_fd, client_sock;
    int opt = 1;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        error("socket failed");

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
        error("setsockopt");


    // Prepare the server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        error("Bind failed");
        exit(1);
    }

    // Listen for incoming connections
    if (listen(server_fd, 1) < 0) {
        error("Listen failed");
        exit(1);
    }

    printf("Server listening on port %d\n", PORT);

    // Accept and handle client connection
    client_sock = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_sock < 0) {
        error("Accept failed");
        exit(1);
    }

    printf("Client connected\n");

    int message_sizes[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576}; // Exponential series
    int num_messages = 1000; // Number of messages to send
    for (int i = 0; i < sizeof(message_sizes) / sizeof(message_sizes[0]); i++) {
        handle_client(client_sock, message_sizes[i], num_messages);
    }

    // Close sockets
    close(client_sock);
    close(server_fd);

    return 0;
}
