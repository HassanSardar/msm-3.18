#include <linux/device.h>
<<<<<<< HEAD
=======
#include <linux/module.h>
>>>>>>> origin/3.18.14.x

#include <linux/notifier.h>
#include <linux/ccic/ccic_notifier.h>
#include <linux/sec_sysfs.h>
#include <linux/ccic/ccic_sysfs.h>
#ifdef CONFIG_USB_TYPEC_MANAGER_NOTIFIER
#include <linux/battery/battery_notifier.h>
#endif
#include <linux/usb_notify.h>

#define DEBUG
#define SET_CCIC_NOTIFIER_BLOCK(nb, fn, dev) do {	\
		(nb)->notifier_call = (fn);		\
		(nb)->priority = (dev);			\
	} while (0)

#define DESTROY_CCIC_NOTIFIER_BLOCK(nb)			\
		SET_CCIC_NOTIFIER_BLOCK(nb, NULL, -1)

static struct ccic_notifier_struct ccic_notifier;

struct device *ccic_device;
<<<<<<< HEAD
static int ccic_notifier_init_done;
int ccic_notifier_init(void);

char CCIC_NOTI_DEST_Print[8][10] = {
	{"INITIAL"},
	{"USB"},
	{"BATTERY"},
	{"PDIC"},
	{"MUIC"},
	{"CCIC"},
	{"MANAGER"},
	{"ALL"},
};

char CCIC_NOTI_ID_Print[7][20] = {
	{"ID_INITIAL"},
	{"ID_ATTACH"},
	{"ID_RID"},
	{"ID_USB"},
	{"ID_POWER_STATUS"},
	{"ID_WATER"},
	{"ID_VCONN"},
};

char CCIC_NOTI_RID_Print[8][15] = {
	{"RID_UNDEFINED"},
	{"RID_000K"},
	{"RID_001K"},
	{"RID_255K"},
	{"RID_301K"},
	{"RID_523K"},
	{"RID_619K"},
	{"RID_OPEN"},
};

