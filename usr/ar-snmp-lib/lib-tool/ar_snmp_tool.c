/**************************************************************************
 * Copyright 2013, Freescale Semiconductor, Inc. All rights reserved.
 ***************************************************************************/
/*
 * File:       ar_snmp_tool.c
 *
 * Description: Toolkit to configure SNMP Objects
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

uint8_t ar_snmp_get_type(unsigned char type)
{
	switch (type) {
	case 'i':
		return ASN_INTEGER;
	case 'u':
	case 'g':
		return ASN_UNSIGNED;
	case 't':
		return ASN_TIMETICKS;
	case 'a':
		return ASN_IPADDRESS;
	case 's':
	case 'x':
	case 'd':
		return ASN_OCTETSTR;
	case 'b':
		return ASN_BITSTR;
	case 'o':
		return ASN_OBJECTID;
	default:
		printf("Invalid type");
		return 0;
	}
}

uint16_t ar_snmp_get_size(unsigned char *value)
{
	uint16_t bit_count = 0, byte_size = 1;
	uint32_t tempvalue = atoi((const char *)value);

	while (tempvalue) {
		tempvalue = tempvalue >> 1;
		bit_count++;
		if (bit_count > 8) {
			byte_size++;
			bit_count = bit_count - 8;
		}
	}
	return byte_size;
}

void ar_snmp_get_val(uint8_t size, uint8_t *tvalue, uint8_t *fvalue)
{
	uint32_t tempval = atoi((const char *)fvalue);
	uint8_t *pvalue;
	pvalue = (uint8_t *)&tempval;
	pvalue = pvalue + (sizeof(uint32_t) - size);
	while (size > 0) {
		*tvalue = *pvalue;
		pvalue++;
		tvalue++;
		size--;
	}
	return;
}

void ar_snmp_atoi(uint8_t *arr)
{
	int32_t i = 0;
	while (arr[i] != '\0') {
		if (arr[i] >= '0' && arr[i] <= '9')
			arr[i] = arr[i] - '0';
		else if (arr[i] >= 'A' && arr[i] <= 'F')
			arr[i] = arr[i] - 'A' + 0x0a;
		else if (arr[i] >= 'a' && arr[i] <= 'f')
			arr[i] = arr[i] - 'a' + 0x0a;
		i++;
	}
	return;

}

void ar_snmp_convert_value(unsigned char *value)
{
	uint8_t idx = 0, *pvalue = value;
	uint32_t size = strlen((const char *)value);

	ar_snmp_atoi(value);
	while (idx < (size - 1)) {
		*value = ((pvalue[idx] << 4) | pvalue[idx + 1]);
		value++;
		idx = idx + 2;
	}
	*value = '\0';
	return;
}

void ar_snmp_convert_bitvalue(uint8_t *value)
{
	uint8_t idx = 0, bit_count = 8, *pvalue = value, tempval = 0;
	uint32_t size = strlen((const char *)value);

	ar_snmp_atoi(value);
	while (idx < size) {
		if (bit_count == 0) {
			*value = tempval;
			tempval = 0;
			idx = idx + 8;
			bit_count = 8;
			value++;
		} else {
			tempval = tempval | (*pvalue << (bit_count - 1));
			pvalue++;
			bit_count--;
		}
	}
	*value = '\0';
	return;
}

void ar_snmp_set_param(unsigned char *objId, unsigned char type,
						unsigned char *value)
{
	/*Parse the value string to get TLV*/
	switch (type) {
	case 'i':
	case 'u':
	case 't':
	case 'g':
		{
				ar_snmp_entry.oidVal = (uint8_t *)malloc(128);
				if (!ar_snmp_entry.oidVal) {
					printf("Memory allocation failed\n");
					return;
				}
				ar_snmp_entry.resVal = (uint8_t *)malloc(132);
				if (!ar_snmp_entry.resVal) {
					free(ar_snmp_entry.oidVal);
					printf("Memory allocation failed\n");
					return;
				}
				ar_snmp_entry.oidSize = strlen((const char *)objId);
				memcpy(ar_snmp_entry.oidVal, objId, ar_snmp_entry.oidSize);
				ar_snmp_entry.oidVal[ar_snmp_entry.oidSize] = '\0';
				ar_snmp_entry.resVal[0]  = ar_snmp_get_type(type);
				ar_snmp_entry.resVal[1]  = ar_snmp_get_size(value);
				ar_snmp_get_val(ar_snmp_entry.resVal[1], &ar_snmp_entry.resVal[2],
										value);
				ar_snmp_entry.resSize = sizeof(ar_snmp_entry.resVal[0])
					+ sizeof(ar_snmp_entry.resVal[1])
					+ ar_snmp_entry.resVal[1];
				ar_snmp_entry.resVal[ar_snmp_entry.resSize]  = '\0';
				break;
		}
	case 'a':
		{
				ar_snmp_entry.oidVal = (uint8_t *)malloc(128);
				if (!ar_snmp_entry.oidVal) {
					printf("Memory allocation failed\n");
					return;
				}
				ar_snmp_entry.resVal = (uint8_t *)malloc(132);
				if (!ar_snmp_entry.resVal) {
					free(ar_snmp_entry.oidVal);
					printf("Memory allocation failed\n");
					return;
				}
				ar_snmp_entry.oidSize = strlen((const char *)objId);
				memcpy(ar_snmp_entry.oidVal, objId, ar_snmp_entry.oidSize);
				ar_snmp_entry.oidVal[ar_snmp_entry.oidSize] = '\0';
				ar_snmp_entry.resVal[0]  = ar_snmp_get_type(type);
				ar_snmp_process_string(value);
				ar_snmp_entry.resVal[1]  = strlen((const char *)value);
				memcpy(&ar_snmp_entry.resVal[2], value, ar_snmp_entry.resVal[1]);
				ar_snmp_entry.resSize = strlen((const char *)ar_snmp_entry.resVal);
				break;
		}
	case 'o':
		{
				ar_snmp_entry.oidVal = (uint8_t *)malloc(128);
				if (!ar_snmp_entry.oidVal) {
					printf("Memory allocation failed\n");
					return;
				}
				ar_snmp_entry.resVal = (uint8_t *)malloc(132);
				if (!ar_snmp_entry.resVal) {
					free(ar_snmp_entry.oidVal);
					printf("Memory allocation failed\n");
					return;
				}
				ar_snmp_entry.oidSize = strlen((const char *)objId);
				memcpy(ar_snmp_entry.oidVal, objId, ar_snmp_entry.oidSize);
				ar_snmp_entry.oidVal[ar_snmp_entry.oidSize] = '\0';
				ar_snmp_entry.resVal[0]  = ar_snmp_get_type(type);
				ar_snmp_process_string(value);
				ar_snmp_entry.resVal[1]  = strlen((const char *)value);
				memcpy(&ar_snmp_entry.resVal[2], value, ar_snmp_entry.resVal[1]);
				ar_snmp_entry.resSize = sizeof(ar_snmp_entry.resVal[0])
					+ sizeof(ar_snmp_entry.resVal[1])
					+ ar_snmp_entry.resVal[1];
				ar_snmp_entry.resVal[ar_snmp_entry.resSize]  = '\0';
				break;
		}
	case 's':
	case 'd':
		{
				ar_snmp_entry.oidVal = (uint8_t *)malloc(128);
				if (!ar_snmp_entry.oidVal) {
					printf("Memory allocation failed\n");
					return;
				}
				ar_snmp_entry.resVal = (uint8_t *)malloc(132);
				if (!ar_snmp_entry.resVal) {
					free(ar_snmp_entry.oidVal);
					printf("Memory allocation failed\n");
					return;
				}
				ar_snmp_entry.oidSize = strlen((const char *)objId);
				memcpy(ar_snmp_entry.oidVal, objId, ar_snmp_entry.oidSize);
				ar_snmp_entry.oidVal[ar_snmp_entry.oidSize] = '\0';
				ar_snmp_entry.resVal[0]  = ar_snmp_get_type(type);
				ar_snmp_entry.resVal[1]  = strlen((const char *)value);
				memcpy(&ar_snmp_entry.resVal[2], value, ar_snmp_entry.resVal[1]);
				ar_snmp_entry.resSize = sizeof(ar_snmp_entry.resVal[0])
					+ sizeof(ar_snmp_entry.resVal[1])
					+ ar_snmp_entry.resVal[1];
				ar_snmp_entry.resVal[ar_snmp_entry.resSize]  = '\0';
				break;
		}
	case 'x':
		{
				ar_snmp_entry.oidVal = (uint8_t *)malloc(128);
				if (!ar_snmp_entry.oidVal) {
					printf("Memory allocation failed\n");
					return;
				}
				ar_snmp_entry.resVal = (uint8_t *)malloc(132);
				if (!ar_snmp_entry.resVal) {
					free(ar_snmp_entry.oidVal);
					printf("Memory allocation failed\n");
					return;
				}
				ar_snmp_entry.oidSize = strlen((const char *)objId);
				memcpy(ar_snmp_entry.oidVal, objId, ar_snmp_entry.oidSize);
				ar_snmp_entry.oidVal[ar_snmp_entry.oidSize] = '\0';
				ar_snmp_entry.resVal[0]  = ar_snmp_get_type(type);
				ar_snmp_convert_value(value);
				ar_snmp_entry.resVal[1]  = strlen((const char *)value);
				memcpy(&ar_snmp_entry.resVal[2], value, ar_snmp_entry.resVal[1]);
				ar_snmp_entry.resSize = sizeof(ar_snmp_entry.resVal[0])
					+ sizeof(ar_snmp_entry.resVal[1])
					+ ar_snmp_entry.resVal[1];
				ar_snmp_entry.resVal[ar_snmp_entry.resSize]  = '\0';
				break;
		}
	case 'b':
		{
				ar_snmp_entry.oidVal = (uint8_t *)malloc(128);
				if (!ar_snmp_entry.oidVal) {
					printf("Memory allocation failed\n");
					return;
				}
				ar_snmp_entry.resVal = (uint8_t *)malloc(132);
				if (!ar_snmp_entry.resVal) {
					free(ar_snmp_entry.oidVal);
					printf("Memory allocation failed\n");
					return;
				}
				ar_snmp_entry.oidSize = strlen((const char *)objId);
				memcpy(ar_snmp_entry.oidVal, objId, ar_snmp_entry.oidSize);
				ar_snmp_entry.oidVal[ar_snmp_entry.oidSize] = '\0';
				ar_snmp_entry.resVal[0]  = ar_snmp_get_type(type);
				ar_snmp_convert_bitvalue(value);
				ar_snmp_entry.resVal[1]  = strlen((const char *)value);
				memcpy(&ar_snmp_entry.resVal[2], value, ar_snmp_entry.resVal[1]);
				ar_snmp_entry.resSize = sizeof(ar_snmp_entry.resVal[0])
					+ sizeof(ar_snmp_entry.resVal[1])
					+ ar_snmp_entry.resVal[1];
				ar_snmp_entry.resVal[ar_snmp_entry.resSize]  = '\0';
				break;
		}
	default:
		{
				printf("Invalid Type\n");
				break;
		}
	}

	return;
}

