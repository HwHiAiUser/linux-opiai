/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description:
 * Author: huawei
 * Create: 2020-8-27
 */
#ifndef __ASCEND_HAL_DEFINE_H__
#define __ASCEND_HAL_DEFINE_H__

#include "ascend_hal_error.h"


/*========================== typedef ===========================*/
typedef signed char int8_t;
typedef signed int int32_t;
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;

typedef unsigned long long UINT64;
typedef unsigned int UINT32;
typedef unsigned short UINT16;
typedef unsigned char UINT8;

/*========================== Queue Manage ===========================*/
#define DRV_ERROR_QUEUE_INNER_ERROR  DRV_ERROR_INNER_ERR             /**< queue error code */
#define DRV_ERROR_QUEUE_PARA_ERROR  DRV_ERROR_PARA_ERROR
#define DRV_ERROR_QUEUE_OUT_OF_MEM  DRV_ERROR_OUT_OF_MEMORY
#define DRV_ERROR_QUEUE_NOT_INIT  DRV_ERROR_UNINIT
#define DRV_ERROR_QUEUE_OUT_OF_SIZE  DRV_ERROR_OVER_LIMIT
#define DRV_ERROR_QUEUE_REPEEATED_INIT  DRV_ERROR_REPEATED_INIT
#define DRV_ERROR_QUEUE_IOCTL_FAIL  DRV_ERROR_IOCRL_FAIL
#define DRV_ERROR_QUEUE_NOT_CREATED  DRV_ERROR_NOT_EXIST
#define DRV_ERROR_QUEUE_RE_SUBSCRIBED  DRV_ERROR_REPEATED_SUBSCRIBED
#define DRV_ERROR_QUEUE_MULIPLE_ENTRY  DRV_ERROR_BUSY
#define DRV_ERROR_QUEUE_NULL_POINTER  DRV_ERROR_INVALID_HANDLE

/*=========================== Event Sched ===========================*/
#define EVENT_MAX_MSG_LEN        128  /* Maximum message length, only 40 allowed by hardware event scheduler */
#define EVENT_MAX_GRP_NAME_LEN   16
/* The grp name used for sending and receiving queue events between host and device */
#define PROXY_HOST_QUEUE_GRP_NAME "proxy_host_grp"
#define DRV_ERROR_SCHED_INNER_ERR  DRV_ERROR_INNER_ERR                   /**< event sched add error*/
#define DRV_ERROR_SCHED_PARA_ERR DRV_ERROR_PARA_ERROR
#define DRV_ERROR_SCHED_OUT_OF_MEM  DRV_ERROR_OUT_OF_MEMORY
#define DRV_ERROR_SCHED_UNINIT  DRV_ERROR_UNINIT
#define DRV_ERROR_SCHED_NO_PROCESS DRV_ERROR_NO_PROCESS
#define DRV_ERROR_SCHED_PROCESS_EXIT DRV_ERROR_PROCESS_EXIT
#define DRV_ERROR_SCHED_NO_SUBSCRIBE_THREAD DRV_ERROR_NO_SUBSCRIBE_THREAD
#define DRV_ERROR_SCHED_NON_SCHED_GRP_MUL_THREAD DRV_ERROR_NON_SCHED_GRP_MUL_THREAD
#define DRV_ERROR_SCHED_GRP_INVALID DRV_ERROR_NO_GROUP
#define DRV_ERROR_SCHED_PUBLISH_QUE_FULL DRV_ERROR_QUEUE_FULL
#define DRV_ERROR_SCHED_NO_GRP DRV_ERROR_NO_GROUP
#define DRV_ERROR_SCHED_GRP_EXIT DRV_ERROR_GROUP_EXIST
#define DRV_ERROR_SCHED_THREAD_EXCEEDS_SPEC DRV_ERROR_THREAD_EXCEEDS_SPEC
#define DRV_ERROR_SCHED_RUN_IN_ILLEGAL_CPU DRV_ERROR_RUN_IN_ILLEGAL_CPU
#define DRV_ERROR_SCHED_WAIT_TIMEOUT  DRV_ERROR_WAIT_TIMEOUT
#define DRV_ERROR_SCHED_WAIT_FAILED   DRV_ERROR_INNER_ERR
#define DRV_ERROR_SCHED_WAIT_INTERRUPT DRV_ERROR_WAIT_INTERRUPT
#define DRV_ERROR_SCHED_THREAD_NOT_RUNNIG DRV_ERROR_THREAD_NOT_RUNNIG
#define DRV_ERROR_SCHED_PROCESS_NOT_MATCH DRV_ERROR_PROCESS_NOT_MATCH
#define DRV_ERROR_SCHED_EVENT_NOT_MATCH DRV_ERROR_EVENT_NOT_MATCH
#define DRV_ERROR_SCHED_PROCESS_REPEAT_ADD DRV_ERROR_PROCESS_REPEAT_ADD
#define DRV_ERROR_SCHED_GRP_NON_SCHED DRV_ERROR_GROUP_NON_SCHED
#define DRV_ERROR_SCHED_NO_EVENT DRV_ERROR_NO_EVENT
#define DRV_ERROR_SCHED_COPY_USER DRV_ERROR_COPY_USER_FAIL
#define DRV_ERROR_SCHED_SUBSCRIBE_THREAD_TIMEOUT DRV_ERROR_SUBSCRIBE_THREAD_TIMEOUT

