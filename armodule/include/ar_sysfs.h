/**************************************************************************
 * Copyright 2013, Freescale Semiconductor, Inc. All rights reserved.
 ***************************************************************************/
/*
 * File:        ar_sysfs.h
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
#include <linux/init.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/types.h>
#include <linux/stat.h>
#include <linux/fcntl.h>
#include <linux/fs.h>

uint32_t ar_create_sysfs(void);

int ar_sysfs_init(void);

int ar_sysfs_exit(void);

ssize_t ar_snmp_flag_store(struct kobject *kobj,
				struct kobj_attribute *attr,
				const char *buf, size_t count);

ssize_t ar_snmp_pvt_string_store(struct kobject *kobj,
				struct kobj_attribute *attr,
				const char *buf, size_t count);

ssize_t ar_snmp_pub_string_store(struct kobject *kobj,
				struct kobj_attribute *attr,
				const char *buf, size_t count);

ssize_t ar_arp_conflict_flag_store(struct kobject *kobj,
				struct kobj_attribute *attr,
				const char *buf, size_t count);

ssize_t ar_ndp_conflict_flag_store(struct kobject *kobj,
				struct kobj_attribute *attr,
				const char *buf, size_t count);

ssize_t ar_ip_pass_flag_store(struct kobject *kobj,
				struct kobj_attribute *attr,
				const char *buf, size_t count);

ssize_t ar_udp_pass_flag_store(struct kobject *kobj,
				struct kobj_attribute *attr,
				const char *buf, size_t count);

ssize_t ar_tcp_pass_flag_store(struct kobject *kobj,
				struct kobj_attribute *attr,
				const char *buf, size_t count);

ssize_t ar_tcp_flag_mask_store(struct kobject *kobj,
				struct kobj_attribute *attr,
				const char *buf, size_t count);

ssize_t ar_interface_store(struct kobject *kobj,
				struct kobj_attribute *attr,
				const char *buf, size_t count);

ssize_t ar_fman_stats_show(struct kobject *kobj,
				struct kobj_attribute *attr,
				const char *buf, size_t count);
