/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
 * Description:
 * Author: huawei
 * Create: 2019-10-15
 */
#ifndef __ASCEND_KERNEL_HAL_H__
#define __ASCEND_KERNEL_HAL_H__
#include <linux/types.h>
#include "ascend_hal_define.h"
#define LPM3_IDLE_CMD 9
#define IPC_CMD_DVPP_MIN    241
#define IPC_CMD_DVPP_MAX    249
#define IPC_CMD_RETR_MIN    250
#define IPC_CMD_RETR_MAX    255

#ifndef u64
typedef unsigned long long u64;
#endif

#ifndef u32
typedef unsigned int u32;
#endif

#ifndef u16
typedef unsigned short u16;
#endif

#ifndef u8
typedef unsigned char u8;
#endif
#include "trs_kernel_interface.h"

/**
* @ingroup driver-stub
* @brief interface for dvpp notify LP to adjust ddr frequency
* @param [in]  unsigned int dev_id: device ID
* @param [in]  unsigned char cmd_type0: message target
* @param [in]  unsigned char cmd_type1: module target
* @param [in]  unsigned char *data: message
* @param [in]  unsigned int data_len: message length
* @return   0 for success, others for fail
*/
int hal_kernel_send_ipc_to_lp_async(unsigned int devid, unsigned char cmd_type0,
    unsigned char cmd_type1, unsigned char *data, unsigned int data_len);

/**
* @ingroup driver-stub
* @brief  interface for reporting the in-position information of the optical module
* @param [in]  unsigned int qsfp_index Optical port index
* @param [out] unsigned int *val presence information
* @return   0 for success, others for fail
*/
int drv_cpld_qsfp_present_query(unsigned int qsfp_index, unsigned int *val);
/**
* @ingroup driver-stub
* @brief  obtain the MAC address of the user configuration area.
* @param [in]  unsigned int dev_id Device ID
* @param [in]  unsigned char *buf Buffer for storing information
* @param [in]  unsigned int buf_size Size of the buffer for storing information
* @param [out] unsigned int *info_size Data length of the returned MAC information
* @return   0 for success, others for fail
* @note Support:Ascend910,Ascend910B
*/
int devdrv_config_get_mac_info(unsigned int dev_id,
                               unsigned char *buf,
                               unsigned int buf_size,
                               unsigned int *info_size);
/**
* @ingroup driver-stub
* @brief   interface for obtaining the information about the user configuration area
* @param [in]  unsigned int dev_id Device ID
* @param [in]  const char *name: configuration item name
* @param [in]  unsigned char *buf Buffer for storing information
* @param [out] unsigned int *buf_size Obtain the information length
* @return   0 for success, others for fail
* @note Support:Ascend310,Ascend910,Ascend610,Ascend310P,Ascend910B,Ascend310B
*/

int devdrv_get_user_config(unsigned int dev_id, const char *name, unsigned char *buf, unsigned int *buf_size);
/**
* @ingroup driver-stub
* @brief   This interface is used to set the information about the user configuration area.
*          Currently, this interface can be invoked only by the DMP process. In other cases, the permission fails to be returned
* @param [in]  unsigned int dev_id Device ID
* @param [in]  const char *name: configuration item name
* @param [in]  unsigned char *buf Buffer for storing information
* @param [out] unsigned int *buf_size Obtain the information length
*              Due to the storage space limit, when the configuration area information is set,
*              The length of the setting information needs to be limited.
*              The current length range is as follows: For cloud-related forms,
*              the maximum value of buf_size is 0x8000, that is, 32 KB.
*              For mini-related forms, the maximum value of buf_size is 0x800, that is, 2 KB.
*              If the length is greater than the value of this parameter, a message is displayed,
*              indicating that the setting fails.
* @return   0 for success, others for fail
* @note Support:Ascend310,Ascend910,Ascend610,Ascend310P,Ascend910B,Ascend310B
*/
int devdrv_set_user_config(unsigned int dev_id, const char *name, unsigned char *buf, unsigned int buf_size);
/**
* @ingroup driver-stub
* @brief   This interface is used to clear the configuration items in the user configuration area.
*          Currently, this interface can be invoked only by the DMP process.
*          In other cases, a permission failure is returned.
* @param [in]  unsigned int dev_id Device ID
* @param [in]  const char *name: configuration item name
* @param [in]  unsigned char *buf Buffer for storing information
* @return   0 Success, others for fail
* @note Support:Ascend310,Ascend910,Ascend610,Ascend310P,Ascend910B,Ascend310B
*/
int devdrv_clear_user_config(unsigned int devid, const char *name);

