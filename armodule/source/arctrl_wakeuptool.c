/**************************************************************************
 * Copyright 2013, Freescale Semiconductor, Inc. All rights reserved.
 ***************************************************************************/
/*
 * File:        arctrl_wakeuptool.c
 *
 * Description: Module to provide interface between wakeup toolkit
 *		and auto response module .
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
spinlock_t		ar_wakeapp_lock;
static int 		ar_wakeopen_count;

/* Interfaces provided by this driver*/
const struct file_operations ar_wakeup_interface_fops = {
	.open 		=	ar_wakeup_interface_open,
	.release 	=	ar_wakeup_interface_release,
	.unlocked_ioctl	=	ar_wakeup_interface_ioctl,
};

int ar_wakeup_interface_open(struct inode *inode, struct file *filp)
{
	int minor = MINOR(inode->i_rdev);

	/*  Allow only one process to open
	    and use the single device at a time. */
	if (minor != 0)
		return -ENODEV; /*it must be minor number 0 */

	spin_lock_bh(&ar_wakeapp_lock);
	if (ar_wakeopen_count) {
		spin_unlock_bh(&ar_wakeapp_lock);
		return -EUSERS;
	}
	ar_wakeopen_count++;
	spin_unlock_bh(&ar_wakeapp_lock);
	return 0;
}

int ar_wakeup_interface_release(struct inode *inode, struct file *filp)
{
	spin_lock_bh(&ar_wakeapp_lock);
	ar_wakeopen_count--;
	spin_unlock_bh(&ar_wakeapp_lock);
	return 0;
}

long ar_wakeup_interface_ioctl(struct file *filp,
		unsigned int cmd, unsigned long arg)
{
	long ret = 0;
	auto_res_port_filtering_e pInfo;

	switch (cmd) {
	case CONFIG_WAKEUP_IP_PROTO:
		{
			char *pUsrArg = (char *)arg;
			char *p_ip_prot = (char *)kzalloc(10, GFP_KERNEL);;
			if (!p_ip_prot) {
				PRINT_INFO("Memory allocation failed\n");
				return -AR_MEMORY_UNAVALABLE_ERROR;
			}
			if (copy_from_user(p_ip_prot, pUsrArg, strlen(pUsrArg))) {
				kzfree(p_ip_prot);
				return -EFAULT;
			}
			p_ip_prot[strlen(pUsrArg)] = '\0';
			ret = ar_wakeup_recv_config(cmd, p_ip_prot);
			if (ret != AR_SUCCESS) {
				PRINT_INFO("Error in sending information\n");
				kzfree(p_ip_prot);
				return ret;
			}
			kzfree(p_ip_prot);
			p_ip_prot = NULL;
			break;
		}
	case CONFIG_WAKEUP_UDP_RULE:
		{
			int resSize = sizeof(auto_res_port_filtering_e);
			auto_res_port_filtering_e *pUsrArg = (auto_res_port_filtering_e *)arg;

			if (copy_from_user(&pInfo, pUsrArg, resSize))
				return -EFAULT;
			ret = ar_wakeup_recv_config(cmd, &pInfo);
			if (ret != AR_SUCCESS) {
				PRINT_INFO("Error in sending information\n");
				return ret;
			}
			break;
		}
	case CONFIG_WAKEUP_TCP_RULE:
		{
			int resSize = sizeof(auto_res_port_filtering_e);
			auto_res_port_filtering_e *pUsrArg = (auto_res_port_filtering_e *)arg;

			if (copy_from_user((void *)&pInfo, (void *)pUsrArg, resSize))
				return -EFAULT;
			ret = ar_wakeup_recv_config(cmd, &pInfo);
			if (ret != AR_SUCCESS) {
				PRINT_INFO("Error in sending information\n");
				return ret;
			}
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
