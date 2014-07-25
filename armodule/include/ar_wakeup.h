/**************************************************************************
 * Copyright 2013, Freescale Semiconductor, Inc. All rights reserved.
 ***************************************************************************/
/*
 * File:        ar_wakeup.h
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
#include <asm-generic/ioctl.h>

/***************************Macro Definition********************************/
#define IP_PROTOCOL_NAME_LENGTH		25
#define IP_PROTOCOL_DB_SIZE		255
/***************************************************************************/

typedef enum ar_wakeup_tool_event {
	CONFIG_WAKEUP_NULL = 0x0,
	CONFIG_WAKEUP_UDP_RULE = 0x1,
	CONFIG_WAKEUP_TCP_RULE = 0x2,
	CONFIG_WAKEUP_TCP_MASK = 0x03,
	CONFIG_WAKEUP_IP_PROTO = 0x04,
} ar_wakeup_tool_e;

typedef struct auto_res_port_filtering_entry auto_res_port_filtering_e;
typedef struct auto_res_filtering_info auto_res_filtering_db;

struct prot {
	uint8_t prot_name[IP_PROTOCOL_NAME_LENGTH];
	uint8_t prot_code;
};

uint32_t ar_wakeup_init_db(void);
int32_t ar_wakeup_get_free_index(char *prot);
int32_t ar_wakeup_get_index(void *pInfo, char *prot);
int32_t ar_wakeup_recv_config(unsigned int cmd, void *pInfo);
uint8_t ar_wakeup_get_prot_code(uint8_t *pProtName);
uint16_t ar_wakeup_atoi(char *pArg);

/*Callback functions registered to the kernel for accessing a device*/
int ar_wakeup_interface_open(struct inode *inode, struct file *filp);
int ar_wakeup_interface_release(struct inode *inode, struct file *filp);
long ar_wakeup_interface_ioctl(struct file *filp, unsigned int cmd,
							unsigned long arg);