/*lint -e116 -e17*/
typedef enum group_type {
    GRP_TYPE_UNINIT = 0,
    /* Bound to a AICPU, multiple threads can be woken up simultaneously within a group */
    GRP_TYPE_BIND_DP_CPU,
    GRP_TYPE_BIND_CP_CPU,             /* Bind to the control CPU */
    GRP_TYPE_BIND_DP_CPU_EXCLUSIVE    /* Bound to a AICPU, intra-group threads are mutex awakened */
} GROUP_TYPE;

typedef enum submit_flag {
    SHARED_EVENT_ENTRY,    /* event num with same struct event_summary */
    SINGLE_EVENT_ENTRY,    /* event num with diff struct event_summary */
} SUBMIT_FLAG;

/* Events can be released between different systems. This parameter specifies the destination type of events
   to be released. The destination type is defined based on the CPU type of the destination system. */
typedef enum schedule_dst_engine {
    ACPU_DEVICE = 0,
    ACPU_HOST = 1,
    CCPU_DEVICE = 2,
    CCPU_HOST = 3,
    DCPU_DEVICE = 4,
    TS_CPU = 5,
    DVPP_CPU = 6,
    ACPU_LOCAL = 7,
    CCPU_LOCAL = 8,
    DST_ENGINE_MAX
} SCHEDULE_DST_ENGINE;

/* When the destination engine is AICPU, select a policy.
   ONLY: The command is executed only on the local AICPU.
   FIRST: The local AICPU is preferentially executed. If the local AICPU is busy, the remote AICPU can be used. */
typedef enum schedule_policy {
    ONLY = 0,
    FIRST = 1,
    POLICY_MAX
} SCHEDULE_POLICY;

typedef enum event_id {
    EVENT_RANDOM_KERNEL,      /* Random operator event */
    EVENT_DVPP_MSG,           /* operator events commited by DVPP */
    EVENT_FR_MSG,             /* operator events commited by Feature retrieves */
    EVENT_TS_HWTS_KERNEL,     /* operator events commited by ts/hwts */
    EVENT_AICPU_MSG,          /* aicpu activates its own stream events */
    EVENT_TS_CTRL_MSG,        /* controls message events of TS */
    EVENT_QUEUE_ENQUEUE,      /* entry event of Queue(consumer) */
    EVENT_QUEUE_FULL_TO_NOT_FULL,   /* full to non-full events of Queue(producers) */
    EVENT_QUEUE_EMPTY_TO_NOT_EMPTY,   /* empty to non-empty event of Queue(consumer) */
    EVENT_TDT_ENQUEUE,        /* data entry event of TDT */
    EVENT_TIMER,              /* ros timer */
    EVENT_HCFI_SCHED_MSG,     /* scheduling events of HCFI */
    EVENT_HCFI_EXEC_MSG,      /* performs the event of HCFI */
    EVENT_ROS_MSG_LEVEL0,
    EVENT_ROS_MSG_LEVEL1,
    EVENT_ROS_MSG_LEVEL2,
    EVENT_ACPU_MSG_TYPE0,
    EVENT_ACPU_MSG_TYPE1,
    EVENT_ACPU_MSG_TYPE2,
    EVENT_CCPU_CTRL_MSG,
    EVENT_SPLIT_KERNEL,
    EVENT_DVPP_MPI_MSG,
    EVENT_CDQ_MSG,            /* message events commited by CDQM(hardware) */
    EVENT_FFTS_PLUS_MSG,      /* operator events commited by FFTS(hardware) */
    EVENT_DRV_MSG,            /* events of drvier */
    EVENT_QS_MSG,             /* events of queue scheduler */
    EVENT_TS_CALLBACK_MSG,
    /* Add a new event here */
    EVENT_TEST,               /* Reserve for test */
    EVENT_USR_START = 48,
    EVENT_USR_END = 63,
    EVENT_MAX_NUM
} EVENT_ID;

