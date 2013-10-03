/**************************************************************************
 * Copyright 2013, Freescale Semiconductor, Inc. All rights reserved.
 ***************************************************************************/
/*
 * File:       ar_wakeup_tool.c
 *
 * Description: Toolkit to configure Wake-up Rules
 *
 * Authors:      Sunil Kumar Kori <B42948@freescale.com>
 *
 */
/***************************************************************************/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "ar_wakeup_tool.h"

FILE *fp;
char line[100];
uint8_t RuleToConfig = CONFIG_WAKEUP_NULL;
auto_res_port_filtering_e udp_param;
auto_res_port_filtering_e tcp_param;
int32_t ar_wakeup_devfd = -1;

int ar_wakeup_dev_open()
{
	if (system("mknod /dev/ar_wakeup_dev c 104 0") < 0) {
		printf("%s:%s: Unable to create device node\n", __FILE__,
				__FUNCTION__);
		return -1;
	}

	ar_wakeup_devfd = open("/dev/ar_wakeup_dev", O_RDWR);
	if (ar_wakeup_devfd < 0) {
		printf("%s:%s: Unable to open device\n", __FILE__, __FUNCTION__);
		return ar_wakeup_devfd;
	}
	return 0;
}

int ar_wakeup_dev_close()
{
	int     ret;
	ret = close(ar_wakeup_devfd);
	return ret;
}

void ar_wakeup_process_line(FILE *fp, char *line)
{
	if ((!memcmp(line, "#Protocol list\n", strlen(line)))) {
		RuleToConfig = CONFIG_WAKEUP_IP_PROTO;
		return;
	} else if ((!memcmp(line, "#TCP rules\n", strlen(line)))) {
		RuleToConfig = CONFIG_WAKEUP_TCP_RULE;
		return;
	} else if ((!memcmp(line, "#UDP rules\n", strlen(line)))) {
		RuleToConfig = CONFIG_WAKEUP_UDP_RULE;
		return;
	} else if (line[0] == '#')
		return;

	switch (RuleToConfig) {
	case CONFIG_WAKEUP_IP_PROTO:
			ar_wakeup_process_ip_prot(line);
			break;
	case CONFIG_WAKEUP_TCP_RULE:
			ar_wakeup_process_tcp_param(line);
			break;
	case CONFIG_WAKEUP_UDP_RULE:
			ar_wakeup_process_udp_param(line);
			break;
	default:
			printf("File reached End of the file\n");
			break;
	}
	return;
}

void ar_wakeup_process_ip_prot(char *line)
{
	char ip_prot[10];

	memcpy(&ip_prot[0], line, strlen(line));
	ip_prot[strlen(line)] = '\0';
	if (ioctl(ar_wakeup_devfd, (unsigned int)CONFIG_WAKEUP_IP_PROTO,
							ip_prot) < 0) {
		printf("Error in sending IP protocol configuration to FM ucode\n");
		return;
	}
	return;
}

void ar_wakeup_process_tcp_param(char *line)
{
	uint16_t srcport, dstport, srcportmask, dstportmask;
	sscanf(line, "%hu %hu %hx %hx", &srcport, &dstport, &srcportmask,
								&dstportmask);
	tcp_param.src_port = srcport;
	tcp_param.dst_port = dstport;
	tcp_param.src_port_mask = srcportmask;
	tcp_param.dst_port_mask = dstportmask;

	/*Send the same information to AR control module*/
	/*1. Sending tcp ports configuration*/
	if (ioctl(ar_wakeup_devfd, (unsigned int)CONFIG_WAKEUP_TCP_RULE,
							&tcp_param) < 0) {
		printf("Error in sending TCP configuration to FM ucode\n");
		return;
	}
	return;
}

void ar_wakeup_process_udp_param(char *line)
{
	uint16_t srcport, dstport, srcportmask, dstportmask;
	sscanf(line, "%hu %hu %hx %hx", &srcport, &dstport, &srcportmask,
								&dstportmask);
	udp_param.src_port = srcport;
	udp_param.dst_port = dstport;
	udp_param.src_port_mask = srcportmask;
	udp_param.dst_port_mask = dstportmask;

	/*Sending udp ports configuration*/
	if (ioctl(ar_wakeup_devfd, (unsigned int)CONFIG_WAKEUP_UDP_RULE,
							&udp_param) < 0) {
		printf("Error in sending UDP configuration to FM ucode\n");
		return;
	}
	return;
}

void ar_wakeup_process_file(FILE *fp)
{
	while (fgets(line, 100, fp) != NULL) {
		ar_wakeup_process_line(fp, line);
	}
	return;
}

int main(int argc, char *argv[])
{
	int32_t retcode = -1;

	/*Checking input parameters*/
	if (argc == 1) {
		printf("Input file required\n");
		return retcode;
	}
	if (argc > 2) {
		printf("Entered more than one file\n");
		return retcode;
	}

	/* Opening device to communicate with FM ucode */
	retcode = ar_wakeup_dev_open();
	if (retcode < 0) {
		printf("Error in opening the device ar_wakeup_devfd\n");
		return retcode;
	}
	/* Opening input file to read configuration*/
	fp = fopen(argv[1], "r");

	/* Processing on the file */
	ar_wakeup_process_file(fp);

	/* Closing input file */
	fclose(fp);

	/* Closing device */
	retcode = ar_wakeup_dev_close();
	if (retcode < 0) {
		printf("Error in closing the device\n");
		return retcode;
	}

	return 0;
}
