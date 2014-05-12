/**************************************************************************
 * Copyright 2013, Freescale Semiconductor, Inc. All rights reserved.
 **************************************************************************/
/*
 * File:	arctrl_core.c
 *
 * Description: Main module for Auto Response Core initialization.
 *
 * Authors:	 Sunil Kumar Kori <B42948@freescale.com>
 *
 */
/**************************************************************************/
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/unistd.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/in.h>
#include <linux/inetdevice.h>
#include <linux/sysctl.h>
#include <net/addrconf.h>
#include <8021q/vlan.h>
#include <uapi/linux/if_addr.h>
#include <mac.h>
#include <dpaa_eth.h>
#include <ar_common.h>
#include <ar_sysfs.h>
#include <ar_pm.h>

MODULE_AUTHOR("Freescale Semiconductor, Inc");
MODULE_DESCRIPTION("Auto Response");
MODULE_LICENSE("GPL");

/*Auto Response port and protocol table size*/
char *ar_resport = "fm1-mac5";
uint32_t ar_wakeup_src = TRUE;
struct auto_res_tables_sizes *p_ar_maxsize;
uint32_t ar_arptblsize;
uint32_t ar_ndptblsize;

module_param(ar_resport, charp, 0644);
MODULE_PARM_DESC(ar_resport, "Configuring Auto Response port");

/*Tables maintained at AR CNTRL which will be sent to FMD*/
/*ARP Table*/
struct auto_res_arp_entry *ar_arp_tbl;
struct auto_res_arp_info ar_arp_db;

/*NDP Table*/
struct auto_res_ndp_entry *ar_ndp_tem_tbl;
struct auto_res_ndp_entry *ar_ndp_tgt_tbl;
struct auto_res_ndp_info ar_nd_db;

/*ICMP ECHO Table*/
struct auto_res_echo_ipv4_info ar_ipv4_echo_db;
struct auto_res_echo_ipv6_info ar_ipv6_echo_db;

/*Extern definitions*/
#ifdef AR_DEBUG_API
extern int  ar_proc_init(void);
extern void ar_proc_exit(void);
#endif

/*brief Index used by Linux to register driver */
#define AR_SNMP_DEV_MAJ_NUM (103)
#define AR_WAKEUP_DEV_MAJ_NUM (104)

/*brief Name used when mounting path */
#define AR_WAKEUP_DEVICE_NAME "ar_wakeup_dev"
#define AR_SNMP_DEVICE_NAME "ar_dev"

/*Extern definition*/
extern const struct file_operations ar_snmp_interface_fops;
extern const struct file_operations ar_wakeup_interface_fops;

int32_t ar_intf_init_db()
{
	struct fm_port *fm_rxport = NULL;
	struct fm_port *fm_txport = NULL;

	ar_get_fm_port(&fm_rxport, &fm_txport);
	if (!fm_rxport && !fm_txport)
		return -AR_INIT_ERROR;

	/*Get Maximum table sizes from FMAN*/
	p_ar_maxsize = fm_port_get_autores_maxsize(fm_rxport);
	if (!p_ar_maxsize) {
		PRINT_INFO("Failure to get information from FMAN\n");
		return -AR_INIT_ERROR;
	}
	ar_arptblsize = p_ar_maxsize->max_num_of_arp_entries;
	ar_ndptblsize = p_ar_maxsize->max_num_of_ndp_entries;

	/*Allocated space to each table*/
	/*ARP*/
	ar_arp_tbl = (struct auto_res_arp_entry *)kzalloc((sizeof(struct auto_res_arp_entry)
								*ar_arptblsize), GFP_KERNEL);
	if (!ar_arp_tbl) {
		PRINT_INFO("Failure to allocate memory");
		return -AR_MEMORY_UNAVALABLE_ERROR;
	}
	/*NDP*/
	ar_ndp_tem_tbl = (struct auto_res_ndp_entry *)kzalloc((sizeof(struct auto_res_ndp_entry)
								*ar_ndptblsize), GFP_KERNEL);
	if (!ar_ndp_tem_tbl) {
		PRINT_INFO("Failure to allocate memory");
		kzfree((void *)ar_arp_tbl);
		return -AR_MEMORY_UNAVALABLE_ERROR;
	}
	ar_ndp_tgt_tbl = (struct auto_res_ndp_entry *)kzalloc((sizeof(struct auto_res_ndp_entry)
								*ar_ndptblsize), GFP_KERNEL);
	if (!ar_ndp_tgt_tbl) {
		PRINT_INFO("Failure to allocate memory");
		kzfree((void *)ar_arp_tbl);
		kzfree((void *)ar_ndp_tem_tbl);
		return -AR_MEMORY_UNAVALABLE_ERROR;
	}

	memset(&ar_arp_db, 0, sizeof(struct auto_res_arp_info));
	ar_arp_db.auto_res_table = ar_arp_tbl;

	memset(&ar_nd_db, 0, sizeof(struct auto_res_ndp_info));
	ar_nd_db.auto_res_table_tmp 		= ar_ndp_tem_tbl;
	ar_nd_db.auto_res_table_assigned 	= ar_ndp_tgt_tbl;


	memset(&ar_ipv4_echo_db, 0, sizeof(struct auto_res_echo_ipv4_info));
	memset(&ar_ipv6_echo_db, 0, sizeof(struct auto_res_echo_ipv4_info));
	ar_ipv4_echo_db.auto_res_table = ar_arp_tbl;
	ar_ipv6_echo_db.auto_res_table = ar_ndp_tgt_tbl;

	return AR_SUCCESS;
}
EXPORT_SYMBOL(ar_intf_init_db);