typedef enum drv_subevent_id {
    DRV_SUBEVENT_QUEUE_INIT_MSG,
    DRV_SUBEVENT_HDC_INIT_MSG,
    DRV_SUBEVENT_CREATE_MSG,
    DRV_SUBEVENT_GRANT_MSG,
    DRV_SUBEVENT_ATTACH_MSG,
    DRV_SUBEVENT_DESTROY_MSG,
    DRV_SUBEVENT_SUBE2NE_MSG,
    DRV_SUBEVENT_UNSUBE2NE_MSG,
    DRV_SUBEVENT_SUBF2NF_MSG,
    DRV_SUBEVENT_UNSUBF2NF_MSG,
    DRV_SUBEVENT_QUERY_MSG,
    DRV_SUBEVENT_PEEK_MSG,
    DRV_SUBEVENT_ENQUEUE_MSG,
    DRV_SUBEVENT_DEQUEUE_MSG,
    DRV_SUBEVENT_QUEUE_MAX_NUM,
    DRV_SUBEVENT_TRS_ALLOC_RES_ID_MSG = 32, /* every module use 32 ids */
    DRV_SUBEVENT_TRS_FREE_RES_ID_MSG,
    DRV_SUBEVENT_TRS_ALLOC_SQCQ_MSG,
    DRV_SUBEVENT_TRS_FREE_SQCQ_MSG,
    DRV_SUBEVENT_TRS_SHR_ID_CONFIG_MSG,
    DRV_SUBEVENT_TRS_SHR_ID_DECONFIG_MSG,
    DRV_SUBEVENT_MAX_MSG
} DRV_SUBEVENT_ID;

typedef enum schedule_priority {
    PRIORITY_LEVEL0,
    PRIORITY_LEVEL1,
    PRIORITY_LEVEL2,
    PRIORITY_LEVEL3,
    PRIORITY_LEVEL4,
    PRIORITY_LEVEL5,
    PRIORITY_LEVEL6,
    PRIORITY_LEVEL7,
    PRIORITY_MAX
} SCHEDULE_PRIORITY;

struct event_sched_grp_qos {
    unsigned int maxNum;
    unsigned int rsv[7]; /* rsv 7 int */
};

struct event_sync_msg {
    int pid; /* local pid */
    unsigned int dst_engine : 4; /* local engine */
    unsigned int gid : 6;
    unsigned int event_id : 6;
    unsigned int subevent_id : 16; /* Range: 0 ~ 4095 */
    char msg[0];
};

#define EVENT_PROC_RSP_LEN 36
struct event_proc_result {
    int ret;
    char data[EVENT_PROC_RSP_LEN];
};

struct event_reply {
    char *buf;
    unsigned int buf_len;
    unsigned int reply_len;
};

struct iovec_info {
    void *iovec_base;
    unsigned long long len;
};

#define QUEUE_MAX_IOVEC_NUM ((~0U) - 1)
struct buff_iovec {
    void *context_base;
    unsigned long long context_len;
    unsigned int count;
    struct iovec_info ptr[0];
};

struct callback_event_info {
    unsigned int cqid : 16;
    unsigned int cb_groupid : 16;
    unsigned int devid : 16;
    unsigned int stream_id : 16;
    unsigned int event_id : 16;
    unsigned int is_block : 16;
    unsigned int res1;
    unsigned int host_func_low;
    unsigned int host_func_high;
    unsigned int fn_data_low;
    unsigned int fn_data_high;
    unsigned int res2;
    unsigned int res3;
};

typedef enum esched_query_type {
    QUERY_TYPE_LOCAL_GRP_ID,
    QUERY_TYPE_REMOTE_GRP_ID,
    QUERY_TYPE_MAX
} ESCHED_QUERY_TYPE;

struct esched_input_info {
    void *inBuff;
    unsigned int inLen;
};

struct esched_output_info {
    void *outBuff;
    unsigned int outLen;
};

struct esched_query_gid_input {
    int pid;  /* In remote query gid scenario, use drvDeviceGetBareTgid() to get remote pid */
    char grp_name[EVENT_MAX_GRP_NAME_LEN];
};

struct esched_query_gid_output {
    unsigned int grp_id;
};

/*=========================== Queue Manage ===========================*/

typedef enum QueEventCmd {
    QUE_PAUSE_EVENT = 1,    /* pause enque event publish in group */
    QUE_RESUME_EVENT        /* resume enque event publish */
} QUE_EVENT_CMD;

/*=========================== Buffer Manage ===========================*/

#define UNI_ALIGN_MAX       4096
#define UNI_ALIGN_MIN       32
#define BUFF_POOL_NAME_LEN 128
#define BUFF_GRP_NAME_LEN 32
#define BUFF_RESERVE_LEN 8

#define BUFF_GRP_MAX_NUM 1024
#define BUFF_PROC_IN_GRP_MAX_NUM 1024
#define BUFF_GRP_IN_PROC_MAX_NUM 32
#define BUFF_PUB_POOL_CFG_MAX_NUM 128
#define BUFF_GROUP_ADDR_MAX_NUM 1024
#define BUFF_ENABLE_PRIVATE_MBUF 0x5A5A5B5B

#define BUFF_ALL_DEVID 1
#define BUFF_ONE_DEVID 0

#define BUFF_FLAGS_ALL_DEVID_OFFSET 31
#define BUFF_FLAGS_DEVID_OFFSET 32

