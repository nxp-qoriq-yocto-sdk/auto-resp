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
#include <dpaa_eth.h>
#include "ar_common.h"

extern struct auto_res_tables_sizes *p_ar_maxsize;

/*Wake-up database which is to be sent to FM Ucode*/
uint32_t ar_wakeuptblsize;
uint32_t udptblsize, tcptblsize, iptblsize;
auto_res_port_filtering_e *ar_wakeup_udp_entity;
auto_res_port_filtering_e *ar_wakeup_tcp_entity;
uint8_t *ar_wakeup_ip_prot_tbl;
auto_res_filtering_db ar_wakeup_db;

/*Name-Protocol mapping*/
struct prot prot_info[IP_PROTOCOL_DB_SIZE] = {
	{"HOPOPT\n",	0x00}, {"ICMP\n",	0x01}, {"IGMP\n",	0x02},
	{"GGP\n",	0x03}, {"IP-in-IP\n",	0x04}, {"ST\n",		0x05},
	{"TCP\n",	0x06}, {"CBT\n",	0x07}, {"EGP\n",	0x08},
	{"IGP\n",	0x09}, {"BBN-RCC-MON\n",	0x0A}, {"NVP-II\n",	0x0B},
	{"PUP\n",	0x0C}, {"ARGUS\n",	0x0D}, {"EMCON\n",	0x0E},
	{"XNET\n",	0x0F}, {"CHAOS\n",	0x10}, {"UDP\n",	0x11},
	{"MUX\n",	0x12}, {"DCN-MEAS\n",	0x13}, {"HMP\n",	0x14},
	{"PRM\n",	0x15}, {"XNS-IDP\n",	0x16}, {"TRUNK-1\n",	0x17},
	{"TRUNK-2\n",	0x18}, {"LEAF-1\n",	0x19}, {"LEAF-2\n",	0x1A},
	{"RDP\n",	0x1B}, {"IRTP\n",	0x1C}, {"ISO-TP4\n",	0x1D},
	{"NETBLT\n",	0x1E}, {"MFE-NSP\n",	0x1F}, {"MERIT-INP\n",	0x20},
	{"DCCP\n",	0x21}, {"3PC\n",	0x22}, {"IDPR\n",	0x23},
	{"XTP\n",	0x24}, {"DDP\n",	0x25}, {"IDPR-CMTP\n",	0x26},
	{"TP++\n",	0x27}, {"IL\n",		0x28}, {"IPv6\n",	0x29},
	{"SDRP\n",	0x2A}, {"IPv6-Route\n",	0x2B}, {"IPv6-Frag\n",	0x2C},
	{"IDRP\n",	0x2D}, {"RSVP\n",	0x2E}, {"GRE\n",	0x2F},
	{"MHRP\n",	0x30}, {"BNA\n",	0x31}, {"ESP\n",	0x32},
	{"AH\n",	0x33}, {"I-NLSP\n",	0x34}, {"SWIPE\n",	0x35},
	{"NARP\n",	0x36}, {"MOBILE\n",	0x37}, {"TLSP\n",	0x38},
	{"SKIP\n",	0x39}, {"IPv6-ICMP\n",	0x3A}, {"IPv6-NoNxt\n",	0x3B},
	{"IPv6-Opts\n",	0x3C}, {"CFTP\n",	0x3E}, {"SAT-EXPAK\n",	0x40},
	{"KRYPTOLAN\n",	0x41}, {"RVD\n",	0x42}, {"IPPC\n",	0x43},
	{"SAT-MON\n",	0x45}, {"VISA\n",	0x46}, {"IPCU\n",	0x47},
	{"CPNX\n",	0x48}, {"CPHB\n",	0x49}, {"WSN\n",	0x4A},
	{"PVP\n",	0x4B}, {"BR-SAT-MON\n",	0x4C}, {"SUN-ND\n",	0x4D},
	{"WB-MON\n",	0x4E}, {"WB-EXPAK\n",	0x4F}, {"ISO-IP\n",	0x50},
	{"VMTP\n",	0x51}, {"SECURE-VMTP\n",	0x52}, {"VINES\n",	0x53},
	{"TTP\n",	0x54}, {"IPTM\n",	0x54}, {"NSFNET-IGP\n",	0x55},
	{"DGP\n",	0x56}, {"TCF\n",	0x57}, {"EIGRP\n",	0x58},
	{"OSPF\n",	0x59}, {"Sprite-RPC\n",	0x5A}, {"LARP\n",	0x5B},
	{"MTP\n",	0x5C}, {"AX.25\n",	0x5D}, {"IPIP\n",	0x5E},
	{"MICP\n",	0x5F}, {"SCC-SP\n",	0x60}, {"ETHERIP\n",	0x61},
	{"ENCAP\n",	0x62}, {"GMTP\n",	0x64}, {"IFMP\n",	0x65},
	{"PNNI\n",	0x66}, {"PIM\n",	0x67}, {"ARIS\n",	0x68},
	{"SCPS\n",	0x69}, {"QNX\n",	0x6A}, {"A/N\n",	0x6B},
	{"IPComp\n",	0x6C}, {"SNP\n",	0x6D}, {"Compaq-Peer\n",	0x6E},
	{"IPX-in-IP\n",	0x6F}, {"VRRP\n",	0x70}, {"PGM\n",	0x71},
	{"L2TP\n",	0x73}, {"DDX\n",	0x74}, {"IATP\n",	0x75},
	{"STP\n",	0x76}, {"SRP\n",	0x77}, {"UTI\n",	0x78},
	{"SMP\n",	0x79}, {"SM\n",		0x7A}, {"PTP\n",	0x7B},
	{"IS-IS over IPv4\n",	0x7C}, {"FIRE\n",	0x7D}, {"CRTP\n",	0x7E},
	{"CRUDP\n",	0x7F}, {"SSCOPMCE\n",	0x80}, {"IPLT\n",	0x81},
	{"SPS\n",	0x82}, {"PIPE\n",	0x83}, {"SCTP\n",	0x84},
	{"FC\n",	0x85}, {"RSVP-E2E-IGNORE\n",	0x86}, {"Mobility Header\n",	0x87},
	{"UDPLite\n",	0x88}, {"MPLS-in-IP\n",	0x89}, {"manet\n",	0x8A},
	{"HIP\n",	0x8B}, {"Shim6\n",	0x8C}, {"WESP\n",	0x8D},
	{"ROHC\n",	0x8E}
};

