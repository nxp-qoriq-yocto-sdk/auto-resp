/**************************************************************************
 * Copyright 2013, Freescale Semiconductor, Inc. All rights reserved.
 **************************************************************************/
/*
 * File:	arctrl_pm.h
 *
 * Description: Registers auto response control module with Linux Power
 *		Management module to get trigger for deep sleep event.
 *
 * Authors:	 Sunil Kumar Kori <B42948@freescale.com>
 *
 */
/**************************************************************************/
#include <linux/notifier.h>

/*API to get trigger from Linux PM module through notifier chain calls*/
int ar_handle_pm_event(struct notifier_block *nb,
			   unsigned long event, void *ptr);
/*API to suspend the autoresponse module*/
int ar_suspend_noirq(void);

/*API to resume the autoresponse module*/
int ar_resume_noirq(void);

/*API to register with Linux PM module*/
int ar_pm_register(void);

/*API to unregister with Linux PM module*/
void ar_pm_unregister(void);