#define XSMEM_BLK_NOT_AUTO_RECYCLE (1UL << 63)
#define XSMEM_BLK_ALLOC_FROM_OS (1UL << 62)

typedef enum group_id_type {
    GROUP_ID_CREATE,
    GROUP_ID_ADD
} GROUP_ID_TYPE;

typedef struct {
    unsigned long long maxMemSize; /* max buf size in grp, in KB, if = 0 means no limit */
    unsigned int cacheAllocFlag;     /* alloc cache memory strategy enable flag */
    unsigned int privMbufFlag;       /* private mbuf enable flag */
    int rsv[BUFF_GRP_NAME_LEN - 2];  /* reserve, caller must clear the value */
} GroupCfg;

typedef struct {
    unsigned long long memSize;     /* cache size, in KB */
    unsigned int memFlag;           /* cache memory flag */
    int rsv[BUFF_RESERVE_LEN];      /* reserve, caller must clear the value */
} GrpCacheAllocPara;

typedef struct {
    unsigned int admin : 1;     /* admin permission, can add other proc to grp */
    unsigned int read : 1;     /* rsv, not support */
    unsigned int write : 1;    /* read and write permission */
    unsigned int alloc : 1;    /* alloc permission (have read and write permission) */
    unsigned int rsv : 28;
}GroupShareAttr;

typedef enum {
    GRP_QUERY_GROUP,                  /* query grp info include proc and permission */
    GRP_QUERY_GROUPS_OF_PROCESS,      /* query process all grp */
    GRP_QUERY_GROUP_ID,               /* query grp ID by grp name */
    GRP_QUERY_GROUP_ADDR_INFO,        /* query group addr info */
    GRP_QUERY_CMD_MAX
} GroupQueryCmdType;

typedef struct {
    char grpName[BUFF_GRP_NAME_LEN];
} GrpQueryGroup; /* cmd: GRP_QUERY_GROUP */

typedef struct {
    int pid;
} GrpQueryGroupsOfProc; /* cmd: GRP_QUERY_GROUPS_OF_PROCESS */

typedef struct {
    char grpName[BUFF_GRP_NAME_LEN];
} GrpQueryGroupId; /* cmd: GRP_QUERY_GROUP_ID */

typedef struct {
    char grpName[BUFF_GRP_NAME_LEN];
    unsigned int devId;
} GrpQueryGroupAddrPara; /* cmd: GRP_QUERY_GROUP_ADDR_INFO */

typedef union {
    GrpQueryGroup grpQueryGroup; /* cmd: GRP_QUERY_GROUP */
    GrpQueryGroupsOfProc grpQueryGroupsOfProc; /* cmd: GRP_QUERY_GROUPS_OF_PROCESS */
    GrpQueryGroupId grpQueryGroupId; /* cmd: GRP_QUERY_GROUP_ID */
    GrpQueryGroupAddrPara grpQueryGroupAddrPara; /* cmd: GRP_QUERY_GROUP_ADDR_INFO */
} GroupQueryInput;

typedef struct {
    int pid; /* pid in grp */
    GroupShareAttr attr; /* process in grp attribute */
} GrpQueryGroupInfo;  /* cmd: GRP_QUERY_GROUP */

typedef struct {
    char groupName[BUFF_GRP_NAME_LEN];  /* grp name */
    GroupShareAttr attr; /* process in grp attribute */
} GrpQueryGroupsOfProcInfo; /* cmd: GRP_QUERY_GROUPS_OF_PROCESS */

typedef struct {
    int groupId; /* grp Id */
} GrpQueryGroupIdInfo; /* cmd: GRP_QUERY_GROUP_ID */

typedef struct {
    unsigned long long addr; /* cache memory addr */
    unsigned long long size; /* cache memory size */
} GrpQueryGroupAddrInfo; /* cmd: GRP_QUERY_GROUP_ADDR_INFO */

typedef union {
    GrpQueryGroupInfo grpQueryGroupInfo[BUFF_PROC_IN_GRP_MAX_NUM];  /* cmd: GRP_QUERY_GROUP */
    GrpQueryGroupsOfProcInfo grpQueryGroupsOfProcInfo[BUFF_GRP_MAX_NUM]; /* cmd: GRP_QUERY_GROUPS_OF_PROCESS */
    GrpQueryGroupIdInfo grpQueryGroupIdInfo; /* cmd: GRP_QUERY_GROUP_ID */
    GrpQueryGroupAddrInfo grpQueryGroupAddrInfo[BUFF_GROUP_ADDR_MAX_NUM]; /* cmd: GRP_QUERY_GROUP_ADDR_INFO */
} GroupQueryOutput;

#define BUFF_MAX_CFG_NUM 64