void ar_snmp_process_string(unsigned char *pString)
{
	unsigned char temp_result[4], oid_result[128];
	unsigned int idx_temp_result = 0, idx_oid_result = 0, idx_pString = 0;

	while (idx_pString < strlen((const char *)pString) + 1) {
		if (pString[idx_pString] != '.' && pString[idx_pString] != '\0') {
			temp_result[idx_temp_result] = pString[idx_pString];
			idx_temp_result++;
			idx_pString++;
		} else {
			temp_result[idx_temp_result] = '\0';
			oid_result[idx_oid_result] = atoi((const char *)temp_result);
			idx_oid_result++;
			idx_pString++;
			idx_temp_result = 0;
		}
	}
	oid_result[idx_oid_result] = '\0';
	memcpy(pString, oid_result, strlen((const char *)oid_result));
	pString[strlen((const char *)oid_result)] = '\0';
	return;
}

void ar_snmp_process_line(unsigned char *pLine)
{
	unsigned char oid[128], type, value[128];
	/*int idx = 0;*/
	int retcode;

	sscanf((const char *)pLine, "%s %c %s", oid, &type, value);
	ar_snmp_process_string(oid);
	ar_snmp_set_param(oid, type, value);
	retcode = ar_snmp_send_ioctl((void *)&ar_snmp_entry, CREATE_SNMP_ENTRY);
	if (retcode < 0) {
		printf("Error in sending the SNMP information\n");
		return;
	}
	free(ar_snmp_entry.oidVal);
	free(ar_snmp_entry.resVal);
	ar_snmp_entry.oidVal = NULL;
	ar_snmp_entry.resVal = NULL;
	return;
}

int main(int argc, char *argv[])
{
	FILE *fp = NULL;
	unsigned char line[512];
	unsigned int retcode;

	if (argc != 2) {
		printf("Invalid input parameter\n");
		return -1;
	}

	/*Opening device to send message to Kernel Space*/
	retcode = ar_snmp_dev_open();
	if (retcode < 0) {
		printf("Error in opening the device\n");
		return retcode;
	}

	/*Opening file in read mode*/
	fp = fopen(argv[1], "r");
	if (fp < 0) {
		printf("Error in opening file\n");
		return -1;
	}
	/*Processing on the file*/
	while (fgets((char *)line, 512, fp) != NULL) {
		if (line[0] != '#')
			ar_snmp_process_line(line);
	}
	/*Closing the file*/
	fclose(fp);

	/*Closing device*/
	retcode = ar_snmp_dev_close();
	if (retcode < 0) {
		printf("Error in closing the device\n");
		return retcode;
	}

	return 0;
}
