/**************************************************************************
 * Copyright 2013, Freescale Semiconductor, Inc. All rights reserved.
 ***************************************************************************/
/*
 * File:        arctrl_wakeup.c
 *
 * Description: Module to support SNMP in Auto Response.
 *
 * Authors:      Sunil Kumar Kori <B42948@freescale.com>
 *
 */
/***************************************************************************/
#include <linux/kernel.h>
#include <linux/slab.h>
#include "ar_common.h"


/*Wake-up database which is to be sent to FM Ucode*/
uint32_t ar_wakeuptblsize = AR_WAKEUP_TABLE_SIZE;
auto_res_port_filtering_e ar_wakeup_udp_entity[AR_WAKEUP_TABLE_SIZE];
auto_res_port_filtering_e ar_wakeup_tcp_entity[AR_WAKEUP_TABLE_SIZE];
uint8_t ar_wakeup_ip_prot_tbl[AR_WAKEUP_TABLE_SIZE];
auto_res_filtering_db ar_wakeup_db;

/*Name-Protocol mapping*/
struct prot prot_info[AR_WAKEUP_TABLE_SIZE] = {{"ICMP\n", 0x01},
						{"IGMP\n", 0x02} };


uint32_t ar_wakeup_init_db()
{
	ar_wakeup_db.ip_prot_table_ptr = ar_wakeup_ip_prot_tbl;
	ar_wakeup_db.udp_ports_table_ptr = ar_wakeup_udp_entity;
	ar_wakeup_db.tcp_ports_table_ptr = ar_wakeup_tcp_entity;
	return AR_SUCCESS;
}

int32_t ar_wakeup_get_free_index(char *prot)
{
	int32_t index;

	if (!strcmp(prot, "UDP")) {
		for (index = 0; index < ar_wakeuptblsize; index++) {
			if (ar_wakeup_udp_entity[index].src_port == 0x0
				&& ar_wakeup_udp_entity[index].dst_port == 0x0)
					return index;
		}
	} else if (!strcmp(prot, "TCP")) {
		for (index = 0; index < ar_wakeuptblsize; index++) {
			if (ar_wakeup_tcp_entity[index].src_port == 0x0
				&& ar_wakeup_tcp_entity[index].dst_port == 0x0)
					return index;
		}
	} else if (!strcmp(prot, "IP_PROT")) {
		for (index = 0; index < ar_wakeuptblsize; index++) {
			if (ar_wakeup_ip_prot_tbl[index] == 0x00)
				return index;
		}
	}
	return -1;
}

uint8_t ar_wakeup_get_prot_code(uint8_t *pProtName)
{
	uint32_t index;
	for (index = 0; index < ar_wakeuptblsize; index++) {
		if (!strncmp(pProtName, prot_info[index].prot_name,
						strlen(pProtName)))
			return prot_info[index].prot_code;
	}
	return -1;
}

int32_t ar_wakeup_get_index(void *pConfig, char *prot)
{
	int32_t index;

	if (!strcmp(prot, "UDP")) {
		auto_res_port_filtering_e *pInfo =
				(auto_res_port_filtering_e *)pConfig;
		for (index = 0; index < ar_wakeup_db.udp_ports_table_size; index++) {
			if (ar_wakeup_udp_entity[index].src_port == pInfo->src_port
			   && ar_wakeup_udp_entity[index].dst_port == pInfo->dst_port
			   && ar_wakeup_udp_entity[index].src_port_mask == pInfo->src_port_mask
			   && ar_wakeup_udp_entity[index].dst_port_mask == pInfo->dst_port_mask)
				return index;
		}
	} else if (!strcmp(prot, "TCP")) {
		auto_res_port_filtering_e *pInfo =
				(auto_res_port_filtering_e *)pConfig;
		for (index = 0; index < ar_wakeup_db.tcp_ports_table_size; index++) {
			if (ar_wakeup_tcp_entity[index].src_port == pInfo->src_port
			&& ar_wakeup_tcp_entity[index].dst_port == pInfo->dst_port
			&& ar_wakeup_tcp_entity[index].src_port_mask == pInfo->src_port_mask
			&& ar_wakeup_tcp_entity[index].dst_port_mask == pInfo->dst_port_mask)
				return index;
		}
	} else if (!strcmp(prot, "IP_PROT")) {
		uint8_t prot_code = ar_wakeup_get_prot_code((uint8_t *)pConfig);
		if (prot_code < 0) {
			PRINT_INFO("Protocol code does not exist\n");
			return prot_code;
		}
		for (index = 0; index < ar_wakeup_db.ip_prot_table_size; index++) {
			if (ar_wakeup_ip_prot_tbl[index] == prot_code)
				return index;
		}
	}
	return -1;
}