typedef struct {
    unsigned int cfg_id;    /* cfg id, start from 0 */
    unsigned long long total_size;  /* one zone total size */
    unsigned int blk_size;  /* blk size, 2^n (0, 2M] */
    unsigned long long max_buf_size; /* max size can alloc from zone */
    unsigned int page_type;  /* page type, small page/huge page, normal/dvpp */
    int elasticEnable; /* elastic enable, only support in private group which only includes one process */
    int elasticRate;
    int elasticRateMax;
    int elasticHighLevel;
    int elasticLowLevel;
    int rsv[8];
} memZoneCfg;

typedef struct {
    memZoneCfg cfg[BUFF_MAX_CFG_NUM];
}BuffCfg;

typedef struct {
    struct {
        unsigned int blkSize;     /* blk size */
        unsigned int blkNum;    /* blk num, blkSize * blkNum must < 4G Byte */
        unsigned int align;      /* addr align, must be an integer multiple of 2, 2< algn <4k */
        unsigned int hugePageFlag; /* huge page flag */
        int reserve[2]; /* reserved */
    } pubPoolCfg[BUFF_PUB_POOL_CFG_MAX_NUM]; /* max allo 128 cfg */
} PubPoolAttr;
/*lint +e116 +e17*/

enum BuffConfCmdType {
    BUFF_CONF_MBUF_TIMEOUT_CHECK = 0,
    BUFF_CONF_MEMZONE_BUFF_CFG = 1,
    BUFF_CONF_MBUF_TIMESTAMP_SET = 2,
    BUFF_CONF_MAX
};

enum BuffGetCmdType {
    BUFF_GET_MBUF_TIMEOUT_INFO = 0,
    BUFF_GET_MBUF_USE_INFO = 1,
    BUFF_GET_MBUF_TYPE_INFO = 2,
    BUFF_GET_BUFF_TYPE_INFO,
    BUFF_GET_MAX
};

struct MbufTimeoutCheckPara {
    unsigned int enableFlag;     /* enable: 1; disable: 0 */
    unsigned int maxRecordNum;   /* maximum number timeout mbuf info recored */
    unsigned int timeout;        /* mbuf timeout value,  unit:ms, minimum 10ms, default 1000 ms */
    unsigned int checkPeriod;    /* mbuf check thread work period, uinit:ms, minimum 1000ms, default:1s */
};

struct MbufDataInfo {
    void *mp;
    int owner;
    unsigned int ref;
    unsigned int blkSize;
    unsigned int totalBlkNum;
    unsigned int availableNum;
    unsigned int allocFailCnt;
};

struct MbufDebugInfo {
    unsigned long long timeStamp;
    void *mbuf;
    int usePid;
    int allocPid;
    unsigned int useTime;
    unsigned int round;
    struct MbufDataInfo dataInfo;
    char poolName[BUFF_POOL_NAME_LEN];
    int reserve[BUFF_RESERVE_LEN];
};

struct MbufUseInfo {
    int allocPid;                /* mbuf alloc pid */
    int usePid;                  /* mbuf use pid */
    unsigned int ref;              /* mbuf reference num */
    unsigned int status;           /* mbuf status, 1 means in use, not support other status currently */
    unsigned long long timestamp;  /* mbuf alloc timestamp, cpu tick */
    int reserve[BUFF_RESERVE_LEN]; /* for reserve */
};

enum MbufType {
    MBUF_CREATE_BY_ALLOC = 3,   /* malloc by mbuf_alloc */
    MBUF_CREATE_BY_POOL,        /* malloc by mbuf_alloc_by_pool */
    MBUF_CREATE_BY_BUILD,       /* malloc by mbuf_alloc_by_build */
    MBUF_CREATE_BY_BARE_BUFF,   /* malloc by mbufBuildBareBuff */
};

struct MbufTypeInfo {
    unsigned int type;         /* mbuf type */
};

enum BuffType {
    BUFF_TYPE_NORMAL = 0,
    BUFF_TYPE_MBUF_DATA,
    BUFF_TYPE_MAX
};

struct BuffTypeInfo {
    enum BuffType type;
};

struct buf_scale_event {
    int type; /* 0: del, 1: add */
    int grpId; /* share buff group id */
    unsigned long long addr;
    unsigned long long size; /* size is invalid in del */
};

enum halCtlCmdType {
    HAL_CTL_REGISTER_LOG_OUT_HANDLE = 1,
    HAL_CTL_UNREGISTER_LOG_OUT_HANDLE = 2,
    HAL_CTL_CMD_MAX
};

struct log_out_handle {
    void (*DlogInner)(int moduleId, int level, const char *fmt, ...);
    unsigned int logLevel;
};

/*=========================== Memory Manage ===========================*/
#define DV_MEM_SVM 0x0001
#define DV_MEM_SVM_HOST 0x0002
#define DV_MEM_SVM_DEVICE 0x0004

#define  DEVMM_MAX_MEM_TYPE_VALUE       4       /**< max memory type */