char CCIC_NOTI_USB_STATUS_Print[5][20] = {
	{"USB_DETACH"},
	{"USB_ATTACH_DFP"},
	{"USB_ATTACH_UFP"},
	{"USB_ATTACH_DRP"},
	{"USB_ATTACH_NO_USB"},
=======
static int ccic_notifier_init_done = 0;
int ccic_notifier_init(void);

char CCIC_NOTI_DEST_Print[CCIC_NOTI_DEST_NUM][10] =
{
    {"INITIAL"},
    {"USB"},
    {"BATTERY"},
    {"PDIC"},
    {"MUIC"},
    {"CCIC"},
    {"MANAGER"},
    {"DP"},
    {"DPUSB"},
    {"ALL"},
};

char CCIC_NOTI_ID_Print[CCIC_NOTI_ID_NUM][20] =
{
    {"ID_INITIAL"},
    {"ID_ATTACH"},
    {"ID_RID"},
    {"ID_USB"},
    {"ID_POWER_STATUS"},
    {"ID_WATER"},
    {"ID_VCONN"},
    {"ID_DP_CONNECT"},
    {"ID_DP_HPD"},
    {"ID_DP_LINK_CONF"},
    {"ID_DP_USB"},
    {"ID_ROLE_SWAP"},
};

char CCIC_NOTI_RID_Print[CCIC_NOTI_RID_NUM][15] =
{
    {"RID_UNDEFINED"},
    {"RID_000K"},
    {"RID_001K"},
    {"RID_255K"},
    {"RID_301K"},
    {"RID_523K"},
    {"RID_619K"},
    {"RID_OPEN"},
};

char CCIC_NOTI_USB_STATUS_Print[CCIC_NOTI_USB_STATUS_NUM][20] =
{
    {"USB_DETACH"},
    {"USB_ATTACH_DFP"},
    {"USB_ATTACH_UFP"},
    {"USB_ATTACH_DRP"},
    {"USB_ATTACH_NO_USB"},
>>>>>>> origin/3.18.14.x
};

int ccic_notifier_register(struct notifier_block *nb, notifier_fn_t notifier,
			ccic_notifier_device_t listener)
{
	int ret = 0;

	pr_info("%s: listener=%d register\n", __func__, listener);
<<<<<<< HEAD
	if (!ccic_notifier_init_done)
		ccic_notifier_init();

	/* Check if CCIC Notifier is ready. */
=======

	/* Check if CCIC Notifier is ready. */
	if(!ccic_notifier_init_done)
		ccic_notifier_init();
	
>>>>>>> origin/3.18.14.x
	if (!ccic_device) {
		pr_err("%s: Not Initialized...\n", __func__);
		return -1;
	}

	SET_CCIC_NOTIFIER_BLOCK(nb, notifier, listener);
	ret = blocking_notifier_chain_register(&(ccic_notifier.notifier_call_chain), nb);
	if (ret < 0)
		pr_err("%s: blocking_notifier_chain_register error(%d)\n",
				__func__, ret);

	/* current ccic's attached_device status notify */
	nb->notifier_call(nb, 0,
			&(ccic_notifier.ccic_template));

	return ret;
}

int ccic_notifier_unregister(struct notifier_block *nb)
{
	int ret = 0;

	pr_info("%s: listener=%d unregister\n", __func__, nb->priority);

	ret = blocking_notifier_chain_unregister(&(ccic_notifier.notifier_call_chain), nb);
	if (ret < 0)
		pr_err("%s: blocking_notifier_chain_unregister error(%d)\n",
				__func__, ret);
	DESTROY_CCIC_NOTIFIER_BLOCK(nb);

	return ret;
}

void ccic_uevent_work(int id, int state)
{
	char *water[2] = { "CCIC=WATER", NULL };
	char *dry[2] = { "CCIC=DRY", NULL };
	char *vconn[2] = { "CCIC=VCONN", NULL };
<<<<<<< HEAD

	pr_info("usb: %s: id=%s state=%d\n", __func__, CCIC_NOTI_ID_Print[id], state);
	if (id == CCIC_NOTIFY_ID_WATER) {
		if (state)
			kobject_uevent_env(&ccic_device->kobj, KOBJ_CHANGE, water);
		else
			kobject_uevent_env(&ccic_device->kobj, KOBJ_CHANGE, dry);
	} else if (id == CCIC_NOTIFY_ID_VCONN) {
		kobject_uevent_env(&ccic_device->kobj, KOBJ_CHANGE, vconn);
=======
#if defined(CONFIG_SEC_FACTORY)
	char ccicrid[15] = {0,};
	char *rid[2] = {ccicrid, NULL};
#endif

	pr_info("usb: %s: id=%s state=%d\n", __func__, CCIC_NOTI_ID_Print[id], state);

	switch (id) {
		case CCIC_NOTIFY_ID_WATER:
			if (state)
				kobject_uevent_env(&ccic_device->kobj, KOBJ_CHANGE, water);
			else
				kobject_uevent_env(&ccic_device->kobj, KOBJ_CHANGE, dry);
			break;
		case CCIC_NOTIFY_ID_VCONN:
			kobject_uevent_env(&ccic_device->kobj, KOBJ_CHANGE, vconn);
			break;
#if defined(CONFIG_SEC_FACTORY)
		case CCIC_NOTIFY_ID_RID:
			snprintf(ccicrid, sizeof(ccicrid), "%s",
				(state<CCIC_NOTI_RID_NUM)? CCIC_NOTI_RID_Print[state] : CCIC_NOTI_RID_Print[0]);
			kobject_uevent_env(&ccic_device->kobj, KOBJ_CHANGE, rid);			
			break;
#endif
		default:
			break;
>>>>>>> origin/3.18.14.x
	}
}

/* ccic's attached_device attach broadcast */
int ccic_notifier_notify(CC_NOTI_TYPEDEF *p_noti, void *pd, int pdic_attach)
{
	int ret = 0;
	ccic_notifier.ccic_template = *p_noti;

	switch (p_noti->id) {
#ifdef CONFIG_USB_TYPEC_MANAGER_NOTIFIER
<<<<<<< HEAD
	case CCIC_NOTIFY_ID_POWER_STATUS:		/* PDIC_NOTIFY_EVENT_PD_SINK */
=======
	case CCIC_NOTIFY_ID_POWER_STATUS:		// PDIC_NOTIFY_EVENT_PD_SINK
>>>>>>> origin/3.18.14.x
		pr_info("%s: src:%01x dest:%01x id:%02x "
			"attach:%02x cable_type:%02x rprd:%01x\n", __func__,
			((CC_NOTI_ATTACH_TYPEDEF *)p_noti)->src,
			((CC_NOTI_ATTACH_TYPEDEF *)p_noti)->dest,
			((CC_NOTI_ATTACH_TYPEDEF *)p_noti)->id,
			((CC_NOTI_ATTACH_TYPEDEF *)p_noti)->attach,
			((CC_NOTI_ATTACH_TYPEDEF *)p_noti)->cable_type,
			((CC_NOTI_ATTACH_TYPEDEF *)p_noti)->rprd);

		if (pd != NULL) {
<<<<<<< HEAD
			if (((CC_NOTI_ATTACH_TYPEDEF *)p_noti)->attach) {
				((struct pdic_notifier_struct *)pd)->event = PDIC_NOTIFY_EVENT_PD_SINK;
			} else {
				if (((struct pdic_notifier_struct *)pd)->event != PDIC_NOTIFY_EVENT_CCIC_ATTACH)
					((struct pdic_notifier_struct *)pd)->event = PDIC_NOTIFY_EVENT_DETACH;
=======
			if (!((CC_NOTI_ATTACH_TYPEDEF *)p_noti)->attach) {
				((struct pdic_notifier_struct *)pd)->event = PDIC_NOTIFY_EVENT_DETACH;
>>>>>>> origin/3.18.14.x
			}
			ccic_notifier.ccic_template.pd = pd;

			pr_info("%s: PD event:%d, num:%d, sel:%d \n", __func__,
				((struct pdic_notifier_struct *)pd)->event,
				((struct pdic_notifier_struct *)pd)->sink_status.available_pdo_num,
				((struct pdic_notifier_struct *)pd)->sink_status.selected_pdo_num);
		}
		break;
#endif
	case CCIC_NOTIFY_ID_ATTACH:
		pr_info("%s: src:%01x dest:%01x id:%02x "
			"attach:%02x cable_type:%02x rprd:%01x\n", __func__,
			((CC_NOTI_ATTACH_TYPEDEF *)p_noti)->src,
			((CC_NOTI_ATTACH_TYPEDEF *)p_noti)->dest,
			((CC_NOTI_ATTACH_TYPEDEF *)p_noti)->id,
			((CC_NOTI_ATTACH_TYPEDEF *)p_noti)->attach,
			((CC_NOTI_ATTACH_TYPEDEF *)p_noti)->cable_type,
			((CC_NOTI_ATTACH_TYPEDEF *)p_noti)->rprd);
		break;
	case CCIC_NOTIFY_ID_RID:
		pr_info("%s: src:%01x dest:%01x id:%02x rid:%02x\n", __func__,
			((CC_NOTI_RID_TYPEDEF *)p_noti)->src,
			((CC_NOTI_RID_TYPEDEF *)p_noti)->dest,
			((CC_NOTI_RID_TYPEDEF *)p_noti)->id,
			((CC_NOTI_RID_TYPEDEF *)p_noti)->rid);
<<<<<<< HEAD
=======
#if defined(CONFIG_SEC_FACTORY)
			ccic_uevent_work(CCIC_NOTIFY_ID_RID,((CC_NOTI_RID_TYPEDEF *)p_noti)->rid);
#endif
>>>>>>> origin/3.18.14.x
		break;
	case CCIC_NOTIFY_ID_WATER:
		pr_info("%s: src:%01x dest:%01x id:%02x attach:%02x\n", __func__,
			((CC_NOTI_ATTACH_TYPEDEF *)p_noti)->src,
			((CC_NOTI_ATTACH_TYPEDEF *)p_noti)->dest,
			((CC_NOTI_ATTACH_TYPEDEF *)p_noti)->id,
			((CC_NOTI_ATTACH_TYPEDEF *)p_noti)->attach);
			ccic_uevent_work(CCIC_NOTIFY_ID_WATER, ((CC_NOTI_ATTACH_TYPEDEF *)p_noti)->attach);
		break;
	case CCIC_NOTIFY_ID_VCONN:
		ccic_uevent_work(CCIC_NOTIFY_ID_VCONN, 0);
		break;
<<<<<<< HEAD
=======
	case CCIC_NOTIFY_ID_ROLE_SWAP:
		pr_info("%s: src:%01x dest:%01x id:%02x sub1:%02x\n", __func__,
			((CC_NOTI_ATTACH_TYPEDEF *)p_noti)->src,
			((CC_NOTI_ATTACH_TYPEDEF *)p_noti)->dest,
			((CC_NOTI_ATTACH_TYPEDEF *)p_noti)->id,
			((CC_NOTI_ATTACH_TYPEDEF *)p_noti)->attach);
>>>>>>> origin/3.18.14.x
	default:
		pr_info("%s: src:%01x dest:%01x id:%02x "
			"sub1:%d sub2:%02x sub3:%02x\n", __func__,
			((CC_NOTI_TYPEDEF *)p_noti)->src,
			((CC_NOTI_TYPEDEF *)p_noti)->dest,
			((CC_NOTI_TYPEDEF *)p_noti)->id,
			((CC_NOTI_TYPEDEF *)p_noti)->sub1,
			((CC_NOTI_TYPEDEF *)p_noti)->sub2,
			((CC_NOTI_TYPEDEF *)p_noti)->sub3);
		break;
	}
<<<<<<< HEAD
#if  defined(CONFIG_USB_NOTIFY_PROC_LOG)
	store_usblog_notify(NOTIFY_CCIC_EVENT, (void *)p_noti , NULL);
=======
#ifdef CONFIG_USB_NOTIFY_PROC_LOG
	store_usblog_notify(NOTIFY_CCIC_EVENT, (void*)p_noti , NULL);
>>>>>>> origin/3.18.14.x
#endif
	ret = blocking_notifier_call_chain(&(ccic_notifier.notifier_call_chain),
			p_noti->id, &(ccic_notifier.ccic_template));


	switch (ret) {
	case NOTIFY_STOP_MASK:
	case NOTIFY_BAD:
		pr_err("%s: notify error occur(0x%x)\n", __func__, ret);
		break;
	case NOTIFY_DONE:
	case NOTIFY_OK:
		pr_info("%s: notify done(0x%x)\n", __func__, ret);
		break;
	default:
		pr_info("%s: notify status unknown(0x%x)\n", __func__, ret);
		break;
	}

	return ret;
<<<<<<< HEAD
}

#if 0
void ccic_notifier_255K_test(void)
{
	ccic_notifier_rid_t rid_test;

	pr_err("%s:\n", __func__);
	rid_test = RID_301K;

	/* ccic's attached_device attach broadcast */
	ccic_notifier_notify();
}
#endif
=======

}
>>>>>>> origin/3.18.14.x

int ccic_notifier_init(void)
{
	int ret = 0;

	pr_info("%s\n", __func__);
<<<<<<< HEAD
	if (ccic_notifier_init_done) {
		pr_err("%s already registered\n", __func__);
		goto out;
	}

=======
	if(ccic_notifier_init_done)
	{
		pr_err("%s already registered\n", __func__);
		goto out;	
	}
	ccic_notifier_init_done = 1;
>>>>>>> origin/3.18.14.x
	ccic_device = sec_device_create(NULL, "ccic");
	if (IS_ERR(ccic_device)) {
		pr_err("%s Failed to create device(switch)!\n", __func__);
		ret = -ENODEV;
		goto out;
	}

	/* create sysfs group */
	ret = sysfs_create_group(&ccic_device->kobj, &ccic_sysfs_group);
	if (ret) {
		pr_err("%s: ccic sysfs fail, ret %d", __func__, ret);
		goto out;
	}


	BLOCKING_INIT_NOTIFIER_HEAD(&(ccic_notifier.notifier_call_chain));
<<<<<<< HEAD
	ccic_notifier_init_done = 1;
=======

>>>>>>> origin/3.18.14.x
out:
	return ret;
}

static void __exit ccic_notifier_exit(void)
{
<<<<<<< HEAD
	pr_info("usb: [M] %s exit\n", __func__);
=======
	pr_info("%s: exit\n", __func__);
>>>>>>> origin/3.18.14.x
}

device_initcall(ccic_notifier_init);
module_exit(ccic_notifier_exit);
