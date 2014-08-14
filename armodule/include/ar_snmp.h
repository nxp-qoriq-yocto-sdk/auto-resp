/**************************************************************************
 * Copyright 2013, Freescale Semiconductor, Inc. All rights reserved.
 ***************************************************************************/
/*
 * File:        ar_snmp.h
 *
 * Description: Common include file for SNMP
 *
 * Authors:      Sunil Kumar Kori <B42948@freescale.com>
 *
 */
/***************************************************************************/
#include <linux/types.h>
#include <linux/sysctl.h>
#include <linux/fs.h>


#define AR_MAX_OID_LENGTH 128
#define AR_SNMP_TABLE_SIZE 64
#define AR_SNMP_MAX_COMM_LENGTH 64
#define AR_MAX_SNMP_MSG_SIZE 1024

typedef enum ar_snmp_tool_event {
	CONFIG_SNMP_PRIV_COMM = 1,
	CONFIG_SNMP_PUB_COMM,
	CONFIG_SNMP_GETALL_FLAG,
	CREATE_SNMP_ENTRY,
	DELETE_SNMP_ENTRY
} ar_snmp_tool_e;

typedef struct auto_res_snmp_entry auto_res_snmp_e;
typedef struct auto_res_snmp_info auto_res_snmp_db;

/*Functions to send/recv the SNMP config*/
uint32_t ar_snmp_init_db(void);
uint32_t ar_snmp_recv_agent_config(unsigned int cmd, auto_res_snmp_e *pInfo);
void ar_snmp_config_comm_str(unsigned int cmd, char *pConfigStr);
void ar_snmp_config_getall_flag_str(char *pConfigStr);
void ar_snmp_send_fmd_config(void);
uint32_t ar_fman_send_config(auto_res_snmp_db *ar_snmp_db);


/*Callback functions registered to the kernel for accessing a device*/
int ar_interface_open(struct inode *inode, struct file *filp);
int ar_interface_release(struct inode *inode, struct file *filp);
long ar_interface_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