#define  MEM_INFO_TYPE_DDR_SIZE         1       /**< DDR memory type */
#define  MEM_INFO_TYPE_HBM_SIZE         2       /**< HBM memory type */
#define  MEM_INFO_TYPE_DDR_P2P_SIZE     3       /**< DDR P2P memory type */
#define  MEM_INFO_TYPE_HBM_P2P_SIZE     4       /**< HBM P2P memory type */
#define  MEM_INFO_TYPE_ADDR_CHECK       5       /**< check addr */
#define  MEM_INFO_TYPE_CTRL_NUMA_INFO   6       /**< query device ctrl numa id config */
#define  MEM_INFO_TYPE_AI_NUMA_INFO     7       /**< query device ai numa id config */
#define  MEM_INFO_TYPE_BAR_NUMA_INFO    8       /**< query device bar numa id config */
#define  MEM_INFO_TYPE_MAX              9       /**< max type */

enum DEVMM_MEMCPY2D_TYPE {
    DEVMM_MEMCPY2D_SYNC = 0,
    DEVMM_MEMCPY2D_ASYNC_CONVERT = 1,
    DEVMM_MEMCPY2D_ASYNC_DESTROY = 2,
    DEVMM_MEMCPY2D_TYPE_MAX
};

enum ADVISE_MEM_TYPE {
    ADVISE_PERSISTENT = 0,
    ADVISE_DEV_MEM = 1,
    ADVISE_TYPE_MAX
};

enum MEMCPY_SUMBIT_TYPE {
    MEMCPY_SUMBIT_SYNC = 0,
    MEMCPY_SUMBIT_ASYNC = 1,
    MEMCPY_SUMBIT_MAX_TYPE
};

struct DMA_OFFSET_ADDR {
    unsigned long long offset;
    unsigned int devid;
};

struct DMA_PHY_ADDR {
    void *src;           /**< src addr(physical addr) */
    void *dst;           /**< dst addr(physical addr) */
    unsigned int len;    /**< length */
    unsigned char flag;  /**< Flag=0 Non-chain, SRC and DST are physical addresses, can be directly DMA copy operations*/
                         /**< Flag=1 chain, SRC is the address of the dma list and can be used for direct dma copy operations*/
    void *priv;
};

struct DMA_ADDR {
    union {
        struct DMA_PHY_ADDR phyAddr;
        struct DMA_OFFSET_ADDR offsetAddr;
    };
    unsigned int fixed_size; /**< Output: the actual conversion size */
    unsigned int virt_id;    /**< store logic id for destroy addr */
};

struct drvMem2D {
    unsigned long long *dst;        /**< destination memory address */
    unsigned long long dpitch;      /**< pitch of destination memory */
    unsigned long long *src;        /**< source memory address */
    unsigned long long spitch;      /**< pitch of source memory */
    unsigned long long width;       /**< width of matrix transfer */
    unsigned long long height;      /**< height of matrix transfer */
    unsigned long long fixed_size;  /**< Input: already converted size. if fixed_size < width*height,
                                         need to call halMemcpy2D multi times */
    unsigned int direction;         /**< copy direction */
    unsigned int resv1;
    unsigned long long resv2;
};

struct drvMem2DAsync {
    struct drvMem2D copy2dInfo;
    struct DMA_ADDR *dmaAddr;
};

struct MEMCPY2D {
    unsigned int type;      /**< DEVMM_MEMCPY2D_SYNC: memcpy2d sync */
                            /**< DEVMM_MEMCPY2D_ASYNC_CONVERT: memcpy2d async convert */
                            /**< DEVMM_MEMCPY2D_ASYNC_DESTROY: memcpy2d async destroy */
    unsigned int resv;
    union {
        struct drvMem2D copy2d;
        struct drvMem2DAsync copy2dAsync;
    };
};

/* enables different options to be specified that affect the host register */
enum drvRegisterTpye {
    HOST_MEM_MAP_DEV = 0,
    HOST_SVM_MAP_DEV,
    DEV_SVM_MAP_HOST,
    HOST_MEM_MAP_DEV_PCIE_TH,
    DEV_MEM_MAP_HOST,
    HOST_REGISTER_MAX_TPYE
};

enum ctrlType {
    CTRL_TYPE_ADDR_MAP = 0,
    CTRL_TYPE_ADDR_UNMAP = 1,
    CTRL_TYPE_SUPPORT_FEATURE = 2,
    CTRL_TYPE_MAX
};

