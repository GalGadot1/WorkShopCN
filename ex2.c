#include <stdio.h>
#include <stdlib.h>
#include <infiniband/verbs.h>
#include <string.h>
#include <time.h>

// Define constants and global variables
#define SERVER_PORT 18515
#define BUFFER_SIZE 1024

struct ibv_device **dev_list;
struct ibv_context *ctx;
struct ibv_pd *pd;
struct ibv_mr *mr;
struct ibv_cq *cq;
struct ibv_qp *qp;
struct ibv_comp_channel *comp_channel;
char *buffer;
int num_devices;

void setup_verbs() {
    // Obtain the list of devices
    dev_list = ibv_get_device_list(&num_devices);
    if (!dev_list) {
        perror("Failed to get IB devices list");
        exit(1);
    }

    // Open the device context
    ctx = ibv_open_device(dev_list[0]);
    if (!ctx) {
        perror("Failed to open device");
        exit(1);
    }

    // Allocate a Protection Domain (PD)
    pd = ibv_alloc_pd(ctx);
    if (!pd) {
        perror("Failed to allocate PD");
        exit(1);
    }

    // Allocate memory for the buffer
    buffer = malloc(BUFFER_SIZE);
    if (!buffer) {
        perror("Failed to allocate memory");
        exit(1);
    }

    // Register the memory region
    mr = ibv_reg_mr(pd, buffer, BUFFER_SIZE, IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE);
    if (!mr) {
        perror("Failed to register memory region");
        exit(1);
    }

    // Create the Completion Queue (CQ)
    cq = ibv_create_cq(ctx, 1, NULL, NULL, 0);
    if (!cq) {
        perror("Failed to create CQ");
        exit(1);
    }

    // Create the Queue Pair (QP)
    struct ibv_qp_init_attr qp_init_attr = {
            .send_cq = cq,
            .recv_cq = cq,
            .cap     = {
                    .max_send_wr  = 1,
                    .max_recv_wr  = 1,
                    .max_send_sge = 1,
                    .max_recv_sge = 1,
                    .max_inline_data = BUFFER_SIZE,
            },
            .qp_type = IBV_QPT_RC,
    };

    qp = ibv_create_qp(pd, &qp_init_attr);
    if (!qp) {
        perror("Failed to create QP");
        exit(1);
    }

    // Set the QP state to INIT
    struct ibv_qp_attr qp_attr = {
            .qp_state        = IBV_QPS_INIT,
            .pkey_index      = 0,
            .port_num        = 1,
            .qp_access_flags = IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE,
    };

    if (ibv_modify_qp(qp, &qp_attr,
                      IBV_QP_STATE | IBV_QP_PKEY_INDEX |
                      IBV_QP_PORT | IBV_QP_ACCESS_FLAGS)) {
        perror("Failed to modify QP to INIT");
        exit(1);
    }
}

void cleanup_verbs() {
    ibv_destroy_qp(qp);
    ibv_destroy_cq(cq);
    ibv_dereg_mr(mr);
    free(buffer);
    ibv_dealloc_pd(pd);
    ibv_close_device(ctx);
    ibv_free_device_list(dev_list);
}

int main(int argc, char *argv[]) {
    setup_verbs();

    // Implement the main logic for RDMA WRITE and IBV_WR_SEND
    // ...

    cleanup_verbs();
    return 0;
}
