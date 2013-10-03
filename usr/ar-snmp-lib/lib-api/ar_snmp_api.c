/**************************************************************************
 * Copyright 2013, Freescale Semiconductor, Inc. All rights reserved.
 ***************************************************************************/
/*
 * File:       ar_snmp_api.c
 *
 * Description: Library to configure SNMP Objects
 *
 * Authors:      Sunil Kumar Kori <B42948@freescale.com>
 *
 */
/***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdint.h>
#include "../ar_snmp_tool.h"

auto_res_snmp_e ar_snmp_entry;
int ar_devfd = -1;

void ar_snmp_agent_get_pdu(auto_res_snmp_e *snmp_config)
{
	unsigned int retcode = 0;

	/*Initialize the snmp entry*/
	ar_snmp_entry.oidVal = snmp_config->oidVal;
	ar_snmp_entry.resVal = snmp_config->resVal;
	ar_snmp_entry.oidSize = snmp_config->oidSize; 
	ar_snmp_entry.resSize = snmp_config->resSize;

	/*Opening device to send message to Kernel Space*/
	retcode = ar_snmp_dev_open();
	if (retcode < 0) {
		printf("Error in opening the device\n");
		return;
	}

	/*Sending SNMP configuration to AR control module*/
	retcode = ar_snmp_send_ioctl((void *)&ar_snmp_entry, CREATE_SNMP_ENTRY);
	if (retcode < 0)
		printf("Error in sending the SNMP information\n");

	/*Closing device*/
	retcode = ar_snmp_dev_close();
	if (retcode < 0)
		printf("Error in closing the device\n");

	return;
}

void ar_snmp_agent_get_comm(uint8_t *comm_name, uint32_t cmd)
{
	unsigned int retcode = 0;

	/*Opening device to send message to Kernel Space*/
	retcode = ar_snmp_dev_open();
	if (retcode < 0) {
		printf("Error in opening the device\n");
		return;
	}
	/*Sending community name to AR control module*/
	retcode = ar_snmp_send_ioctl((void *)comm_name, cmd); 
	if (retcode < 0)
		printf("Error in sending the community name\n");

	/*Closing device*/
	retcode = ar_snmp_dev_close();
	if (retcode < 0)
		printf("Error in closing the device\n");

	return;
}


int ar_snmp_dev_open()
{
	if (system("mknod /dev/ar_dev c 103 0") < 0) {
		printf("%s:%s: Unable to create device node\n", __FILE__,
				__FUNCTION__);
		return -1;
	}

	ar_devfd = open("/dev/ar_dev", O_RDWR);
	if (ar_devfd < 0) {
		printf("%s:%s: Unable to open device\n", __FILE__, __FUNCTION__);
		return ar_devfd;
	}
	return 0;
}

int ar_snmp_dev_close()
{
	int	ret;
	ret = close(ar_devfd);
	return ret;
}

int ar_snmp_send_ioctl(void *ar_snmp_config, uint32_t cmd)
{
	if (ioctl(ar_devfd, cmd, ar_snmp_config) < 0)
		return -1;
	return 0;
}
