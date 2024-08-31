#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <infiniband/verbs.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_KEY_LEN 256
#define MAX_VALUE_LEN 4096
#define MAX_ENTRIES 1024
#define MAX_CLIENTS 10
#define PORT 11325

void error(const char *msg) {
    perror(msg);
    exit(1);
}

typedef struct {
    char key[MAX_KEY_LEN];
    char value[MAX_VALUE_LEN];
} kv_entry_t;

typedef struct {
    struct ibv_context *context;
    struct ibv_pd *pd;
    struct ibv_mr *mr;
    struct ibv_cq *cq;
    struct ibv_qp *qp;
    kv_entry_t *kv_store;
    int client_socket;
} kv_handle_t;

kv_entry_t kv_store[MAX_ENTRIES];
int kv_count = 0;

// Function to handle SET requests for small values (<4KB)
void handle_set(char *key, char *value) {
    for (int i = 0; i < kv_count; i++) {
        if (strcmp(kv_store[i].key, key) == 0) {
            strncpy(kv_store[i].value, value, MAX_VALUE_LEN);
            return;
        }
    }
    if (kv_count < MAX_ENTRIES) {
        strncpy(kv_store[kv_count].key, key, MAX_KEY_LEN);
        strncpy(kv_store[kv_count].value, value, MAX_VALUE_LEN);
        kv_count++;
    }
}

// Function to handle GET requests for small values (<4KB)
char* handle_get(char *key) {
    for (int i = 0; i < kv_count; i++) {
        if (strcmp(kv_store[i].key, key) == 0) {
            return kv_store[i].value;
        }
    }
    return "";
}

// Function to handle SET requests for large values (>=4KB) using RDMA
void handle_large_set(kv_handle_t *kv_handle, char *key, char *value, size_t value_size) {
    handle_set(key, value);  // For demonstration, treat similarly to small values
}

// Function to handle GET requests for large values (>=4KB) using RDMA
void handle_large_get(kv_handle_t *kv_handle, char *key, char **value, size_t *value_size) {
    *value = handle_get(key);  // For demonstration, treat similarly to small values
}

// Thread function to handle client requests
void* client_handler(void* arg) {
    kv_handle_t *kv_handle = (kv_handle_t *)arg;
    char key[MAX_KEY_LEN];
    char value[MAX_VALUE_LEN];
    while(1) {
        // Simulate receiving a SET request
        int valread = read(kv_handle->client_socket, key, MAX_KEY_LEN);
        if(valread <= 0) {
            close(kv_handle->client_socket);
            return NULL;
        }
        valread = read(kv_handle->client_socket, value, MAX_VALUE_LEN);

        if (valread > 0) {
            handle_set(key, value);
        } else {
            char *retrieved_value = handle_get(key);
            write(kv_handle->client_socket, retrieved_value, MAX_VALUE_LEN);
        }
    }
}

// Function to establish an RDMA connection (simplified)
int kv_open(char *servername, kv_handle_t **kv_handle) {
    *kv_handle = malloc(sizeof(kv_handle_t));
    return 0; // Success
}

// Function to handle SET requests from the client
int kv_set(kv_handle_t *kv_handle, const char *key, const char *value) {
    write(kv_handle->client_socket, key, MAX_KEY_LEN);
    write(kv_handle->client_socket, value, MAX_VALUE_LEN);
    return 0; // Success
}

// Function to handle GET requests from the client
int kv_get(kv_handle_t *kv_handle, const char *key, char **value) {
    write(kv_handle->client_socket, key, MAX_KEY_LEN);
    *value = malloc(MAX_VALUE_LEN);
    read(kv_handle->client_socket, *value, MAX_VALUE_LEN);
    return 0; // Success
}

// Function to release memory allocated for GET values
void kv_release(char *value) {
    free(value);
}

// Function to close the RDMA connection and clean up
int kv_close(kv_handle_t *kv_handle) {
    close(kv_handle->client_socket);
    free(kv_handle);
    return 0; // Success
}

void start_server() {
    int server_socket, client_socket;
    int opt = 1;

    struct sockaddr_in server_addr, client_addr;
    pthread_t client_threads[MAX_CLIENTS];
    kv_handle_t kv_handles[MAX_CLIENTS];

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        error("socket failed");

    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
        error("setsockopt");

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        error("bind failed");

    if (listen(server_socket, MAX_CLIENTS) < 0)
        error("listen");

    for (int i = 0; i < MAX_CLIENTS; i++) {
        socklen_t addr_size = sizeof(client_addr);
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, (socklen_t*)&addr_size)) < 0)
            error("accept");
        kv_handles[i].client_socket = client_socket;
        pthread_create(&client_threads[i], NULL, client_handler, &kv_handles[i]);
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
        pthread_join(client_threads[i], NULL);
    }

    close(server_socket);
}

void start_client(char *servername) {
    kv_handle_t *handle;
    struct sockaddr_in server_addr;

    kv_open(servername, &handle);

    if ((handle->client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        error("Socket creation error");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, servername, &server_addr.sin_addr) <= 0)
        error("Invalid address/ Address not supported");

    if (connect(handle->client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        error("Connection Failed");

    kv_set(handle, "key1", "value1");

    char *value;
    kv_get(handle, "key1", &value);
    printf("Received value: %s\n", value);

    kv_release(value);
    kv_close(handle);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        start_server();
    } else {
        start_client(argv[1]);
    }

    return 0;
}