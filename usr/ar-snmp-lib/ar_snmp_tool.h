/**************************************************************************
 * Copyright 2013, Freescale Semiconductor, Inc. All rights reserved.
 ***************************************************************************/
/*
 * File:       ar_snmp_tool.h
 *
 * Description:
 *
 * Authors:      Sunil Kumar Kori <B42948@freescale.com>
 *
 */
/***************************************************************************/

/*! Filename used to access device by Linux */
#define DEV_MOUNT_PATH   "/dev/ar_dev"

/* ASN DataTypes*/
#define ASN_APPLICATION		((u_char)0x40)
#define ASN_UNIVERSAL		((u_char)0x00)
#define ASN_CONTEXT		((u_char)0x80)
#define ASN_PRIVATE		((u_char)0xC0)

#define ASN_BOOLEAN		((u_char)0x01)
#define ASN_INTEGER		((u_char)0x02)
#define ASN_BITSTR		((u_char)0x03)
#define ASN_OCTETSTR		((u_char)0x04)
#define ASN_NULL		((u_char)0x05)
#define ASN_OBJECTID		((u_char)0x06)
#define ASN_SEQUENCE		((u_char)0x10)
#define ASN_SET			((u_char)0x11)
#define ASN_IPADDRESS		(ASN_APPLICATION | (u_char)0x0)
#define ASN_COUNTER		(ASN_APPLICATION | (u_char)0x1)
#define ASN_GAUGE		(ASN_APPLICATION | (u_char)0x2)
#define ASN_UNSIGNED		(ASN_APPLICATION | (u_char)0x2)
#define ASN_TIMETICKS		(ASN_APPLICATION | (u_char)0x3)
#define ASN_OPAQUE		(ASN_APPLICATION | (u_char)0x4)
#define ASN_NSAP		(ASN_APPLICATION | (u_char)0x5)
#define ASN_COUNTER64		(ASN_APPLICATION | (u_char)0x6)
#define ASN_UINTEGER		(ASN_APPLICATION | (u_char)0x7)


typedef enum ar_snmp_tool_event {
	CONFIG_SNMP_PRIV_COMM = 1,
	CONFIG_SNMP_PUB_COMM,
	CONFIG_SNMP_GETALL_FLAG,
	CREATE_SNMP_ENTRY,
	DELETE_SNMP_ENTRY
} ar_snmp_tool_e;

typedef struct auto_res_snmp_entry {
	uint16_t	oidSize;
	uint8_t		*oidVal;
	uint16_t	resSize;
	/*Response result "Type|Length|Value"*/
	uint8_t		*resVal;
} auto_res_snmp_e;

void ar_snmp_process_string(unsigned char *pString);

void ar_snmp_get_agent_config(auto_res_snmp_e *snmp_config);

int ar_snmp_dev_open();

int ar_snmp_dev_close();

int ar_snmp_send_ioctl(void *ar_snmp_config, uint32_t cmd);
