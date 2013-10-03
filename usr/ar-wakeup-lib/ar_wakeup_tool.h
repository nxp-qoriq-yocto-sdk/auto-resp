/**************************************************************************
 * Copyright 2013, Freescale Semiconductor, Inc. All rights reserved.
 ***************************************************************************/
/*
 * File:       ar_wakeup_tool.h
 *
 * Description:
 *
 * Authors:      Sunil Kumar Kori <B42948@freescale.com>
 *
 */
/***************************************************************************/
#include <linux/types.h>
#include <asm-generic/ioctl.h>

/*! Filename used to access device by Linux */
#define DEV_MOUNT_PATH   "/dev/ar_wakeup_dev"


typedef struct auto_res_port_filtering_entry {
	uint16_t    src_port;
	uint16_t    dst_port;
	uint16_t    src_port_mask;
	uint16_t    dst_port_mask;
} auto_res_port_filtering_e;

typedef enum ar_wakeup_tool_event {
	CONFIG_WAKEUP_NULL = 0x0,
	CONFIG_WAKEUP_UDP_RULE = 0x1,
	CONFIG_WAKEUP_TCP_RULE = 0x2,
	CONFIG_WAKEUP_TCP_MASK = 0x3,
	CONFIG_WAKEUP_IP_PROTO = 0x4,
} ar_wakeup_tool_e;

void ar_wakeup_process_file(FILE *fp);
void ar_wakeup_process_line(FILE *fp, char *line);
void ar_wakeup_process_ip_prot(char *line);
void ar_wakeup_process_tcp_param(char *line);
void ar_wakeup_process_udp_param(char *line);
int ar_wakeup_dev_open(void);
int ar_wakeup_dev_close(void);
