#include <infiniband/verbs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    struct ibv_device **dev_list;
    struct ibv_device *ib_dev;
    struct ibv_context *ctx;
    struct ibv_pd *pd;
    struct ibv_cq *cq;
    struct ibv_qp *qp;
    struct ibv_qp_init_attr qp_init_attr = {0};
    int max_inline_data;
    int step = 1; // Increment step for max_inline_data
    int ret;

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

    // Allocate Protection Domain (PD)
    pd = ibv_alloc_pd(ctx);
    if (!pd) {
        perror("Failed to allocate PD");
        return 1;
    }

    // Create Completion Queue (CQ)
    cq = ibv_create_cq(ctx, 16, NULL, NULL, 0);
    if (!cq) {
        perror("Failed to create CQ");
        return 1;
    }

    // Initialize QP attributes
    qp_init_attr.send_cq = cq;
    qp_init_attr.recv_cq = cq;
    qp_init_attr.cap.max_send_wr = 1;
    qp_init_attr.cap.max_recv_wr = 1;
    qp_init_attr.cap.max_send_sge = 1;
    qp_init_attr.cap.max_recv_sge = 1;
    qp_init_attr.qp_type = IBV_QPT_RC;

    // Start with a reasonable max_inline_data value and increment to find the maximum
    for (max_inline_data = 768; max_inline_data <= 832; max_inline_data += step) {
        qp_init_attr.cap.max_inline_data = max_inline_data;

        qp = ibv_create_qp(pd, &qp_init_attr);
        if (qp) {
            printf("Successfully created QP with max_inline_data = %d bytes\n", max_inline_data);
            ibv_destroy_qp(qp);  // Clean up QP if created successfully
        } else {
            printf("Failed to create QP with max_inline_data = %d bytes\n", max_inline_data);
            max_inline_data -= step;  // Step back to the last successful value
            break;
        }
    }

    printf("Maximum supported max_inline_data size: %d bytes\n", max_inline_data);

    // Cleanup
    ibv_destroy_cq(cq);
    ibv_dealloc_pd(pd);
    ibv_close_device(ctx);
    ibv_free_device_list(dev_list);

    return 0;
}