struct net_device *ar_get_device(void)
{
	return dev_get_by_name(&init_net, ar_resport);
}
EXPORT_SYMBOL(ar_get_device);

void ar_get_fm_port(struct fm_port **rxport,
		struct fm_port **txport)
{
	struct net_device *netdev;
	struct dpa_priv_s *priv;
	struct mac_device *mac_dev;

	netdev = dev_get_by_name(&init_net, ar_resport);
	if (!netdev) {
		PRINT_INFO("Device not found\n");
		return;
	}
	if (netdev->priv_flags & IFF_802_1Q_VLAN) {
		netdev = vlan_dev_priv(netdev)->real_dev;
	}
	if (netdev->flags & IFF_UP) {
		priv = (struct dpa_priv_s *)netdev_priv(netdev);
		mac_dev = priv->mac_dev;
		*rxport = mac_dev->port_dev[RX];
		*txport = mac_dev->port_dev[TX];
	} else
		PRINT_INFO("Auto response port is down\n");
	return;
}
EXPORT_SYMBOL(ar_get_fm_port);

uint16_t ar_get_vlanid(struct net_device *dev)
{
	return vlan_dev_priv(dev)->vlan_id;
}


void ar_get_ipv4_info(struct net_device *netdev)
{
	struct in_ifaddr *ifa = NULL;
	struct in_device *indev = NULL;
	struct net_device *dev = NULL;
	uint32_t index;
	bool maxlim_flag = 0;

	/*Getting information for Physical interfaces*/
	indev = __in_dev_get_rcu(netdev);
	for (index = 0, ifa = indev->ifa_list; ifa; index++, ifa = ifa->ifa_next) {
		memcpy(ar_arp_tbl[index].mac, netdev->dev_addr, 6);
		ar_arp_tbl[index].ip_address = ifa->ifa_address;
		ar_arp_tbl[index].is_vlan    = FALSE;
		ar_arp_tbl[index].vid        = 0;
		ar_arp_db.table_size++;
		if (ar_arp_db.table_size >= ar_arptblsize) {
			maxlim_flag = 1;
			break;
		}
	}
	if (!maxlim_flag) {
		/*Getting information for VLAN interfaces*/
		dev = first_net_device(&init_net);
		while (dev) {
			if ((dev->priv_flags & IFF_802_1Q_VLAN) &&
					netdev == vlan_dev_priv(dev)->real_dev) {

				/*Copying MAC address*/
				memcpy(ar_arp_tbl[index].mac, dev->dev_addr, 6);

				/*Copying IPv4 address*/
				indev = __in_dev_get_rcu(dev);
				ifa = indev->ifa_list;
				ar_arp_tbl[index].ip_address = ifa->ifa_address;

				/*Copying VLAN information*/
				ar_arp_tbl[index].is_vlan    = TRUE;
				ar_arp_tbl[index].vid        = ar_get_vlanid(dev);

				ar_arp_db.table_size++;
				if (ar_arp_db.table_size >= ar_arptblsize)
					break;
				/*Move to next index*/
				index++;
			}
			dev = next_net_device(dev);
		}
	}
	return;
}