#define CTRL_SUPPORT_NUMA_TS_BIT 0
#define CTRL_SUPPORT_NUMA_TS_MASK (1ul << CTRL_SUPPORT_NUMA_TS_BIT)
#define CTRL_SUPPORT_PCIE_BAR_MEM_BIT 1
#define CTRL_SUPPORT_PCIE_BAR_MEM_MASK (1ul << CTRL_SUPPORT_PCIE_BAR_MEM_BIT)
#define CTRL_SUPPORT_DEV_MEM_REGISTER_BIT 2
#define CTRL_SUPPORT_DEV_MEM_REGISTER_MASK (1ul << CTRL_SUPPORT_DEV_MEM_REGISTER_BIT)
#define CTRL_SUPPORT_PCIE_BAR_HUGE_MEM_BIT 3
#define CTRL_SUPPORT_PCIE_BAR_HUGE_MEM_MASK (1ul << CTRL_SUPPORT_PCIE_BAR_HUGE_MEM_BIT)

struct supportFeaturePara {
    unsigned long long support_feature;
    unsigned int devid;
};

enum addrMapType {
    ADDR_MAP_TYPE_L2_BUFF = 0,
    ADDR_MAP_TYPE_REG_C2C_CTRL = 1,
    ADDR_MAP_TYPE_MAX
};

struct AddrMapInPara {
    unsigned int addr_type;
    unsigned int devid;
};

struct AddrMapOutPara {
    unsigned long long ptr;
    unsigned long long len;
};

struct AddrUnmapInPara {
    unsigned int addr_type;
    unsigned int devid;
    unsigned long long ptr;
    unsigned long long len;
};

typedef enum tagProcStatus {
    STATUS_NOMEM = 0x1,           /* Out of memory */
    STATUS_MAX
} processStatus_t;

typedef enum tagProcType {
    PROCESS_CP1 = 0,   /* aicpu_scheduler */
    PROCESS_CP2,       /* custom_process */
    PROCESS_DEV_ONLY,  /* TDT */
    PROCESS_QS,        /* queue_scheduler */
    PROCESS_CPTYPE_MAX
} processType_t;

enum drv_mem_side {
    MEM_HOST_SIDE = 0,
    MEM_DEV_SIDE,
    MEM_MAX_SIDE
};

enum drv_mem_pg_type {
    MEM_NORMAL_PAGE_TYPE = 0,
    MEM_HUGE_PAGE_TYPE,
    MEM_MAX_PAGE_TYPE
};

enum drv_mem_type {
    MEM_HBM_TYPE = 0,
    MEM_DDR_TYPE,
    MEM_P2P_HBM_TYPE,
    MEM_P2P_DDR_TYPE,
    MEM_TS_DDR_TYPE,
    MEM_MAX_TYPE
};

/*=========================== Memory Manage End =======================*/

/*=============================== TSDRV START =============================*/
#define TSDRV_FLAG_REUSE_CQ (0x1 << 0)
#define TSDRV_FLAG_REUSE_SQ (0x1 << 1)
#define TSDRV_FLAG_THREAD_BIND_IRQ (0x1 << 2)
#define TSRRV_FLAG_SQ_RDONLY    (0x1 << 3)
#define TSDRV_FLAG_ONLY_SQCQ_ID (0x1 << 4)
#define TSDRV_FLAG_REMOTE_ID (0x1 << 5)

#define SQCQ_RTS_INFO_LENGTH 5
#define SQCQ_RESV_LENGTH 8
#define SQCQ_UMAX 0xFFFFFFFF

typedef enum tagDrvSqCqType {
    DRV_NORMAL_TYPE = 0,
    DRV_CALLBACK_TYPE,
    DRV_LOGIC_TYPE,
    DRV_SHM_TYPE,
    DRV_CTRL_TYPE,
    DRV_INVALID_TYPE
}  drvSqCqType_t;

struct halSqCqInputInfo {
    drvSqCqType_t type;  // normal : 0, callback : 1
    uint32_t tsId;
    /* The size and depth of each cqsq can be configured in normal mode, but this function is not yet supported */
    uint32_t sqeSize;    // normal : 64Byte
    uint32_t cqeSize;    // normal : 12Byte
    uint32_t sqeDepth;   // normal : 1024
    uint32_t cqeDepth;   // normal : 1024

    uint32_t grpId;   // runtime thread identifier,normal : 0
    uint32_t flag;    // ref to TSDRV_FLAG_*
    uint32_t cqId;    // if flag bit 0 is 0, don't care about it
    uint32_t sqId;    // if flag bit 1 is 0, don't care about it

    uint32_t info[SQCQ_RTS_INFO_LENGTH];  // inform to ts through the mailbox, consider single operator performance
    uint32_t res[SQCQ_RESV_LENGTH];
};

struct halSqCqOutputInfo {
    uint32_t sqId;  // return to UMAX when there is no sq
    uint32_t cqId;  // return to UMAX when there is cq
    unsigned long long queueVAddr; /* return shm sq addr */
    uint32_t flag;    // ref to TSDRV_FLAG_*
    uint32_t res[SQCQ_RESV_LENGTH - 3];
};