/**
* @ingroup driver-stub
* @brief   This interface is used to get flash item in the user configuration area.
* @param [in]  unsigned int: dev_id Device ID
* @param [in]  const char *: configuration name
* @param [in]  unsigned char *:data buffer for storing information
* @param [in]  unsigned int: data buffer length
* @param [out] unsigned int: data buffer out length
* @return   0 Success, others for fail
* @        -2 item not set
* @note Support:Ascend910,Ascend610,Ascend310P,Ascend910B,Ascend310B
*/
int hal_kernel_get_user_config(unsigned int dev_id, const char *name, unsigned char *data,
    unsigned int in_len, unsigned int *out_len);

/**
* @ingroup driver-stub
* @brief   This interface is used to set flash item in the user configuration area.
* @param [in]  unsigned int: dev_id Device ID
* @param [in]  const char *: configuration name
* @param [in]  unsigned char *:data buffer for storing information
* @param [in]  unsigned int: data buffer length
* @return   0 Success, others for fail
* @        -2 item not set
* @note Support:Ascend910,Ascend610,Ascend310P,Ascend910B,Ascend310B
*/
int hal_kernel_set_user_config(unsigned int dev_id, const char *name, u8 *data, u32 in_len);

typedef enum dms_pg_info_type {
    PG_INFO_TYPE_AIC = 0,
    PG_INFO_TYPE_CPU,
    PG_INFO_TYPE_HBM,
    PG_INFO_TYPE_MATA,
    PG_INFO_TYPE_MAX,
} HAL_DMS_PG_INFO_TYPE;

typedef struct {
    u32 num;
    u32 freq;       /* core working frequency */
    u64 bitmap;     /* 1:good, 0:bad */
    u64 reserved;
} hal_dms_common_pg_info_t;

/**
* @ingroup driver-stub
* @brief   This interface is used to get pg info from dms module.
* @param [in]  unsigned int: dev_id Device ID
* @param [in]  HAL_DMS_PG_INFO_TYPE: pg info type
* @param [out]  unsigned char *: pg info data pointer
* @param [in]  unsigned int : input data size
* @param [out]  unsigned int *: return info size
* @return   0 Success, others for fail
*/
int hal_kernel_dms_get_pg_info(unsigned int dev_id, HAL_DMS_PG_INFO_TYPE info_type,
    char* data, unsigned int size, unsigned int *ret_size);

#define BUFF_SP_NORMAL 0
#define BUFF_SP_HUGEPAGE_PRIOR (1 << 0)
#define BUFF_SP_HUGEPAGE_ONLY (1 << 1)
#define BUFF_SP_DVPP (1 << 2)

/**
* @ingroup driver-stub
* @brief   This interface is used to switch chip id to numa id.
* @param [in]  unsigned int device_id: device id
* @param [in]  unsigned int type: memory type
* @return   success return numa id, fail return fail code
*/
int hal_kernel_numa_get_nid(unsigned int device_id, unsigned int type);

struct buff_proc_free {
    int pid;
} ;

typedef void (*buff_free_ops)(struct buff_proc_free *arg);

/**
* @ingroup driver
* @brief   This interface is used to register func, which call before buff drv free share pool mem
* @param [in]  func: need to call before recycle
* @param [out]  module_idx: to unreg func module idx
* @return   0 Success, others for fail
*/
int hal_kernel_buff_register_proc_free_notifier(buff_free_ops func, unsigned int *module_idx);

/**
* @ingroup driver-stub
* @brief   This interface is used to unregister the func reg by hal_kernel_buff_register_proc_free_notifier
* @param [in] module_idx: module idx, which is returned in reg func
* @return   0 Success, others for fail
*/
int hal_kernel_buff_unregister_proc_free_notifier(unsigned int module_idx);

enum sensorhub_pps_source_type {
    PPS_FROM_XGMAC = 0x0,
    PPS_FROM_CHIP  = 0x1
};

enum sensorhub_ssu_ctrl_type {
    SSU_SW      = 0x0,
    PATH_SW     = 0x1,
    SSU_BUT
};