void ar_get_ipv6_info(struct net_device *netdev)
{
	struct inet6_dev *idev = NULL;
	struct list_head *p = NULL;
	struct inet6_ifaddr *ifa = NULL;
	struct net_device *dev = NULL;
	uint32_t index_tgt = 0, index_tem = 0;
	bool maxlim_flag_tgt = 0, maxlim_flag_tem = 0;

	/*Getting information for physical interface*/
	idev = __in6_dev_get(netdev);
	list_for_each(p, &idev->addr_list) {
		ifa = list_entry(p, struct inet6_ifaddr, if_list);
		if (ifa->flags & IFA_F_TEMPORARY) {
			memcpy(ar_ndp_tem_tbl[index_tem].mac, netdev->dev_addr, 6);
			ar_ndp_tem_tbl[index_tem].ip_address[0] = ifa->addr.s6_addr32[0];
			ar_ndp_tem_tbl[index_tem].ip_address[1] = ifa->addr.s6_addr32[1];
			ar_ndp_tem_tbl[index_tem].ip_address[2] = ifa->addr.s6_addr32[2];
			ar_ndp_tem_tbl[index_tem].ip_address[3] = ifa->addr.s6_addr32[3];
			ar_ndp_tem_tbl[index_tem].is_vlan       = FALSE;
			ar_ndp_tem_tbl[index_tem].vid           = 0;
			ar_nd_db.table_size_tmp++;
			if (ar_nd_db.table_size_tmp >= ar_ndptblsize) {
				maxlim_flag_tem = 1;
				break;
			}
			index_tem++;
		} else if (ifa->flags & IFA_F_PERMANENT) {
			memcpy(ar_ndp_tgt_tbl[index_tgt].mac, netdev->dev_addr, 6);
			ar_ndp_tgt_tbl[index_tgt].ip_address[0] = ifa->addr.s6_addr32[0];
			ar_ndp_tgt_tbl[index_tgt].ip_address[1] = ifa->addr.s6_addr32[1];
			ar_ndp_tgt_tbl[index_tgt].ip_address[2] = ifa->addr.s6_addr32[2];
			ar_ndp_tgt_tbl[index_tgt].ip_address[3] = ifa->addr.s6_addr32[3];
			ar_ndp_tgt_tbl[index_tgt].is_vlan	    = FALSE;
			ar_ndp_tgt_tbl[index_tgt].vid 	    = 0;
			ar_nd_db.table_size_assigned++;
			if (ar_nd_db.table_size_assigned >= ar_ndptblsize) {
				maxlim_flag_tgt = 1;
				break;
			}
			index_tgt++;
		}
	}
	/*Getting information for VLAN interfaces*/
	dev = first_net_device(&init_net);
	while (dev) {
		if ((dev->priv_flags & IFF_802_1Q_VLAN) &&
			netdev == vlan_dev_priv(dev)->real_dev) {

			idev = __in6_dev_get(dev);
			p = (&idev->addr_list)->next;
			ifa = list_entry(p, struct inet6_ifaddr, if_list);
			if ((ifa->flags & IFA_F_TEMPORARY) && maxlim_flag_tem) {
				/*Copying IPv6 address*/
				ar_ndp_tem_tbl[index_tem].ip_address[0] = ifa->addr.s6_addr32[0];
				ar_ndp_tem_tbl[index_tem].ip_address[1] = ifa->addr.s6_addr32[1];
				ar_ndp_tem_tbl[index_tem].ip_address[2] = ifa->addr.s6_addr32[2];
				ar_ndp_tem_tbl[index_tem].ip_address[3] = ifa->addr.s6_addr32[3];

				/*Copying MAC address*/
				memcpy(ar_ndp_tem_tbl[index_tem].mac, dev->dev_addr, 6);

				/*Copying VLAN information*/
				ar_ndp_tem_tbl[index_tem].is_vlan 	    = TRUE;
				ar_ndp_tem_tbl[index_tem].vid           = ar_get_vlanid(dev);

				ar_nd_db.table_size_tmp++;
				if (ar_nd_db.table_size_tmp >= ar_ndptblsize)
					break;
				/*Move to next index*/
				index_tem++;
			} else if ((ifa->flags & IFA_F_PERMANENT) && maxlim_flag_tgt) {
				/*Copying IPv6 address*/
				ar_ndp_tgt_tbl[index_tgt].ip_address[0] = ifa->addr.s6_addr32[0];
				ar_ndp_tgt_tbl[index_tgt].ip_address[1] = ifa->addr.s6_addr32[1];
				ar_ndp_tgt_tbl[index_tgt].ip_address[2] = ifa->addr.s6_addr32[2];
				ar_ndp_tgt_tbl[index_tgt].ip_address[3] = ifa->addr.s6_addr32[3];

				/*Copying MAC address*/
				memcpy(ar_ndp_tgt_tbl[index_tgt].mac, dev->dev_addr, 6);

				/*Copying VLAN information*/
				ar_ndp_tgt_tbl[index_tgt].is_vlan       = TRUE;
				ar_ndp_tgt_tbl[index_tgt].vid           = ar_get_vlanid(dev);

				ar_nd_db.table_size_assigned++;
				if (ar_nd_db.table_size_assigned >= ar_ndptblsize)
					break;
				/*Move to next index*/
				index_tgt++;
			}
		}
		dev = next_net_device(dev);
	}
	return;
}

