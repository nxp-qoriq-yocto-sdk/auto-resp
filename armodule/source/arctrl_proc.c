/**************************************************************************
 * Copyright 2013, Freescale Semiconductor, Inc. All rights reserved.
 ***************************************************************************/
/*
 * File:        arctrl_proc.c
 *
 * Description: Module to provide proc interface.
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
#include <internal.h>
#include <linux/types.h>
#include <linux/stat.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include "ar_common.h"

#ifdef AR_DEBUG_API
/*Macros for Deep sleep events*/
#define AR_DEEP_SLEEP_MODE deep_sleep_mode

/*Systable definitions*/
struct proc_dir_entry *ar_dir;
EXPORT_SYMBOL(ar_dir);

uint32_t *ar_data;
EXPORT_SYMBOL(ar_data);

const struct file_operations proc_fops = {
	.write = ar_handle_deep_sleep_event
};

ssize_t ar_handle_deep_sleep_event(struct file *file, const char __user *buffer,
		size_t count, loff_t *data)
{
	uint32_t out_status;

	/*Process deep sleep event*/
	count = 2; /*Just a random value*/
	ar_process_deep_sleep_event(&out_status);
	if (out_status != AR_SUCCESS) {
		printk("Error in processing the event with status = %d\n",
				out_status);
	}
	return count;
}

int ar_proc_init(void)
{
	/*Creating Proc interface to simulate
	  deep sleep event*/
	struct proc_dir_entry   *proc_file;

	/*Register other under /proc/autoresponse */
	ar_dir =  proc_mkdir("autoresponse", NULL);
	if (!ar_dir) {
		PRINT_INFO("Error in creating proc directory\n");
		return -ENOMEM;
	}

	/*Allocate space to recieve data*/
	ar_data = kzalloc(sizeof(*ar_data), GFP_KERNEL);
	if (!ar_data) {
		PRINT_INFO("Error in allocating memory\n");
		remove_proc_entry("autoresponse", NULL);
		return -ENOMEM;
	}

	proc_file = proc_create("autoresponse/deep_sleep_mode", 0644,
							NULL, &proc_fops);
	if (!proc_file) {
		PRINT_INFO("Cannot create proc entry\n");
		remove_proc_entry("autoresponse", NULL);
		kzfree((const void *)ar_data);
		return -EFAULT;
	}
	proc_file->data = ar_data;

	return 0;
}
void  ar_proc_exit(void)
{
	/*Unregister proc interface*/
	remove_proc_entry("deep_sleep_mode", ar_dir);
	remove_proc_entry("autoresponse", NULL);
	kzfree((const void *)ar_data);
	ar_data = NULL;
	return;

}
#endif