enum sensorhub_intr_ctrl_type {
    NORMAL1_MASK_CFG = 0x0,
    NORMAL2_MASK_CFG = 0x1,
    ERROR1_MASK_CFG  = 0x2,
    ERROR2_MASK_CFG  = 0x3,
    NORMAL_INT_CLR   = 0x4,
    INT_CTR_BUT
};

enum sensorhub_fsin_cfg_type {
    PPS_THRESHOLD       = 0x0,
    PPS_PW              = 0x1,
    PPS_BIAS_THRESH     = 0x2,
    EXPO_INIT_PRE       = 0x3,
    FSIN_FRAME_RATE     = 0x4,
    FSIN_CLR_TS         = 0x5,
    FSIN_CAM_MAP        = 0x6,
    FSIN_COMP_NUM       = 0x7,
    FSIN_CTRL_BUT
};

enum sensorhub_module_type {
    SSU_CTRL_MODULE         = 0x0,
    INTERRUPT_CTRL_MODULE   = 0x1,
    FSIN_CTRL_MODULE        = 0x2,
    IMU_CTRL_MODULE         = 0x3,
    PERI_SUBCTRL_MODULE     = 0x4,
    GPIO_CTRL_MODULE        = 0x5,
    FAULT_CHECK_CFG         = 0x6,
    SSU_STROBE_SET_CFG      = 0x7,
    SSU_CAMERA_CHECK_READ   = 0x8,
    MODULE_BUT
};

enum sensorhub_fault_check_type {
    EXPO_CHECK_PRE_THRESHOLD    = 0x0,
    EXPO_CHECK_POST_THRESHOLD   = 0x1
};

struct sensorhub_sub_cmd_value_stru {
    unsigned int value;
};

struct sensorhub_fsync_info_stru {
    unsigned int value;
    unsigned int sub_mod_id;
};

struct sensorhub_sub_cmd_info_stru {
    unsigned int value_0;
    unsigned int value_1;
};

struct sensorhub_msg_head_stru {
    enum sensorhub_module_type cmd;
    unsigned int sub_cmd;  /* fsin_cfg_type or ssu_ctrl_type or intr_ctrl_type */
    unsigned int len;
    void *param; /* sub_cmd_value or fsync_info or sub_cmd_info */
};

/**
* @ingroup driver-stub
* @brief   This interface is used to configure sensorhub module..
* @param [in]  hal_kernel_buff_notify_handle handle: notify handle
* @return   0 Success, others for fail
*/
int hal_kernel_sensorhub_set_module(struct sensorhub_msg_head_stru *para_cfg);

#define QOS_CFG_RESERVED_LEN 8
#define QOS_MASTER_BITMAP_LEN 4
#define MAX_QOS_MASTER_NODE_NAME_LEN 256

enum qos_master_type {
    MASTER_DVPP_VENC    = 0,
    MASTER_DVPP_VDEC    = 1,
    MASTER_DVPP_VPC     = 2,
    MASTER_DVPP_JPEGE   = 3,
    MASTER_DVPP_JPEGD   = 4,
    MASTER_ROCE         = 5,
    MASTER_NIC          = 6,
    MASTER_PCIE         = 7,
    MASTER_AICPU        = 8,
    MASTER_AIC_DAT      = 9,
    MASTER_AIC_INS      = 10,
    MASTER_AIV_DAT      = 11,
    MASTER_AIV_INS      = 12,
    MASTER_SDMA         = 13,
    MASTER_STARS        = 14,
    MASTER_ISP_VICAP    = 15,
    MASTER_ISP_VIPROC   = 16,
    MASTER_ISP_VIPE     = 17,
    MASTER_ISP_GDC      = 18,
    MASTER_ISP_VGS      = 19,
    MASTER_USB          = 20,
    MASTER_MATA         = 21,
    MASTER_DMC          = 22,
    MASTER_VDP          = 23,
    MASTER_GPU          = 24,
    MASTER_AUDIO        = 26,
    MASTER_SATA         = 27,
    MASTER_TPU          = 28,
    MASTER_RPU          = 29,
    MASTER_INVALID
};

