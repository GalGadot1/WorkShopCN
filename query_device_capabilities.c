//
// Created by Daniel Ben Naim on 19/07/2024.
//
#include <infiniband/verbs.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_INLINE 256  // Example value, should be less than device_attr.max_inline_data

struct rdma_context {
    struct ibv_context *ctx;
    struct ibv_pd *pd;
    struct ibv_cq *cq;
    struct ibv_qp *qp;
};

struct rdma_context *init_context(struct ibv_context *verbs, int max_inline_data) {
    struct rdma_context *ctx = (struct rdma_context *)malloc(sizeof(struct rdma_context));
    ctx->ctx = verbs;
    ctx->pd = ibv_alloc_pd(ctx->ctx);
    ctx->cq = ibv_create_cq(ctx->ctx, 16, NULL, NULL, 0);

    struct ibv_qp_init_attr qp_attr = {
            .send_cq = ctx->cq,
            .recv_cq = ctx->cq,
            .cap = {
                    .max_send_wr = 1,
                    .max_recv_wr = 1,
                    .max_send_sge = 1,
                    .max_recv_sge = 1,
                    .max_inline_data = max_inline_data
            },
            .qp_type = IBV_QPT_RC
    };

    ctx->qp = ibv_create_qp(ctx->pd, &qp_attr);
    if (!ctx->qp) {
        perror("Failed to create QP");
        exit(1);
    }

    return ctx;
}

int main() {
    struct ibv_device **dev_list;
    struct ibv_device *ib_dev;
    struct ibv_context *ctx;
    struct ibv_device_attr device_attr;

    // Get the list of available devices
    dev_list = ibv_get_device_list(NULL);
    if (!dev_list) {
        perror("Failed to get IB devices list");
        return 1;
    }

    ib_dev = dev_list[0];
    if (!ib_dev) {
        fprintf(stderr, "No IB devices found\n");
        return 1;
    }

    // Open the device
    ctx = ibv_open_device(ib_dev);
    if (!ctx) {
        perror("Failed to open device");
        return 1;
    }

    // Query device attributes
    if (ibv_query_device(ctx, &device_attr)) {
        perror("Failed to query device attributes");
        return 1;
    }

    // Initialize RDMA context with the max inline data size
    struct rdma_context *rdma_ctx = init_context(ctx, device_attr.max_inline_data);

    // Clean up
    ibv_close_device(ctx);
    ibv_free_device_list(dev_list);

    // Further code to use the rdma_ctx...

    return 0;
}