int32_t ar_wakeup_recv_config(unsigned int cmd, void *pConfig)
{
	int32_t retVal;

	if (NULL == pConfig) {
		PRINT_INFO("Received no information from SNMP Agent\n");
		return -AR_GET_INFO_ERROR;
	}
	switch (cmd) {
	case CONFIG_WAKEUP_IP_PROTO:
		{
			uint8_t *p_ip_prot = (uint8_t *)pConfig;
			uint8_t prot_code = ar_wakeup_get_prot_code(p_ip_prot);
			if (prot_code < 0) {
				PRINT_INFO("Protocol code does not exist\n");
				return prot_code;
			}

			/*Check for the existing entry.
			If yes then return with success*/
			retVal = ar_wakeup_get_index(p_ip_prot, "IP_PROT");
			if (retVal != -1) {
				ar_wakeup_ip_prot_tbl[retVal] = prot_code;
				break;
			}
			/*If No, look for the empty location
			If location found then update and if DB is full then
			then return an error.*/
			else {
				retVal = ar_wakeup_get_free_index("IP_PROT");
				if (retVal != -1) {
					ar_wakeup_ip_prot_tbl[retVal] = prot_code;
					ar_wakeup_db.ip_prot_table_size++;
				} else
					PRINT_INFO("No space available in DB\n");
				break;
			}
			return -1;
		}
	case CONFIG_WAKEUP_UDP_RULE:
		{
			auto_res_port_filtering_e *pInfo =
					(auto_res_port_filtering_e *)pConfig;
			/*Check for existing entry*/
			retVal = ar_wakeup_get_index(pInfo, "UDP");
			if (retVal != -1) {
				ar_wakeup_udp_entity[retVal].src_port =
								pInfo->src_port;
				ar_wakeup_udp_entity[retVal].dst_port =
								pInfo->dst_port;
				ar_wakeup_udp_entity[retVal].src_port_mask =
								pInfo->src_port_mask;
				ar_wakeup_udp_entity[retVal].dst_port_mask =
								pInfo->dst_port_mask;
				break;
			} else {
				retVal = ar_wakeup_get_free_index("UDP");
				if (retVal != -1) {
					ar_wakeup_udp_entity[retVal].src_port =
								pInfo->src_port;
					ar_wakeup_udp_entity[retVal].dst_port =
								pInfo->dst_port;
					ar_wakeup_udp_entity[retVal].src_port_mask =
								pInfo->src_port_mask;
					ar_wakeup_udp_entity[retVal].dst_port_mask =
								pInfo->dst_port_mask;
					ar_wakeup_db.udp_ports_table_size++;
					break;
				}

			}
				return -1;
		}
	case CONFIG_WAKEUP_TCP_RULE:
		{
			auto_res_port_filtering_e *pInfo =
					(auto_res_port_filtering_e *)pConfig;

			/*Check for existing entry*/
			retVal = ar_wakeup_get_index(pInfo, "TCP");
			if (retVal != -1) {
				ar_wakeup_tcp_entity[retVal].src_port =
								pInfo->src_port;
				ar_wakeup_tcp_entity[retVal].dst_port =
								pInfo->dst_port;
				ar_wakeup_tcp_entity[retVal].src_port_mask =
								pInfo->src_port_mask;
				ar_wakeup_tcp_entity[retVal].dst_port_mask =
								pInfo->dst_port_mask;
				break;
			} else {
				retVal = ar_wakeup_get_free_index("TCP");
				if (retVal != -1) {
					ar_wakeup_tcp_entity[retVal].src_port =
									pInfo->src_port;
					ar_wakeup_tcp_entity[retVal].dst_port =
									pInfo->dst_port;
					ar_wakeup_tcp_entity[retVal].src_port_mask =
									pInfo->src_port_mask;
					ar_wakeup_tcp_entity[retVal].dst_port_mask =
									pInfo->dst_port_mask;
					ar_wakeup_db.tcp_ports_table_size++;
					break;
				}
			}
			return -1;
		}
	default:
		{
#ifdef AR_DEBUG
			PRINT_INFO("Wrong command type\n");
#endif
			break;
		}

	}
	return AR_SUCCESS;
}