struct qos_master_config_type {
    enum qos_master_type type; /* master type */
    unsigned int mpamid; /* mpam id */
    unsigned int qos; /* qos */
    unsigned int pmg; /* pmg */
    unsigned long long bitmap[QOS_MASTER_BITMAP_LEN]; /* max support 64 * 4  */
    unsigned int mode; /* 0 -- regs vaild, 1 -- smmu vaild, 2 -- sqe vaild */
    unsigned int reserved[QOS_CFG_RESERVED_LEN - 1];
};

#define MAX_OTSD_LEVEL 2
struct qos_otsd_config_type {
    enum qos_master_type master;
    unsigned int otsd_mode; /* 0 -- disable otsd limit, 1 -- read & write merge, 2 -- read & write not merge */
    unsigned int otsd_lvl[MAX_OTSD_LEVEL]; /* otsd level */
    int reserved[QOS_CFG_RESERVED_LEN];
    unsigned long long bitmap[4]; /* max support 64 * 4 */
};

#define MAX_QOS_ALLOW_LEVEL 3
struct qos_allow_config_type {
    enum qos_master_type master;
    unsigned int qos_allow_mode;        /* 0 -- disable bp, 1 -- produce bp, 2 -- response bp */
    unsigned int qos_allow_ctrl;        /* 0 -- all, 1 -- read, 2 -- write */
    unsigned int qos_allow_threshold;   /* for media, threshold for generating a count(unit: ns) */
    unsigned int qos_allow_windows;     /* width of the statistics window, (unit: ns) */
    unsigned int qos_allow_saturation;  /* allowable error value of bandwidth limit, (unit: GB/s) */
    unsigned int qos_allow_lvl[MAX_QOS_ALLOW_LEVEL]; /* qos allow level */
    int reserved[QOS_CFG_RESERVED_LEN];
    unsigned long long bitmap[4];       /* max support 64 * 4 */
};

typedef int (*set_qos_cfg)(int dev_id, const struct qos_master_config_type *cfg);
typedef int (*get_qos_cfg)(int dev_id, struct qos_master_config_type *cfg);
typedef int (*set_allow_cfg)(int dev_id, const struct qos_allow_config_type *cfg);
typedef int (*get_allow_cfg)(int dev_id, struct qos_allow_config_type *cfg);
typedef int (*set_otsd_cfg)(int dev_id, const struct qos_otsd_config_type *cfg);
typedef int (*get_otsd_cfg)(int dev_id, struct qos_otsd_config_type *cfg);

struct qos_master_node {
    char name[MAX_QOS_MASTER_NODE_NAME_LEN]; /* master name */
    struct qos_master_config_type cfg; /* master qos config */
    set_qos_cfg set;            /* set qos config handler */
    get_qos_cfg get;            /* get qos config handler */
    set_allow_cfg set_allow;    /* if support cfg qos allow, can't be null */
    get_allow_cfg get_allow;    /* if support cfg qos allow, can't be null */
    set_otsd_cfg set_otsd;      /* if support cfg otsd, can't be null */
    get_otsd_cfg get_otsd;      /* if support cfg otsd, can't be null */
};

/**
* @brief register qos master node to ascend qos hal
* @param [in] const struct qos_master_node *master: qos master node info
* @return 0: success, else: fail
*/
int hal_kernel_qos_node_register(const struct qos_master_node *master);

/**
* @brief unregister qos master node from ascend qos hal
* @param [in] const struct qos_master_node *master: qos master node info
* @return 0: success, else: fail
*/
int hal_kernel_qos_node_unregister(const struct qos_master_node *master);

/**
* @ingroup driver
* @brief Set qos config to PCIe registers
* @param [in]  int dev_id: device id
* @param [in]  const struct qos_master_config *cfg: qos config
* @return 0: success, else: fail
*/
int hal_kernel_agentdrv_set_qos_config(int dev_id, const struct qos_master_config_type *cfg);

/**
* @brief notify QoS driver that module has gone online
* @param [in] master: master type
* @return 0: success, else: fail
*/
int hal_kernel_qos_notify_module_online(int dev_id, enum qos_master_type master);

/**
* @brief notify QoS driver that module has gone offline
* @param [in] master: master type
* @return 0: success, else: fail
*/
int hal_kernel_qos_notify_module_offline(int dev_id, enum qos_master_type master);
/**
* @ingroup driver
* @brief Get qos config form PCIe registers
* @param [in]  int dev_id: device id
* @param [out] struct qos_master_config_type *cfg: qos config
* @return 0: success, else: fail
*/
int hal_kernel_agentdrv_get_qos_config(int dev_id, struct qos_master_config_type *cfg);

