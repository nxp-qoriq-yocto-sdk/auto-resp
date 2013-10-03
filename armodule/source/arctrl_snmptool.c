/**************************************************************************
 * Copyright 2013, Freescale Semiconductor, Inc. All rights reserved.
 ***************************************************************************/
/*
 * File:        arctrl_snmptool.c
 *
 * Description: Module to provide interface between snmptool kit and auto response module .
 *
 * Authors:      Sunil Kumar Kori <B42948@freescale.com>
 *
 */
/***************************************************************************/
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/sysctl.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include "ar_common.h"

/*Variable to read data from device atomicaly*/
spinlock_t		ar_app_lock;
static int 		ar_open_count;


/* Interfaces provided by this driver*/
const struct file_operations ar_snmp_interface_fops = {
	.open 		=	ar_interface_open,
	.release 	=	ar_interface_release,
	.unlocked_ioctl	=	ar_interface_ioctl,
};

int ar_interface_open(struct inode *inode, struct file *filp)
{
	int minor = MINOR(inode->i_rdev);

	/*  Allow only one process to open
	    and use the single device at a time. */
	if (minor != 0)
		return -ENODEV; /*it must be minor number 0 */

	spin_lock_bh(&ar_app_lock);
	if (ar_open_count) {
		spin_unlock_bh(&ar_app_lock);
		return -EUSERS;
	}
	ar_open_count++;
	spin_unlock_bh(&ar_app_lock);

#ifdef AR_DEBUG
	PRINT_INFO("AR device  Interface driver opened\n");
#endif
	return 0;
}

int ar_interface_release(struct inode *inode, struct file *filp)
{
#ifdef AR_DEBUG
	PRINT_INFO("AR device Interface driver released\n");
#endif
	spin_lock_bh(&ar_app_lock);
	ar_open_count--;
	spin_unlock_bh(&ar_app_lock);

	return 0;
}

long ar_interface_ioctl(struct file *filp,
		unsigned int cmd, unsigned long arg)
{
	long ret = 0;
	auto_res_snmp_e pInfo;
	uint8_t *pConfigStr;

	switch (cmd) {
	case CREATE_SNMP_ENTRY:
		{
			auto_res_snmp_e *pUsrArg;
			int resSize = sizeof(auto_res_snmp_e);
			pUsrArg = (auto_res_snmp_e *)arg;
			if (copy_from_user(&pInfo, pUsrArg, resSize))
				return -EFAULT;

			pInfo.oidVal = (uint8_t *)kzalloc(128, GFP_KERNEL);
			if (!pInfo.oidVal) {
				PRINT_INFO("Memory allocation failed\n");
				return -AR_MEMORY_UNAVALABLE_ERROR;
			}
			if (copy_from_user(pInfo.oidVal, pUsrArg->oidVal, 128))
				return -EFAULT;

			pInfo.resVal = (uint8_t *)kzalloc(132, GFP_KERNEL);
			if (!pInfo.resVal) {
				PRINT_INFO("Memory allocation failed\n");
				return AR_MEMORY_UNAVALABLE_ERROR;
			}
			if (copy_from_user(pInfo.resVal, pUsrArg->resVal, 132))
				return -EFAULT;
			ret =  ar_snmp_recv_agent_config(cmd, (auto_res_snmp_e *)&pInfo);
			break;
		}
	case CONFIG_SNMP_PUB_COMM:
	case CONFIG_SNMP_PRIV_COMM:
		{
			uint8_t *pUsrArg;
			pUsrArg = (uint8_t *)arg;
			pConfigStr = (uint8_t *)kzalloc(16, GFP_KERNEL);
			if (!pConfigStr) {
				PRINT_INFO("Memory allocation failed\n");
				return -AR_MEMORY_UNAVALABLE_ERROR;
			}
			if (copy_from_user(pConfigStr, pUsrArg, 16))
				return -EFAULT;
			ar_snmp_config_comm_str(cmd, pConfigStr);

			break;
		}
	case CONFIG_SNMP_GETALL_FLAG:
		{
			uint8_t *pUsrArg;
			pUsrArg = (uint8_t *)arg;
			pConfigStr = (uint8_t *)kzalloc(1, GFP_KERNEL);
			if (!pConfigStr) {
				PRINT_INFO("Memory allocation failed\n");
				return -AR_MEMORY_UNAVALABLE_ERROR;
			}
			if (copy_from_user(pConfigStr, pUsrArg, 1))
				return -EFAULT;
			ar_snmp_config_getall_flag_str(pConfigStr);
			kzfree(pConfigStr);
			break;
		}
	default:
		{
#ifdef AR_DEBUG
			printk(KERN_INFO "Invalid AR IOCTL cmd\n");
#endif
			ret = -EBADRQC;
			break;
		}
	}
	return ret;
}
