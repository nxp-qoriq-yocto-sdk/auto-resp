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

static struct dev_pm_ops ar_pm_ops = {
	.suspend_noirq = ar_suspend_noirq,
	.resume_noirq = ar_resume_noirq
};

#define AR_PM_OPS (&ar_pm_ops)

static const struct of_device_id ar_match[] = {
	{
		.compatible	= "fsl,auto-res"
	},
	{}
};
MODULE_DEVICE_TABLE(of, ar_match);


/* Structure for a device driver */
static struct platform_driver ar_driver = {
	.driver = {
		.name = "fsl-autores",
		.of_match_table = ar_match,
		.owner = THIS_MODULE,
		.pm = AR_PM_OPS,
	},
	.probe = NULL,
	.remove = NULL,
};

int ar_suspend_noirq(struct device *dev)
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

int ar_resume_noirq(struct device *dev)
{
#ifdef AR_DEBUG
	printk("%s invoked\n", __FUNCTION__);
#endif
	return 0;
}

int ar_pm_register(void)
{
	int retVal;
	retVal = platform_driver_register(&ar_driver);
	if (retVal < 0)
		printk("Error in registering to PM module\n");
	return retVal;
}

void ar_pm_unregister(void)
{
	platform_driver_unregister(&ar_driver);
	return;
}
