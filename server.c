//
// Created by Daniel Ben Naim on 13/06/2024.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1048576] = {0};  // 1MB buffer

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        error("socket failed");

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
        error("setsockopt");

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
        error("bind failed");

    if (listen(server_fd, 3) < 0)
        error("listen");

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
        error("accept");

    while (1) {
        int valread = read(new_socket, buffer, sizeof(buffer));
        if (valread <= 0) break;
    }

    close(new_socket);
    close(server_fd);
    return 0;
}
