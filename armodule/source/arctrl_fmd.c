/**************************************************************************
 * Copyright 2013, Freescale Semiconductor, Inc. All rights reserved.
 ***************************************************************************/
/*
 * File:        arctrl_fmd.c
 *
 * Description: Control module of Auto Response as interface with FMD.
 *
 * Authors:     Sunil Kumar Kori <B42948@freescale.com>
 *
 */
/***************************************************************************/
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/module.h>
#include <mac.h>
#include <dpaa_eth.h>
#include "ar_common.h"

/*Extern definitions*/
extern uint32_t ar_arptblsize;
extern uint32_t ar_ndptblsize;
extern uint32_t ar_snmp_tblsize;
extern uint32_t ar_wakeuptblsize;

extern int fm_port_enter_autores_for_deepsleep(struct fm_port *port,
				struct auto_res_port_params *params);
extern int fm_port_config_autores_for_deepsleep_support(struct fm_port *port,
				struct auto_res_tables_sizes *params);
extern int fm_port_get_autores_stats(struct fm_port *port,
				struct auto_res_port_stats *stats);

/*Information sent to the FMD*/
/*Tables maintained at AR CNTRL which will be sent to FMD*/
/*ARP Table*/
extern struct auto_res_arp_info ar_arp_db;
/*NDP Table*/
extern struct auto_res_ndp_info ar_nd_db;
/*ICMP ECHO Table*/
extern struct auto_res_echo_ipv4_info ar_ipv4_echo_db;
extern struct auto_res_echo_ipv6_info ar_ipv6_echo_db;
/*SNMP Table*/
extern auto_res_snmp_db ar_snmp_db;
/*Wake UP rules Table*/
extern auto_res_filtering_db ar_wakeup_db;

/*FMAN Stats*/
struct auto_res_port_stats ar_stats;

/*Object to contain networking configuration*/
struct auto_res_port_params ar_port_param;

#ifdef AR_DEBUG_API
void ar_interface_info_dump()
{
	int index, j;
	printk("=========================================================\n");
	printk("Index\tIPv4\t\tMAC\t\tisvlan\t\tvlanID\n");
	printk("=========================================================\n");
	for (index = 0; index < ar_arp_db.table_size; index++) {
		printk("%d\t%0x\t", index, ar_arp_db.auto_res_table[index].ip_address);
		printk("%pM\t", ar_arp_db.auto_res_table[index].mac);
		printk("%u\t%u\n", ar_arp_db.auto_res_table[index].is_vlan,
				ar_arp_db.auto_res_table[index].vid);
	}
	printk("arp_conflict_detection = %d\n", ar_arp_db.enable_conflict_detection);
	printk("=========================================================\n");
	printk("Index\tIPv6\t\tMAC\t\tisvlan\t\tvlanID\n");
	printk("=========================================================\n");
	for (index = 0; index < ar_nd_db.table_size_assigned; index++) {
		printk("%d\t%0x:%0x:%0x:%0x\t", index, ar_nd_db.auto_res_table_assigned[index].ip_address[0]
				, ar_nd_db.auto_res_table_assigned[index].ip_address[1]
				, ar_nd_db.auto_res_table_assigned[index].ip_address[2]
				, ar_nd_db.auto_res_table_assigned[index].ip_address[3]);

		printk("%pM\t", ar_nd_db.auto_res_table_assigned[index].mac);
		printk("%u\t%u\n", ar_nd_db.auto_res_table_assigned[index].is_vlan,
				ar_nd_db.auto_res_table_assigned[index].vid);
	}
	printk("ndp_conflict_detection = %d\n", ar_nd_db.enable_conflict_detection);
	printk("=========================================================\n");
	printk("index\toidSize\t\toidVal\t\tresSize\t\tresVal\n");
	printk("=========================================================\n");
	for (index = 0; index < ar_snmp_db.oid_table_size; index++) {
		printk("%d\t%d\t", index, ar_snmp_db.oid_table[index].oidSize);
		for (j = 0; j < ar_snmp_db.oid_table[index].oidSize; j++)
			printk("%x", ar_snmp_db.oid_table[index].oidVal[j]);
		printk("\t%d\t", ar_snmp_db.oid_table[index].resSize);
		for (j = 0; j < ar_snmp_db.oid_table[index].resSize; j++)
			printk("%hhx", ar_snmp_db.oid_table[index].resVal[j]);
		printk("\n");
	}
	printk("getall_flag = %d\tcomunity_public_string = %s\
					comunity_privat_string = %s\n",
					ar_snmp_db.control,
					ar_snmp_db.community_read_write_string,
					ar_snmp_db.community_read_only_string);
	printk("===========================================================\n");
	printk("IP Protocols\n");
	printk("index\tProtocol\n");
	printk("===========================================================\n");
	for (index = 0; index < ar_wakeup_db.ip_prot_table_size; index++)
		printk("%d\t%x\n", index, ar_wakeup_db.ip_prot_table_ptr[index]);

	printk("===========================================================\n");
	printk("TCP Rules\n");
	printk("index\tsrcport\t\tdstport\t\tsrcportmask\t\tdstportmask\t\ttcpflagmask\n");
	printk("===========================================================\n");
	for (index = 0; index < ar_wakeup_db.tcp_ports_table_size; index++) {
		printk("%d\t%hd\t\t%hd\t\t%hx\t\t%hx\t%hx\n", index,
				ar_wakeup_db.tcp_ports_table_ptr[index].src_port,
				ar_wakeup_db.tcp_ports_table_ptr[index].dst_port,
				ar_wakeup_db.tcp_ports_table_ptr[index].src_port_mask,
				ar_wakeup_db.tcp_ports_table_ptr[index].dst_port_mask,
				ar_wakeup_db.tcp_flags_mask);
	}
	printk("===========================================================\n");
	printk("UDP Rules\n");
	printk("index\tsrcport\t\tdstport\t\tsrcportmask\t\tdstportmask\n");
	printk("===========================================================\n");
	for (index = 0; index < ar_wakeup_db.udp_ports_table_size; index++) {
		printk("%d\t%hd\t\t%hd\t\t%hx\t\t%hx\n", index,
				ar_wakeup_db.udp_ports_table_ptr[index].src_port,
				ar_wakeup_db.udp_ports_table_ptr[index].dst_port,
				ar_wakeup_db.udp_ports_table_ptr[index].src_port_mask,
				ar_wakeup_db.udp_ports_table_ptr[index].dst_port_mask);
	}
	printk("ip_prot_drop_on_hit = %d\tudp_port_drop_on_hit = %d\
					tcp_port_drop_on_hit = %d\n",
					ar_wakeup_db.ip_prot_drop_on_hit,
					ar_wakeup_db.udp_port_drop_on_hit,
					ar_wakeup_db.tcp_port_drop_on_hit);

	return;
}

