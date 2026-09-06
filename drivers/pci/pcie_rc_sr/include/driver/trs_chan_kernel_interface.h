/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description:
 * Author: huawei
 * Create: 2023-09-01
 */
#ifndef TRS_CHAN_KERNEL_INTERFACE_H
#define TRS_CHAN_KERNEL_INTERFACE_H
#include <linux/types.h>
#include "trs_base_define.h"

#define CHAN_TYPE_HW 0 /* Both SQ and CQ interact with hardware. (normal sqcq) */
#define CHAN_TYPE_SW 1 /* Both SQ and CQ interact with software(TSCPU). (the physical sqcq used by logic/shm sqcq) */
#define CHAN_TYPE_MAINT 2 /* Both SQ and CQ interact with the TSCPU for maintenance and test (functional sqcq) */
#define CHAN_TYPE_TASK_SCHED 3 /* SQ and CQ for the task initiated by the TSCPU for the user process
                             (the physical sqcq used by callback sqcq) */
#define CHAN_TYPE_MAX 4

/* CHAN_TYPE_HW subtype */
#define CHAN_SUB_TYPE_HW_RTS            0
#define CHAN_SUB_TYPE_HW_TOPIC_SCHED    1
#define CHAN_SUB_TYPE_HW_DVPP           2
#define CHAN_SUB_TYPE_HW_TS             3
#define CHAN_SUB_TYPE_HW_RSV_TS         4

/* CHAN_TYPE_SW subtype */
#define CHAN_SUB_TYPE_SW_CTRL 0
#define CHAN_SUB_TYPE_SW_LOGIC 1
#define CHAN_SUB_TYPE_SW_SHM 2

/* CHAN_TYPE_MAINT subtype */
#define CHAN_SUB_TYPE_MAINT_LOG 0
#define CHAN_SUB_TYPE_MAINT_PROF 1
#define CHAN_SUB_TYPE_MAINT_HB 2
#define CHAN_SUB_TYPE_MAINT_DBG 3

/* CHAN_TYPE_TASK_SCHED subtype */
#define CHAN_SUB_TYPE_TASK_SCHED_ASYNC_CB 0
#define CHAN_SUB_TYPE_TASK_SCHED_SYNC_CB 1

#define CHAN_SUB_TYPE_MAX 5

#define CHAN_FLAG_ALLOC_SQ_BIT 0
#define CHAN_FLAG_ALLOC_CQ_BIT 1
#define CHAN_FLAG_NOTICE_TS_BIT 2
#define CHAN_FLAG_AUTO_UPDATE_SQ_HEAD_BIT 3
#define CHAN_FLAG_RECV_BLOCK_BIT 4
#define CHAN_FLAG_NO_ALLOC_SQCQ_MEM_BIT 5
#define CHAN_FLAG_USE_MASTER_PID_BIT 6

#define CQ_RECV_CONTINUE 0
#define CQ_RECV_FINISH 1

#define TRS_INVALID_CHAN_ID (-1)
#define SQCQ_INFO_LENGTH 5

struct trs_chan_type {
    u32 type;
    u32 sub_type;
};

struct trs_chan_sq_para {
    u32 sq_depth;
    u32 sqe_size;
};

struct trs_chan_cq_para {
    u32 cq_depth;
    u32 cqe_size;
};

struct trs_chan_sq_trace {
    /* Content from trs chan */
    u32 sqid;
    u32 status;
    u32 sq_head;
    u32 sq_tail;

    u32 chan_id;
    struct trs_chan_type types;
    /* Content from sqe */
    u32 type;
    u32 task_id;
    u32 stream_id;
};

struct trs_chan_cq_trace {
    /* Content from trs chan */
    u32 cqid;
    u32 cq_head;
    u32 round;

    u32 chan_id;
    struct trs_chan_type types;
    /* Content from sqe */
    u32 task_id;
    u32 sq_id;
    u32 sq_head;
    u32 stream_id;
};

struct trs_chan_ops {
    bool (*cqe_is_valid)(void *cqe, u32 round); /* not must */
    void (*get_sq_head_in_cqe)(void *cqe, u32 *sq_head); /* not must */
    /* should only return CQ_RECV_CONTINUE or CQ_RECV_FINISH */
    int (*cq_recv)(struct trs_id_inst *inst, u32 cqid, void *cqe); /* not must */
    int (*abnormal_proc)(struct trs_id_inst *inst, int chan_id, u8 err_type);
    void (*trace_cqe_fill)(struct trs_id_inst *inst, struct trs_chan_cq_trace *cq_trace, void *cqe); /* not must */
    void (*trace_sqe_fill)(struct trs_id_inst *inst, struct trs_chan_sq_trace *sq_trace, void *sqe); /* not must */
};

struct trs_chan_para {
    u32 flag;
    int ssid;
    u32 msg[SQCQ_INFO_LENGTH]; /* send to ts */
    struct trs_chan_type types;
    struct trs_chan_sq_para sq_para;
    struct trs_chan_cq_para cq_para;
    struct trs_chan_ops ops;
};

struct trs_chan_send_para {
    u8 *sqe;
    u32 sqe_num;
    u32 first_pos; /* output */
    int timeout; /* ms */
};

struct trs_chan_recv_para {
    u8 *cqe;
    u32 cqe_num;
    u32 recv_cqe_num; /* output */
    int timeout; /* ms */
};

/**
* @trs channel create
* @param [in]  inst: channel inst
* @param [in]  para: channel arg
* @param [out] chan_id: channel id
* @return 0: success, else: fail
*/
int hal_kernel_trs_chan_create(struct trs_id_inst *inst, struct trs_chan_para *para, int *chan_id);
/**
* @trs channel destroy
* @param [in]  inst: channel inst
* @param [in] chan_id: channel id
*/
void hal_kernel_trs_chan_destroy(struct trs_id_inst *inst, int chan_id);
/**
* @trs channel send
* @param [in]  inst: channel inst
* @param [in] chan_id: channel id
* @param [in] para: send para
* @return 0: success, else: fail
*/
int hal_kernel_trs_chan_send(struct trs_id_inst *inst, int chan_id, struct trs_chan_send_para *para);
/**
* @trs channel recv
* @param [in]  inst: channel inst
* @param [in] chan_id: channel id
* @param [inout] para: recv para
* @return 0: success, else: fail
*/
int hal_kernel_trs_chan_recv(struct trs_id_inst *inst, int chan_id, struct trs_chan_recv_para *para);

#endif