struct halSqCqFreeInfo {
    drvSqCqType_t type; // normal : 0, callback : 1
    uint32_t tsId;
    uint32_t sqId;
    uint32_t cqId;  // cqId to be freed, if flag bit 0 is 0, don't care about it
    uint32_t flag;  // bit 0 : whether cq is to be freed  0 : free, 1 : no free
    uint32_t res[SQCQ_RESV_LENGTH];
};

#define SQCQ_CONFIG_INFO_LENGTH 8
#define SQCQ_QUERY_INFO_LENGTH 8
#define RESOURCE_CONFIG_INFO_LENGTH 7

typedef enum tagDrvSqCqPropType {
    DRV_SQCQ_PROP_SQ_STATUS = 0x0,
    DRV_SQCQ_PROP_SQ_HEAD,
    DRV_SQCQ_PROP_SQ_TAIL,
    DRV_SQCQ_PROP_SQ_DISABLE_TO_ENABLE,
    DRV_SQCQ_PROP_SQ_CQE_STATUS, /* read clear */
    DRV_SQCQ_PROP_SQ_REG_BASE,
    DRV_SQCQ_PROP_SQ_BASE,
    DRV_SQCQ_PROP_SQ_DEPTH,
    DRV_SQCQ_PROP_MAX
} drvSqCqPropType_t;

struct halSqCqConfigInfo {
    drvSqCqType_t type;
    uint32_t tsId;
    uint32_t sqId;
    uint32_t cqId;
    drvSqCqPropType_t prop;
    uint32_t value[SQCQ_CONFIG_INFO_LENGTH];
};

struct halSqCqQueryInfo {
    drvSqCqType_t type;
    uint32_t tsId;
    uint32_t sqId;
    uint32_t cqId;
    drvSqCqPropType_t prop;
    uint32_t value[SQCQ_QUERY_INFO_LENGTH];
};

typedef enum tagDrvResourceConfigType {
    DRV_STREAM_BIND_LOGIC_CQ = 0x0,
    DRV_STREAM_UNBIND_LOGIC_CQ,
    DRV_ID_RECORD,
    DRV_STREAM_ENABLE_EVENT,
    DRV_RES_ID_CONFIG_MAX
} drvResourceConfigType_t;

struct halResourceConfigInfo {
    drvResourceConfigType_t prop;
    uint32_t value[RESOURCE_CONFIG_INFO_LENGTH];
};

typedef enum tagDrvResQueryType {
    DRV_RES_QUERY_OFFSET,
    DRV_RES_QUERY_MAX
} drvResQueryType_t;

struct halResourceDetailInfo {
    drvResQueryType_t type;
    uint32_t value0; /* type=0: offset */
    uint32_t value1;
    uint32_t reserve[2];
};

enum shr_id_type {
    SHR_ID_NOTIFY_TYPE = 0,
    SHR_ID_EVENT_TYPE = 1,
    SHR_ID_TYPE_MAX
};

struct drvShrIdInfo {
    uint32_t devid; /* input:logic devid; output:phy devid */
    uint32_t tsid;  /* input and output */
    uint32_t id_type;
    uint32_t shrid;
    uint32_t flag;  /* for remote id */
    uint32_t rsv[2];
};

struct halTaskSendInfo {
    drvSqCqType_t type;
    uint32_t tsId;
    uint32_t sqId;
    int32_t timeout;      // send wait time
    uint8_t *sqe_addr;
    uint32_t sqe_num;
    uint32_t pos; /* output: first sqe pos */
    uint32_t res[SQCQ_RESV_LENGTH];  /* must zero out */
};

struct halReportRecvInfo {
    drvSqCqType_t type;
    uint32_t tsId;
    uint32_t cqId;
    int32_t timeout;      // recv wait time
    uint8_t *cqe_addr;
    uint32_t cqe_num;
    uint32_t report_cqe_num; /* output */
    uint32_t stream_id;
    uint32_t task_id; /* If this parameter is set to all 1, strict matching is not performed for taskid. */
    uint32_t res[SQCQ_RESV_LENGTH];
};

typedef enum tagTsDrvCtlCmdType {
    TSDRV_CTL_CMD_CB_GROUP_NUM_GET = 0,
    TSDRV_CTL_CMD_BIND_STL = 1,
    TSDRV_CTL_CMD_MAX
} tsDrvCtlCmdType_t;

/*=============================== TSDRV END ===============================*/

/*=============================== HDC START =============================*/
enum drvHdcSessionStatus {
    HDC_SESSION_STATUS_CONNECT = 1,
    HDC_SESSION_STATUS_CLOSE,
    HDC_SESSION_STATUS_UNKNOW_ERR,
    HDC_SESSION_STATUS_MAX
};
/*=============================== HDC END ===============================*/

/*=============================== DP_PROC START =============================*/
typedef enum {
    BIND_AICPU_CGROUP = 0,
    BIND_DATACPU_CGROUP,
    BIND_CGROUP_MAX_TYPE
} BIND_CGROUP_TYPE;
/*=============================== DP_PROC END ===============================*/

#endif
