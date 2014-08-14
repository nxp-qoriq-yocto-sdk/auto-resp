/**************************************************************************
 * Copyright 2013, Freescale Semiconductor, Inc. All rights reserved.
 ***************************************************************************/
/*
 * File:        arctrl_sysfs.c
 *
 * Description: Module to provide sysfs interface.
 *
 * Authors:      Sunil Kumar Kori <B42948@freescale.com>
 *
 */
/***************************************************************************/
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/unistd.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/types.h>
#include <linux/stat.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include "ar_common.h"
#include "ar_sysfs.h"

extern struct auto_res_arp_info ar_arp_db;
extern struct auto_res_ndp_info ar_nd_db;
extern auto_res_snmp_db ar_snmp_db;
extern auto_res_filtering_db ar_wakeup_db;
extern char ar_resport[10];

/*SNMP sysfs entries*/
struct kobject *ar_kobj;

static struct kobj_attribute get_all_flag_attr = \
				__ATTR(get_all_flag, 0644,
				NULL, ar_snmp_flag_store);

static struct kobj_attribute com_pvt_string_attr = \
				__ATTR(com_pvt_string, 0644,
				NULL, ar_snmp_pvt_string_store);

static struct kobj_attribute com_pub_string_attr = \
				__ATTR(com_pub_string, 0644,
				NULL, ar_snmp_pub_string_store);

static struct kobj_attribute arp_conf_det_attr = \
				__ATTR(arp_conflict_detect, 0644,
				NULL, ar_arp_conflict_flag_store);

static struct kobj_attribute ndp_conf_det_attr = \
				__ATTR(ndp_conflict_detect, 0644,
				NULL, ar_ndp_conflict_flag_store);

static struct kobj_attribute ip_prot_pass_on_hit_attr = \
				__ATTR(ip_prot_pass_on_hit, 0644,
				NULL, ar_ip_pass_flag_store);

static struct kobj_attribute udp_port_pass_on_hit_attr = \
				__ATTR(udp_port_pass_on_hit, 0644,
				NULL, ar_udp_pass_flag_store);

static struct kobj_attribute tcp_port_pass_on_hit_attr = \
				__ATTR(tcp_port_pass_on_hit, 0644,
				NULL, ar_tcp_pass_flag_store);

static struct kobj_attribute tcp_flag_mask_attr = \
				__ATTR(tcp_flag_mask, 0644,
				NULL, ar_tcp_flag_mask_store);
#ifdef AR_DEBUG_API
static struct kobj_attribute fman_stats_attr = \
				__ATTR(fman_stats, 0644,
				NULL, ar_fman_stats_show);
#endif

static struct attribute *ar_sysfs_attrs[] = {
	&get_all_flag_attr.attr,
	&com_pvt_string_attr.attr,
	&com_pub_string_attr.attr,
	&arp_conf_det_attr.attr,
	&ndp_conf_det_attr.attr,
	&ip_prot_pass_on_hit_attr.attr,
	&udp_port_pass_on_hit_attr.attr,
	&tcp_port_pass_on_hit_attr.attr,
	&tcp_flag_mask_attr.attr,
#ifdef AR_DEBUG_API
	&fman_stats_attr.attr,
#endif
	NULL
};

static struct attribute_group ar_snmp_attr_group = {
	.attrs = ar_sysfs_attrs,
};

ssize_t ar_snmp_flag_store(struct kobject *kobj,
		struct kobj_attribute *attr,
		const char *buf, size_t count)
{
	char *pArg = (char *)kzalloc(strlen(buf) + 1, GFP_KERNEL);
	sscanf(buf, "%s", pArg);
	ar_snmp_db.control = *pArg-'0';
#ifdef AR_DEBUG
	printk("%d\n", ar_snmp_db.control);
#endif
	kfree(pArg);
	return count;
}

ssize_t ar_snmp_pvt_string_store(struct kobject *kobj,
		struct kobj_attribute *attr,
		const char *buf, size_t count)
{
	uint8_t len = strlen(buf) - 1;

	if (len >= (AR_SNMP_MAX_COMM_LENGTH - 3)) {
		PRINT_INFO("Error: String too big\n");
		return count;
	}
	ar_snmp_db.community_read_write_string[0] = 0x04;  /*String type*/
	ar_snmp_db.community_read_write_string[1] = len; /*String len*/
	memcpy(&(ar_snmp_db.community_read_write_string[2]), buf, len);
	ar_snmp_db.community_read_write_string[len + 2] = '\0';
#ifdef AR_DEBUG
	printk("%s\n", &(ar_snmp_db.community_read_write_string[2]));
#endif
	return count;
}

ssize_t ar_snmp_pub_string_store(struct kobject *kobj,
		struct kobj_attribute *attr,
		const char *buf, size_t count)
{
	uint8_t len = strlen(buf) - 1;

	if (len >= (AR_SNMP_MAX_COMM_LENGTH - 3)) {
		PRINT_INFO("Error: String too big\n");
		return count;
	}
	ar_snmp_db.community_read_only_string[0] = 0x04;   /*String type*/
	ar_snmp_db.community_read_only_string[1] = len; /*String len*/
	memcpy(&(ar_snmp_db.community_read_only_string[2]), buf, len);
	ar_snmp_db.community_read_only_string[len + 2] = '\0';
#ifdef AR_DEBUG
	printk("%s\n", &(ar_snmp_db.community_read_only_string[2]));
#endif
	return count;
}