/********************************* svm kernel_api start ***********************************************/
struct svm_dma_desc_addr_info {
    u64 src_va;
    u64 dst_va;
    u64 size;
};

struct svm_dma_desc_handle {
    int pid;
    u32 key;
    u32 subkey;
};

struct svm_dma_desc {
    void *sq_addr;
    u32 sq_tail;    /* means the cnt of sq_addr */
};

/* if subkey==SVM_DMA_DESC_INVALID_SUB_KEY, means destroy all nodes related to key */
#define SVM_DMA_DESC_INVALID_SUB_KEY 0xFFFFFFFFu

int hal_kernel_svm_dma_desc_create(struct svm_dma_desc_addr_info *addr_info,
    struct svm_dma_desc_handle *handle, struct svm_dma_desc *dma_desc);
void hal_kernel_svm_dma_desc_destroy(struct svm_dma_desc_handle *handle);

#ifndef CFG_ENABLE_ASAN
/* orther addr not test, dev mmap fail because program segments loading random */
#define SVM_READ_ONLY_ADDR_START (0x100000000000ULL + 0x20000000000Ul)
#define SVM_READ_ONLY_ADDR_END (0x100000000000ULL + 0x20000000000U + 0x4000000000Ul - 1)
#else
/* device asan 0x100000000000ULL mmap fail */
#define SVM_READ_ONLY_ADDR_START (0x210000000000ULL + 0x20000000000Ul)
#define SVM_READ_ONLY_ADDR_END (0x210000000000ULL + 0x20000000000U + 0x4000000000Ul - 1)
#endif

static inline int hal_kernel_svm_addr_is_read_only(u64 addr, u64 len)
{
    return (int)((addr >= SVM_READ_ONLY_ADDR_START) && (addr <= SVM_READ_ONLY_ADDR_END) &&
        ((addr + len) <= SVM_READ_ONLY_ADDR_END) &&
        (len <= (SVM_READ_ONLY_ADDR_END - SVM_READ_ONLY_ADDR_START)));
}

#define DEVMM_MEM_ATTR_READONLY 0U
#define DEVMM_MEM_ATTR_DVPP     1U
#define DEVMM_MEM_ATTR_DEV      2U
#define DEVMM_MEM_ATTR_TYPE_MAX 3U

int hal_kernel_svm_check_mem_attribute(int devpid, u64 va, u64 size, u32 attribute);
/********************************* svm kernel_api end ***********************************************/

/**
* @ingroup driver
* @brief   This interface is used to init bootdot block
* @param [in]  block_id: block id
* @param [in]  magic: magic
* @param [in]  execption_id: execption id
* @param [in]  expect_status: expect status
* @return   0 Success, others for fail
* @note Support: Ascend310Brc
*/
int bootdot_init_blk(u32 block_id, u32 magic, u32 execption_id, u32 expect_status);

/**
* @ingroup driver
* @brief   This interface is used to set bootdot block
* @param [in]  block_id: block id
* @param [in]  magic: magic
* @param [in]  current_status: current status
* @return   0 Success, others for fail
* @note Support: Ascend310Brc
*/
int bootdot_set_blk(u32 block_id, u32 magic, u32 current_status);

#define DRV_HW_INFO_RESERVED_LEN 16

typedef struct devdrv_base_hw_info {
    u8 chip_id;
    u8 multi_chip;
    u8 multi_die;
    u8 mainboard_id;
    u16 connect_type;
    u16 board_id;
    /* 4 bytes reserved for BIOS; 12 bytes reserved for later */
    u8 reserved[DRV_HW_INFO_RESERVED_LEN];
} devdrv_base_hw_info_t;

typedef struct devdrv_hardware_info {
    unsigned long long phy_addr_offset;
    devdrv_base_hw_info_t base_hw_info;
} devdrv_hardware_info_t;

/**
* @ingroup driver
* @brief   This interface is used to get hardware information
* @param [in]  phy_id : Physical device id
* @param [out]  hardware_info: Hardware information
* @return   0 Success, others for fail
* @note Support: Ascend910B
*/
int hal_kernel_get_hardware_info(unsigned int phy_id, devdrv_hardware_info_t *hardware_info);

#endif