uint32_t ar_wakeup_init_db()
{
	ar_wakeuptblsize = p_ar_maxsize->max_num_of_ip_prot_filtering;
	udptblsize = p_ar_maxsize->max_num_of_udp_port_filtering;
	tcptblsize = p_ar_maxsize->max_num_of_tcp_port_filtering;
	iptblsize  = p_ar_maxsize->max_num_of_ip_prot_filtering;

	ar_wakeup_udp_entity = (auto_res_port_filtering_e *)kzalloc((sizeof(auto_res_port_filtering_e) *
							udptblsize), GFP_KERNEL);
	if (!ar_wakeup_udp_entity) {
		PRINT_INFO("Error in allocating the space\n");
		return -AR_MEMORY_UNAVALABLE_ERROR;
	}
	ar_wakeup_tcp_entity = (auto_res_port_filtering_e *)kzalloc((sizeof(auto_res_port_filtering_e) *
							tcptblsize), GFP_KERNEL);
	if (!ar_wakeup_tcp_entity) {
		PRINT_INFO("Error in allocating the space\n");
		kzfree(ar_wakeup_udp_entity);
		return -AR_MEMORY_UNAVALABLE_ERROR;
	}

	ar_wakeup_ip_prot_tbl = (uint8_t *)kzalloc((sizeof(uint8_t) * iptblsize), GFP_KERNEL);
	if (!ar_wakeup_ip_prot_tbl) {
		PRINT_INFO("Error in allocating the space\n");
		kzfree(ar_wakeup_tcp_entity);
		kzfree(ar_wakeup_udp_entity);
		return -AR_MEMORY_UNAVALABLE_ERROR;
	}

	ar_wakeup_db.ip_prot_table_ptr = ar_wakeup_ip_prot_tbl;
	ar_wakeup_db.udp_ports_table_ptr = ar_wakeup_udp_entity;
	ar_wakeup_db.tcp_ports_table_ptr = ar_wakeup_tcp_entity;
	return AR_SUCCESS;
}

int32_t ar_wakeup_get_free_index(char *prot)
{
	int32_t index;

	if (!strcmp(prot, "UDP")) {
		for (index = 0; index < udptblsize; index++) {
			if (ar_wakeup_udp_entity[index].src_port == 0x0
				&& ar_wakeup_udp_entity[index].dst_port == 0x0)
					return index;
		}
	} else if (!strcmp(prot, "TCP")) {
		for (index = 0; index < tcptblsize; index++) {
			if (ar_wakeup_tcp_entity[index].src_port == 0x0
				&& ar_wakeup_tcp_entity[index].dst_port == 0x0)
					return index;
		}
	} else if (!strcmp(prot, "IP_PROT")) {
		for (index = 0; index < iptblsize; index++) {
			if (ar_wakeup_ip_prot_tbl[index] == 0x00)
				return index;
		}
	}
	return -1;
}

uint8_t ar_wakeup_get_prot_code(uint8_t *pProtName)
{
	uint32_t index;
	for (index = 0; index < IP_PROTOCOL_DB_SIZE; index++) {
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