ssize_t ar_arp_conflict_flag_store(struct kobject *kobj,
		struct kobj_attribute *attr,
		const char *buf, size_t count)
{
	char *pArg = (char *)kzalloc(strlen(buf) + 1, GFP_KERNEL);
	sscanf(buf, "%s", pArg);
	ar_arp_db.enable_conflict_detection = *pArg-'0';
#ifdef AR_DEBUG
	printk("arp_conflict_detection = %d\n", ar_arp_db.enable_conflict_detection);
#endif
	kfree(pArg);
	return count;
}

ssize_t ar_ndp_conflict_flag_store(struct kobject *kobj,
		struct kobj_attribute *attr,
		const char *buf, size_t count)
{
	char *pArg = (char *)kzalloc(strlen(buf) + 1, GFP_KERNEL);
	sscanf(buf, "%s", pArg);
	ar_nd_db.enable_conflict_detection = *pArg-'0';
#ifdef AR_DEBUG
	printk("ndp_conflict_detection = %d\n", ar_nd_db.enable_conflict_detection);
#endif
	kfree(pArg);
	return count;
}

ssize_t ar_ip_pass_flag_store(struct kobject *kobj,
		struct kobj_attribute *attr,
		const char *buf, size_t count)
{
	char *pArg = (char *)kzalloc(strlen(buf) + 1, GFP_KERNEL);
	sscanf(buf, "%s", pArg);
	ar_wakeup_db.ip_prot_pass_on_hit = *pArg-'0';
#ifdef AR_DEBUG
	printk("%d\n", ar_wakeup_db.ip_prot_pass_on_hit);
#endif
	kfree(pArg);
	return count;
}

ssize_t ar_udp_pass_flag_store(struct kobject *kobj,
		struct kobj_attribute *attr,
		const char *buf, size_t count)
{
	char *pArg = (char *)kzalloc(strlen(buf) + 1, GFP_KERNEL);
	sscanf(buf, "%s", pArg);
	ar_wakeup_db.udp_port_pass_on_hit = *pArg-'0';
#ifdef AR_DEBUG
	printk("%d\n", ar_wakeup_db.udp_port_pass_on_hit);
#endif
	kfree(pArg);
	return count;
}

ssize_t ar_tcp_pass_flag_store(struct kobject *kobj,
		struct kobj_attribute *attr,
		const char *buf, size_t count)
{
	char *pArg = (char *)kzalloc(strlen(buf) + 1, GFP_KERNEL);
	sscanf(buf, "%s", pArg);
	ar_wakeup_db.tcp_port_pass_on_hit = *pArg-'0';
#ifdef AR_DEBUG
	printk("%d\n", ar_wakeup_db.tcp_port_pass_on_hit);
#endif
	kfree(pArg);
	return count;
}

ssize_t ar_tcp_flag_mask_store(struct kobject *kobj,
		struct kobj_attribute *attr,
		const char *buf, size_t count)
{
	char *pArg = (char *)kzalloc(strlen(buf) + 1, GFP_KERNEL);
	sscanf(buf, "%s", pArg);
	pArg[strlen(buf)] = '\0';
	ar_wakeup_db.tcp_flags_mask = ar_wakeup_atoi(pArg);
#ifdef AR_DEBUG
	printk("%hx\n", ar_wakeup_db.tcp_flags_mask);
#endif
	kfree(pArg);
	return count;
}

#ifdef AR_DEBUG_API
ssize_t ar_fman_stats_show(struct kobject *kobj,
				struct kobj_attribute *attr,
				const char *buf, size_t count)
{
	ar_get_fman_stats();
	return count;
}
#endif
uint16_t ar_wakeup_atoi(char *pArg)
{
	uint16_t result = 0, i = 0;

	while (pArg[i] != '\0') {
		if (pArg[i] >= '0' && pArg[i] <= '9')
			pArg[i] = pArg[i] - '0';
		else if (pArg[i] >= 'A' && pArg[i] <= 'F')
			pArg[i] = pArg[i] - 'A' + 10;
		else if (pArg[i] >= 'a' && pArg[i] <= 'f')
			pArg[i] = pArg[i] - 'a' + 10;
		i++;
	}
	i = 0;
	result = (pArg[i] << 4) | pArg[i+1];
	result = result << 8;
	i = i + 2;
	result = result | (((pArg[i] << 4) | (pArg[i+1])));
	return result;
}

int ar_sysfs_init(void)
{
	uint32_t error;

	ar_kobj = kobject_create_and_add("arctrl", NULL);
	if (!ar_kobj)
		return -ENOMEM;

	error = sysfs_create_group(ar_kobj, &ar_snmp_attr_group);
	if (error)
		goto attr_exit;
	return 0;

attr_exit:
	kobject_put(ar_kobj);
	return error;
}

int ar_sysfs_exit(void)
{
	if (!ar_kobj)
		return 0;

	sysfs_remove_group(ar_kobj, &ar_snmp_attr_group);
	kobject_put(ar_kobj);
	return 0;
}

uint32_t ar_create_sysfs()
{
	return ar_sysfs_init();
}
