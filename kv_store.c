//
// Created by Gal on 30/08/2024.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <infiniband/verbs.h>
#include <pthread.h>

#define MAX_KEY_LEN 256
#define MAX_VALUE_LEN 4096
#define MAX_ENTRIES 1024
#define MAX_CLIENTS 10

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
    // Simulate RDMA write operation to receive large values directly into memory
    // Actual implementation would involve ibv_post_send with IBV_WR_RDMA_WRITE
    handle_set(key, value);  // For demonstration, we treat it similarly
}

// Function to handle GET requests for large values (>=4KB) using RDMA
void handle_large_get(kv_handle_t *kv_handle, char *key, char **value, size_t *value_size) {
    // Simulate RDMA read operation to send large values directly from memory
    // Actual implementation would involve ibv_post_send with IBV_WR_RDMA_READ
    *value = handle_get(key);  // For demonstration, we treat it similarly
}

// Thread function to handle client requests
void* client_handler(void* arg) {
    kv_handle_t *kv_handle = (kv_handle_t *)arg;
    // Handle requests for this client
    // Poll for incoming messages, determine if SET/GET or large SET/GET
    char key[MAX_KEY_LEN] = "example_key";
    char value[MAX_VALUE_LEN] = "example_value";

    // Simulate receiving a SET request
    if (strlen(value) < MAX_VALUE_LEN) {
        handle_set(key, value);
    } else {
        handle_large_set(kv_handle, key, value, strlen(value));
    }

    // Simulate receiving a GET request
    char *retrieved_value = handle_get(key);
    printf("Retrieved value: %s\n", retrieved_value);

    return NULL;
}

// Function to establish an RDMA connection (simplified)
int kv_open(char *servername, kv_handle_t **kv_handle) {
    *kv_handle = malloc(sizeof(kv_handle_t));
    // RDMA setup code: create context, PD, QP, etc.
    return 0; // Success
}

// Function to handle SET requests from the client
int kv_set(kv_handle_t *kv_handle, const char *key, const char *value) {
    size_t value_size = strlen(value);
    if (value_size < MAX_VALUE_LEN) {
        handle_set((char *)key, (char *)value);
    } else {
        handle_large_set(kv_handle, (char *)key, (char *)value, value_size);
    }
    return 0; // Success
}

// Function to handle GET requests from the client
int kv_get(kv_handle_t *kv_handle, const char *key, char **value) {
    size_t value_size = 0;
    if (value_size < MAX_VALUE_LEN) {
        *value = handle_get((char *)key);
    } else {
        handle_large_get(kv_handle, (char *)key, value, &value_size);
    }
    return 0; // Success
}

// Function to release memory allocated for GET values
void kv_release(char *value) {
    // Free any allocated resources for the value
    // In this example, no dynamic allocation, so no need to free
}

// Function to close the RDMA connection and clean up
int kv_close(kv_handle_t *kv_handle) {
    // Destroy the RDMA connection and free resources
    free(kv_handle);
    return 0; // Success
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <server|client>\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "server") == 0) {
        pthread_t client_threads[MAX_CLIENTS];
        kv_handle_t kv_handles[MAX_CLIENTS];

        // Initialize RDMA resources for the server

        for (int i = 0; i < MAX_CLIENTS; i++) {
            // Accept connections from clients and create a thread for each
            pthread_create(&client_threads[i], NULL, client_handler, &kv_handles[i]);
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            pthread_join(client_threads[i], NULL);
        }

        // Cleanup RDMA resources

    } else if (strcmp(argv[1], "client") == 0) {
        kv_handle_t *handle;
        kv_open("servername", &handle);

        kv_set(handle, "key1", "value1");

        char *value;
        kv_get(handle, "key1", &value);
        printf("Received value: %s\n", value);

        kv_release(value);
        kv_close(handle);
    }

    return 0;
}