void ar_get_fman_stats()
{
	struct fm_port *fm_rxport = NULL;
	struct fm_port *fm_txport = NULL;

	ar_get_fm_port(&fm_rxport, &fm_txport);
	fm_port_get_autores_stats(fm_rxport, &ar_stats);

	printk("Following is the statistics:\n");
	printk("============================\n");
	printk("ARP Reqquest Cnt = %d\n", ar_stats.arp_ar_cnt);
	printk("NDP Reqquest Cnt = %d\n", ar_stats.ndp_ar_cnt);
	printk("ICMPv4 Reqquest Cnt = %d\n", ar_stats.echo_icmpv4_ar_cnt);
	printk("ICMPv6 Reqquest Cnt = %d\n", ar_stats.echo_icmpv6_ar_cnt);
	return;
}
#endif

void ar_process_deep_sleep_event (uint32_t  *out_status)
{
	struct net_device *netdev = NULL;
	struct fm_port *fm_rxport = NULL;
	struct fm_port *fm_txport = NULL;
	t_Handle h_FmPortTx;
	t_Error retcode;

	/*Get FM ports*/
	ar_get_fm_port(&fm_rxport, &fm_txport);
	if (!fm_rxport) {
		PRINT_INFO("Auto Response cannot be started as interface is down\n");
		*out_status = AR_GET_INFO_ERROR;
		return;
	}
	if (!fm_txport) {
		PRINT_INFO("Auto Response cannot be started as interface is down\n");
		*out_status = AR_GET_INFO_ERROR;
		return;
	}

	/*Get network device information from Linux stack*/
	netdev = ar_get_device();
	retcode = ar_get_interface_info(netdev);
	if (retcode != AR_SUCCESS) {
		PRINT_INFO("Error in getting interface configuration\n");
		*out_status = AR_GET_INFO_ERROR;
		return;
	}

	h_FmPortTx = (t_Handle)fm_port_get_handle(fm_txport);

	/*Storing network configuration*/
	ar_ipv4_echo_db.table_size		= ar_arp_db.table_size;
	ar_ipv6_echo_db.table_size		= ar_nd_db.table_size_assigned;
	ar_port_param.h_FmPortTx  		= h_FmPortTx;
	ar_port_param.p_auto_res_arp_info  	= &ar_arp_db;
	ar_port_param.p_auto_res_ndp_info  	= &ar_nd_db;
	ar_port_param.p_auto_res_echo_ipv4_info = &ar_ipv4_echo_db;
	ar_port_param.p_auto_res_echo_ipv6_info = &ar_ipv6_echo_db;
	ar_port_param.p_auto_res_snmp_info 	= &ar_snmp_db;
	ar_port_param.p_auto_res_filtering_info	= &ar_wakeup_db;

	/*Before entering deep sleep, configure MAC for wakeup sources*/
	device_set_wakeup_enable(netdev->dev.parent, ar_wakeup_src);

	/*Send network device information to the FMD for
	  configuring FM ucode. Wait for the ack*/
	retcode = fm_port_enter_autores_for_deepsleep(fm_rxport, &ar_port_param);

	*out_status = AR_SUCCESS;
	return;
}
EXPORT_SYMBOL(ar_process_deep_sleep_event);

void ar_process_resume_event(int32_t  *out_status)
{
	struct fm_port *fm_rxport = NULL;
	struct fm_port *fm_txport = NULL;

	/*Get FM ports*/
	ar_get_fm_port(&fm_rxport, &fm_txport);
	if ((!fm_rxport) || (!fm_txport)) {
		PRINT_INFO("Error in fetching FMAN RX/TX port\n");
		*out_status = -AR_GET_INFO_ERROR;
		return;
	}
	fm_port_exit_auto_res_for_deep_sleep(fm_rxport, fm_txport);
	*out_status = AR_SUCCESS;
	return;
}