int  ar_get_interface_info(struct net_device *netdev)
{
	ar_arp_db.table_size = 0;
	ar_nd_db.table_size_assigned = 0;
	ar_nd_db.table_size_tmp = 0;

	if (!netdev) {
		PRINT_INFO("No Network device present\n");
		return -AR_GET_INFO_ERROR;
	}
	ar_get_ipv4_info(netdev);
	ar_get_ipv6_info(netdev);
#ifdef AR_DEBUG
	printk("Interface information recieved successfully of device = %s\n",\
			netdev->name);
#endif
	return AR_SUCCESS;
}

static int __init ar_init(void)
{
	/*Registering the character device */
	if (register_chrdev(AR_SNMP_DEV_MAJ_NUM,
				AR_SNMP_DEVICE_NAME,
				&ar_snmp_interface_fops) < 0) {
		printk("%s : %s : Unable to get the major number %d\n", __FILE__,
				__FUNCTION__, AR_SNMP_DEV_MAJ_NUM);
		return -1;
	}

	if (register_chrdev(AR_WAKEUP_DEV_MAJ_NUM,
				AR_WAKEUP_DEVICE_NAME,
				&ar_wakeup_interface_fops) < 0) {
			printk("%s : %s : Unable to get the major number %d\n", __FILE__,
							__FUNCTION__, AR_WAKEUP_DEV_MAJ_NUM);
			return -1;
	}


#ifdef AR_DEBUG_API
	/*Init Proc interface*/
	ar_proc_init();
#endif
	/*Create sysfs entries*/
	if (ar_create_sysfs() < 0) {
		PRINT_INFO("Error in creating Sysfs interface\n");
		return -AR_INIT_ERROR;
	}

	/*Init ARP/ND database*/
	if (ar_intf_init_db() < 0) {
		PRINT_INFO("Error in initialization\n");

		/*Unregister SNMP char device*/
		unregister_chrdev(AR_SNMP_DEV_MAJ_NUM, AR_SNMP_DEVICE_NAME);

		/*Unregister wake-up char device*/
		unregister_chrdev(AR_WAKEUP_DEV_MAJ_NUM, AR_WAKEUP_DEVICE_NAME);

#ifdef AR_DEBUG_API
		/*Unregister proc interface*/
		ar_proc_exit();
#endif
		/*Unregister sysfs interface*/
		ar_sysfs_exit();

		return -AR_INIT_ERROR;
	}

	/*Init SNMP database*/
	ar_snmp_init_db();

	/*Init Wake rules database*/
	ar_wakeup_init_db();

	/*Register with Linux PM module*/
	if (ar_pm_register() < 0) {
		PRINT_INFO("Error in registering with Linux PM module");

		/*Unregister SNMP char device*/
		unregister_chrdev(AR_SNMP_DEV_MAJ_NUM, AR_SNMP_DEVICE_NAME);

		/*Unregister wake-up char device*/
		unregister_chrdev(AR_WAKEUP_DEV_MAJ_NUM, AR_WAKEUP_DEVICE_NAME);

#ifdef AR_DEBUG_API
		/*Unregister proc interface*/
		ar_proc_exit();
#endif
		/*Unregister sysfs interface*/
		ar_sysfs_exit();

		return -AR_INIT_ERROR;
	}
	return AR_SUCCESS;
}

static void  __exit ar_exit(void)
{
	/*Unregister SNMP char device*/
	unregister_chrdev(AR_SNMP_DEV_MAJ_NUM, AR_SNMP_DEVICE_NAME);

	/*Unregister wake-up char device*/
	unregister_chrdev(AR_WAKEUP_DEV_MAJ_NUM, AR_WAKEUP_DEVICE_NAME);

	/*Unregister sysfs interface*/
	ar_sysfs_exit();

#ifdef AR_DEBUG_API
	/*Unregister proc interface*/
	ar_proc_exit();
#endif
	/*Unregister with Linux PM module*/
	ar_pm_unregister();

	return;

}
module_init(ar_init);
module_exit(ar_exit);
