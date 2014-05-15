/**************************************************************************
 * Copyright 2013, Freescale Semiconductor, Inc. All rights reserved.
 **************************************************************************/
/*
 * File:	arctrl_pm.c
 *
 * Description: Registers auto response control module with Linux Power
 *		Management module to get trigger for deep sleep event.
 *
 * Authors:	 Sunil Kumar Kori <B42948@freescale.com>
 *
 */
/**************************************************************************/

#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <ar_common.h>
#include <ar_pm.h>
#include <linux/notifier.h>
#include <linux/suspend.h>

static struct notifier_block ar_notifier_block = {
	.notifier_call = ar_handle_pm_event,
};

int ar_handle_pm_event(struct notifier_block *nb,
			   unsigned long event, void *unused)
{
	switch (event) {
	case PM_SUSPEND_PREPARE:
		ar_suspend_noirq();
		break;
	case PM_POST_SUSPEND:
		ar_resume_noirq();
		break;
	default:
		break;
	}
	return 0;
}

int ar_suspend_noirq(void)
{
	uint32_t out_status;

	/*Process deep sleep evenet*/
	ar_process_deep_sleep_event(&out_status);
	if (out_status != AR_SUCCESS) {
		printk("Error in processing the event with status = %d\n",
				out_status);
		return -out_status;
	}
	return 0;
}

int ar_resume_noirq(void)
{
	int32_t out_status;
#ifdef AR_DEBUG
	printk("%s invoked\n", __FUNCTION__);
#endif
	ar_process_resume_event(&out_status);
	if (out_status < 0) {
		printk("Error in resumeing: Error Code = %d\n",
							out_status);
		return out_status;
	}
	return 0;
}

int ar_pm_register(void)
{
	int retval;
	retval = register_pm_notifier(&ar_notifier_block);
	if (retval < 0)
		printk("Error in registering to PM module\n");
	return retval;
}

void ar_pm_unregister(void)
{
	unregister_pm_notifier(&ar_notifier_block);
	return;
}
