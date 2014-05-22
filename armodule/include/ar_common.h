/**************************************************************************
 * Copyright 2013, Freescale Semiconductor, Inc. All rights reserved.
 ***************************************************************************/
/*
 * File:        ar_common.h
 *
 * Description: Common include file
 *
 * Authors:      Sunil Kumar Kori <B42948@freescale.com>
 *
 */
/***************************************************************************/


#include <ncsw_ext.h>
#include <lnxwrp_fsl_fman.h>
#include "ar_snmp.h"
#include "ar_wakeup.h"

#define MINORBITS       20
#define MINORMASK       ((1U << MINORBITS) - 1)

#define MAJOR(dev)      ((unsigned int) ((dev) >> MINORBITS))
#define MINOR(dev)      ((unsigned int) ((dev) & MINORMASK))

#define AR_ENABLE	1
#define AR_DISABLE	0

/* Common Return status used between SW stack and FM ucode*/
typedef enum {
	AR_SUCCESS = 0,
	AR_CB_RGSTR_ERROR,
	AR_TBL_ALLOC_ERROR,
	AR_MAX_LIMIT_ERROR,
	AR_INIT_ERROR,
	AR_MEMORY_UNAVALABLE_ERROR,
	AR_GET_INFO_ERROR
} ar_ret_status;

int32_t ar_intf_init_db(void);

void ar_process_deep_sleep_event (uint32_t  *out_status);

void ar_process_resume_event(int32_t  *out_status);

struct net_device *ar_get_device(void);

uint16_t ar_get_vlanid(struct net_device *dev);

int  ar_get_interface_info(struct net_device *netdev);

void ar_get_fm_port(struct fm_port **fm_rxport,
		struct fm_port **fm_txport);

void ar_get_fman_stats(void);

void ar_interface_info_dump(void);

void  ar_proc_exit(void);

int  ar_proc_init(void);

ssize_t ar_handle_deep_sleep_event(struct file *file, const char __user *buffer,
		size_t count, loff_t *data);

extern int32_t ar_wakeup_src;

/*Macro for debug print*/
#define PRINT_INFO(fmt) printk("[%s:%d]:%s", __FUNCTION__, __LINE__, fmt)
