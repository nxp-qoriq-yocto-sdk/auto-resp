/**************************************************************************
 * Copyright 2013, Freescale Semiconductor, Inc. All rights reserved.
 ***************************************************************************/
/*
 * File:        arctrl_snmp.c
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
uint32_t ar_snmp_tblsize;

/*SNMP database which is to be sent to FM Ucode*/
auto_res_snmp_e *ar_snmp_entity;
auto_res_snmp_db ar_snmp_db;

uint32_t ar_snmp_init_db()
{
	char private_str[] = "private";
	char public_str[] = "public";
	uint32_t priv_str_len, pub_str_len;

	ar_snmp_tblsize = p_ar_maxsize->max_num_of_snmp_oid_entries;
	ar_snmp_entity = (auto_res_snmp_e *)kzalloc((sizeof(auto_res_snmp_e) * ar_snmp_tblsize),
							GFP_KERNEL);

	memset(&ar_snmp_db, 0, sizeof(ar_snmp_db));
	memset(ar_snmp_entity, 0, sizeof(auto_res_snmp_e) * ar_snmp_tblsize);
	ar_snmp_db.community_read_write_string = (uint8_t *)kzalloc(16, GFP_KERNEL);
	if (!ar_snmp_db.community_read_write_string) {
		PRINT_INFO("Memory Allocation failed\n");
		return -AR_SUCCESS;
	}
	ar_snmp_db.community_read_only_string = (uint8_t *)kzalloc(16, GFP_KERNEL);
	if (!ar_snmp_db.community_read_only_string) {
		kfree(ar_snmp_db.community_read_write_string);
		PRINT_INFO("Memory Allocation failed\n");
		return -AR_SUCCESS;
	}
	priv_str_len = strlen(private_str);
	pub_str_len = strlen(public_str);

	memcpy(ar_snmp_db.community_read_write_string, private_str, priv_str_len);
	memcpy(ar_snmp_db.community_read_only_string, public_str, pub_str_len);
	ar_snmp_db.community_read_write_string[priv_str_len] = '\0';
	ar_snmp_db.community_read_only_string[pub_str_len] = '\0';
	ar_snmp_db.control = 0;
	ar_snmp_db.max_snmp_msg_length = AR_MAX_SNMP_MSG_SIZE;
	ar_snmp_db.oid_table = ar_snmp_entity;
	return AR_SUCCESS;
}

uint32_t ar_snmp_get_free_index(auto_res_snmp_e *pInfo)
{
	int index;
	for (index = 0; index < ar_snmp_tblsize; index++) {
		if (ar_snmp_entity[index].oidVal == NULL)
			return index;
	}
	return -1;
}

uint32_t ar_snmp_get_index(auto_res_snmp_e *pInfo)
{
	int index;
	for (index = 0; index < ar_snmp_db.oid_table_size; index++) {
		if (!(strcmp(pInfo->oidVal, ar_snmp_entity[index].oidVal)))
			return index;
	}
	return -1;
}

uint32_t ar_snmp_recv_agent_config(unsigned int cmd, auto_res_snmp_e *pInfo)
{
	uint32_t retVal;
	if (NULL == pInfo) {
		PRINT_INFO("Received no information from SNMP Agent\n");
		return -AR_SUCCESS;
	}
	switch (cmd) {
	case CREATE_SNMP_ENTRY:
		{
			/*Check for existing entry*/
			retVal = ar_snmp_get_index(pInfo);
			if (retVal != -1) {
				ar_snmp_entity[retVal].oidSize = pInfo->oidSize;
				ar_snmp_entity[retVal].oidVal  = pInfo->oidVal;
				ar_snmp_entity[retVal].resSize = pInfo->resSize;
				ar_snmp_entity[retVal].resVal  = pInfo->resVal;
				break;
			} else {
				retVal = ar_snmp_get_free_index(pInfo);
				if (retVal != -1) {
					ar_snmp_entity[retVal].oidSize = pInfo->oidSize;
					ar_snmp_entity[retVal].oidVal  = pInfo->oidVal;
					ar_snmp_entity[retVal].resSize = pInfo->resSize;
					ar_snmp_entity[retVal].resVal  = pInfo->resVal;
					ar_snmp_db.oid_table_size++;
					break;
				} else
					PRINT_INFO("Maximum limit reached for SNMP entries\n");
			}
			return -AR_SUCCESS;
		}
	case DELETE_SNMP_ENTRY:
		{
			/*Get the entry which is to be deleted*/
			retVal = ar_snmp_get_index(pInfo);
			if (retVal != -1) {
				while (retVal < ar_snmp_db.oid_table_size - 1) {
					ar_snmp_entity[retVal].oidSize =
						ar_snmp_entity[retVal + 1].oidSize;
					ar_snmp_entity[retVal].resSize =
						ar_snmp_entity[retVal + 1].resSize;
					ar_snmp_entity[retVal].oidVal =
						ar_snmp_entity[retVal + 1].oidVal;
					ar_snmp_entity[retVal].resVal =
						ar_snmp_entity[retVal + 1].resVal;
					retVal++;
				} 
				ar_snmp_entity[retVal].oidSize = 0;
				ar_snmp_entity[retVal].resSize = 0;
				kfree(ar_snmp_entity[retVal].oidVal);
				kfree(ar_snmp_entity[retVal].resVal);
				ar_snmp_entity[retVal].oidVal = NULL;
				ar_snmp_entity[retVal].resVal = NULL;
				ar_snmp_db.oid_table_size--;
				break;
			}
			return -AR_SUCCESS;
		}
	default:
		{
			PRINT_INFO("Wrong command type\n");
			break;
		}
	}
	return AR_SUCCESS;
}


void ar_snmp_config_getall_flag_str(char *pConfigStr)
{
	ar_snmp_db.control = *pConfigStr-'0';
	kfree(pConfigStr);
	return;
}

void ar_snmp_config_comm_str(unsigned int cmd, char *pConfigStr)
{

	switch (cmd) {
	case CONFIG_SNMP_PRIV_COMM:
		{
			if (ar_snmp_db.community_read_write_string) {
				kzfree(ar_snmp_db.community_read_write_string);
				ar_snmp_db.community_read_write_string = NULL;
			}
			ar_snmp_db.community_read_write_string = pConfigStr;
			ar_snmp_db.community_read_write_string[strlen(pConfigStr)] = '\0';
			break;
		}
	case CONFIG_SNMP_PUB_COMM:
		{
			if (ar_snmp_db.community_read_only_string) {
				kzfree(ar_snmp_db.community_read_only_string);
				ar_snmp_db.community_read_only_string = NULL;
			}
			ar_snmp_db.community_read_only_string = pConfigStr;
			ar_snmp_db.community_read_only_string[strlen(pConfigStr)] = '\0';
			break;
		}
	default:
		{
			PRINT_INFO("Invalid cmd\n");
			break;
		}
	}
	return;
}
