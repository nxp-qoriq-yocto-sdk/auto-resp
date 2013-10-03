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

/*API to get trigger from Linux PM module*/
int ar_suspend_noirq(struct device *dev);
int ar_resume_noirq(struct device *dev);
/*API to register with Linux PM module*/
int ar_pm_register(void);
void ar_pm_unregister(void);
