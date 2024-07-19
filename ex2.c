//
// Created by Daniel Ben Naim on 05/07/2024.
//
#include <infiniband/verbs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MSG_SIZE 1024

// Helper function for high-resolution timing
static inline uint64_t get_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

int main(int argc, char *argv[]) {
    // Initialization and resource allocation (context, PD, CQ, etc.)
    // Code for creating QP, registering memory, and setting up the RDMA communication

    struct ibv_context *ctx;
    struct ibv_pd *pd;
    struct ibv_cq *cq;
    struct ibv_qp *qp;
    struct ibv_mr *mr;
    char *buf;
    int num_conns = 1;

    // Allocate memory for the message
    posix_memalign((void **)&buf, 4096, MSG_SIZE);

    // Setup the RDMA resources and connections here
    // ...

    // Measure throughput
    uint64_t start_time = get_time_ns();
    for (int i = 0; i < num_conns; i++) {
        struct ibv_sge sge;
        struct ibv_send_wr wr, *bad_wr;

        // Setup the scatter-gather entry
        sge.addr = (uintptr_t)buf;
        sge.length = MSG_SIZE;
        sge.lkey = mr->lkey;

        // Setup the send work request
        memset(&wr, 0, sizeof(wr));
        wr.wr_id = i;
        wr.opcode = IBV_WR_RDMA_WRITE;
        wr.send_flags = IBV_SEND_SIGNALED;
        wr.sg_list = &sge;
        wr.num_sge = 1;
        wr.wr.rdma.remote_addr = remote_addr; // Remote address
        wr.wr.rdma.rkey = remote_rkey;        // Remote key

        // Post the send request
        if (ibv_post_send(qp, &wr, &bad_wr)) {
            fprintf(stderr, "Error, ibv_post_send() failed\n");
            exit(1);
        }

        // Wait for completion
        struct ibv_wc wc;
        while (ibv_poll_cq(cq, 1, &wc) == 0) {
            // Polling for completion
        }
        if (wc.status != IBV_WC_SUCCESS) {
            fprintf(stderr, "Error, completion status is %d\n", wc.status);
            exit(1);
        }
    }
    uint64_t end_time = get_time_ns();

    // Calculate throughput
    double throughput = (double)(MSG_SIZE * num_conns) / ((end_time - start_time) / 1000000000.0);
    printf("Throughput: %f MB/s\n", throughput / (1024 * 1024));

    // Clean up and release resources
    // ...

    return 0;
}
