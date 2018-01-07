
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/firmware.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
<<<<<<< HEAD
#include <linux/hrtimer.h>
=======
>>>>>>> origin/3.18.14.x
#include <linux/interrupt.h>
#include <linux/gpio.h>

#include "fts_ts.h"

#define WRITE_CHUNK_SIZE 64
<<<<<<< HEAD
#define FTS_DEFAULT_UMS_FW "/sdcard/stm.fw"
#define FTS_DEFAULT_FFU_FW "ffu_tsp.bin"
#define FTS64FILE_SIGNATURE 0xaaaa5555
=======
#define FTS_DEFAULT_UMS_FW "/sdcard/Firmware/TSP/stm.fw"
#define FTS_DEFAULT_FFU_FW "ffu_tsp.bin"
#define FTSFILE_SIGNATURE 0xAAAA5555
>>>>>>> origin/3.18.14.x

enum {
	BUILT_IN = 0,
	UMS,
	NONE,
	FFU,
};

<<<<<<< HEAD
struct fts64_header {
=======
struct fts_header {
>>>>>>> origin/3.18.14.x
	unsigned int signature;
	unsigned short fw_ver;
	unsigned char fw_id;
	unsigned char reserved1;
	unsigned char internal_ver[8];
	unsigned char released_ver[8];
	unsigned int reserved2;
	unsigned int checksum;
};

<<<<<<< HEAD
static int fts_fw_wait_for_flash_ready(struct fts_ts_info *info)
{
	unsigned char regAdd;
	unsigned char buf[3];
	int retry = 0;

	regAdd = FTS_CMD_READ_FLASH_STAT;

	while (info->fts_read_reg
		(info, &regAdd, 1, (unsigned char *)buf, 1)) {
		if ((buf[0] & 0x01) == 0)
			break;

		if (retry++ > FTS_RETRY_COUNT * 10) {
			tsp_debug_err(true, info->dev,
				 "%s: Time Over\n",
				 __func__);
			return -FTS_ERROR_TIMEOUT;
		}
		msleep(20);
	}
=======
#define	FW_IMAGE_SIZE_D3	(256 * 1024)
#define	SIGNEDKEY_SIZE		(256)

int FTS_Check_DMA_Done(struct fts_ts_info *info)
{
	int timeout = 60;
	unsigned char regAdd[2] = { 0xF9, 0x05};
	unsigned char val[1];

	do {
		info->fts_read_reg(info, &regAdd[0], 2, (unsigned char*)val, 1);

		if ((val[0] & 0x80) != 0x80)
			break;

		fts_delay(50);
		timeout--;
	} while (timeout != 0);

	if (timeout == 0)
		return -1;
>>>>>>> origin/3.18.14.x

	return 0;
}

<<<<<<< HEAD
#define	FW_IMAGE_SIZE_D1	64 * 1024
#define	FW_IMAGE_SIZE_D2	128 * 1024
#define	SIGNEDKEY_SIZE	256

static int fts_fw_burn(struct fts_ts_info *info, unsigned char *fw_data)
{
	unsigned char regAdd[WRITE_CHUNK_SIZE + 3];
	int section;
	int fsize = FW_IMAGE_SIZE_D1;

	/* Check busy Flash */
	if (fts_fw_wait_for_flash_ready(info)<0)
		return -1;

	/* FTS_CMD_UNLOCK_FLASH */
	tsp_debug_info(true, info->dev, "%s: Unlock Flash\n", __func__);
	regAdd[0] = FTS_CMD_UNLOCK_FLASH;
	regAdd[1] = 0x74;
	regAdd[2] = 0x45;
	info->fts_write_reg(info, &regAdd[0], 3);
	msleep(500);

	/* Copy to PRAM */
	if (info->digital_rev == FTS_DIGITAL_REV_2)
		fsize = FW_IMAGE_SIZE_D2 + sizeof(struct fts64_header);

	tsp_debug_info(true, info->dev, "%s: Copy to PRAM [Size : %d]\n", __func__, fsize);

	for (section = 0; section < (fsize / WRITE_CHUNK_SIZE); section++) {
		regAdd[0] = FTS_CMD_WRITE_PRAM + (((section * WRITE_CHUNK_SIZE) >> 16) & 0x0f);
		regAdd[1] = ((section * WRITE_CHUNK_SIZE) >> 8) & 0xff;
		regAdd[2] = (section * WRITE_CHUNK_SIZE) & 0xff;
		memcpy(&regAdd[3],
			&fw_data[section * WRITE_CHUNK_SIZE +
				sizeof(struct fts64_header)],
			WRITE_CHUNK_SIZE);

		info->fts_write_reg(info, &regAdd[0], WRITE_CHUNK_SIZE + 3);
	}

	msleep(100);

	/* Erase Program Flash */
	tsp_debug_info(true, info->dev, "%s: Erase Program Flash\n", __func__);
	info->fts_command(info, FTS_CMD_ERASE_PROG_FLASH);
	msleep(100);

	/* Check busy Flash */
	if (fts_fw_wait_for_flash_ready(info)<0)
		return -1;

	/* Burn Program Flash */
	tsp_debug_info(true, info->dev, "%s: Burn Program Flash\n", __func__);
	info->fts_command(info, FTS_CMD_BURN_PROG_FLASH);
	msleep(100);

	/* Check busy Flash */
	if (fts_fw_wait_for_flash_ready(info)<0)
		return -1;

	/* Reset FTS */
	info->fts_systemreset(info);
	return 0;
}

static int fts_get_system_status(struct fts_ts_info *info, unsigned char *val1, unsigned char *val2)
{
	bool rc = -1;
	unsigned char regAdd1[4] = { 0xb2, 0x07, 0xfb, 0x04 };
	unsigned char regAdd2[4] = { 0xb2, 0x17, 0xfb, 0x04 };
	unsigned char data[FTS_EVENT_SIZE];
	int retry = 0;

	if (info->digital_rev == FTS_DIGITAL_REV_2)
		regAdd2[1] = 0x1f;

	info->fts_write_reg(info, &regAdd1[0], 4);
	info->fts_write_reg(info, &regAdd2[0], 4);

	memset(data, 0x0, FTS_EVENT_SIZE);
	regAdd1[0] = READ_ONE_EVENT;

	while (info->fts_read_reg(info, &regAdd1[0], 1, (unsigned char *)data,
				   FTS_EVENT_SIZE)) {
		if ((data[0] == 0x12) && (data[1] == regAdd1[1])
			&& (data[2] == regAdd1[2])) {
			rc = 0;
			*val1 = data[3];
			tsp_debug_info(true, info->dev,
				"%s: System Status 1 : 0x%02x\n",
			__func__, data[3]);
		}
		else if ((data[0] == 0x12) && (data[1] == regAdd2[1])
			&& (data[2] == regAdd2[2])) {
			rc = 0;
			*val2 = data[3];
			tsp_debug_info(true, info->dev,
				"%s: System Status 2 : 0x%02x\n",
			__func__, data[3]);
			break;
		}

		if (retry++ > FTS_RETRY_COUNT) {
			rc = -1;
			tsp_debug_err(true, info->dev,
				"%s: Time Over\n", __func__);
			break;
		}
	}
	return rc;
}

int fts_fw_wait_for_event(struct fts_ts_info *info, unsigned char eid)
=======
static int FTS_Check_Erase_Done(struct fts_ts_info *info)
{
	int timeout = 60;  // 3 sec timeout
	unsigned char regAdd[2] = {0xF9, 0x02};
	unsigned char val[1];

	do {
		info->fts_read_reg(info, &regAdd[0], 2, (unsigned char*)val, 1);

		if ((val[0] & 0x80) != 0x80)
			break;

		fts_delay(50);
		timeout--;
	} while (timeout != 0);

	if (timeout == 0)
		return -1;

	return 0;
}

static int fts_fw_burn_d3(struct fts_ts_info *info, unsigned char *fw_data)
{
	int rc;
	const unsigned long int FTS_CODE_SIZE = (244 * 1024);	// Total 244kB for Code
	const unsigned long int FTS_CONFIG_SIZE = (4 * 1024);	// Total 4kB for Config
	const unsigned long int DRAM_LEN = (64 * 1024);	// 64KB
	const unsigned int CODE_ADDR_START = (0x0000);
	const unsigned int CONFIG_ADDR_START = (0xFC00);
	const unsigned int WRITE_CHUNK_SIZE_D3 = 32;

	unsigned char *config_data = NULL;

	unsigned long int size = 0;
	unsigned long int i;
	unsigned long int j;
	unsigned long int k;
	unsigned long int dataLen;
	unsigned long int len = 0;
	unsigned long int writeAddr = 0;
	unsigned char buf[WRITE_CHUNK_SIZE_D3 + 3];
	unsigned char regAdd[8] = {0};
	int cnt;

	//==================== System reset ====================
	//System Reset ==> F7 52 34
	regAdd[0] = 0xF7;
	regAdd[1] = 0x52;
	regAdd[2] = 0x34;
	info->fts_write_reg(info, &regAdd[0], 3);
	fts_delay(200);

	//==================== Unlock Flash ====================
	//Unlock Flash Command ==> F7 74 45
	regAdd[0] = 0xF7;
	regAdd[1] = 0x74;
	regAdd[2] = 0x45;
	info->fts_write_reg(info, &regAdd[0], 3);
	fts_delay(100);

	//==================== Unlock Erase Operation ====================
	regAdd[0] = 0xFA;
	regAdd[1] = 0x72;
	regAdd[2] = 0x01;
	info->fts_write_reg(info, &regAdd[0], 3);
	fts_delay(100);

	//==================== Erase Partial Flash ====================
	for (i = 0; i < 64; i++) {
		if ( (i == 61) || (i == 62) )   // skip CX2 area (page 61 and page 62)
			continue;

		regAdd[0] = 0xFA;
		regAdd[1] = 0x02;
		regAdd[2] = (0x80 + i) & 0xFF;
		info->fts_write_reg(info, &regAdd[0], 3);
		rc = FTS_Check_Erase_Done(info);
		if (rc < 0)
			return rc;
	}

	//==================== Unlock Programming operation ====================
	regAdd[0] = 0xFA;
	regAdd[1] = 0x72;
	regAdd[2] = 0x02;
	info->fts_write_reg(info, &regAdd[0], 3);

	//========================== Write to FLASH ==========================
	// Main Code Programming
	i = 0;
	k = 0;
	size = FTS_CODE_SIZE;

	while(i < size) {
		j = 0;
		dataLen = size - i;

		while ((j < DRAM_LEN) && (j < dataLen)) {	//DRAM_LEN = 64*1024
			writeAddr = j & 0xFFFF;

			cnt = 0;
			buf[cnt++] = 0xF8;
			buf[cnt++] = (writeAddr >> 8) & 0xFF;
			buf[cnt++] = (writeAddr >> 0) & 0xFF;

			memcpy(&buf[cnt], &fw_data[sizeof(struct fts_header) + i], WRITE_CHUNK_SIZE_D3);
			cnt += WRITE_CHUNK_SIZE_D3;

			info->fts_write_reg(info, &buf[0], cnt);

			i += WRITE_CHUNK_SIZE_D3;
			j += WRITE_CHUNK_SIZE_D3;
		}
		input_info(true, info->dev, "%s: Write to Flash - Total %ld bytes\n", __func__, i);

		 //===================configure flash DMA=====================
		len = j / 4 - 1; 	// 64*1024 / 4 - 1

		buf[0] = 0xFA;
		buf[1] = 0x06;
		buf[2] = 0x00;
		buf[3] = 0x00;
		buf[4] = (CODE_ADDR_START +( (k * DRAM_LEN) >> 2)) & 0xFF;			// k * 64 * 1024 / 4
		buf[5] = (CODE_ADDR_START + ((k * DRAM_LEN) >> (2+8))) & 0xFF;		// k * 64 * 1024 / 4 / 256
		buf[6] = (len >> 0) & 0xFF;    //DMA length in word
		buf[7] = (len >> 8) & 0xFF;    //DMA length in word
		buf[8] = 0x00;
		info->fts_write_reg(info, &buf[0], 9);

		fts_delay(100);

		//===================START FLASH DMA=====================
		buf[0] = 0xFA;
		buf[1] = 0x05;
		buf[2] = 0xC0;
		info->fts_write_reg(info, &buf[0], 3);

		rc = FTS_Check_DMA_Done(info);
		if (rc < 0)
			return rc;
		k++;
	}
	input_info(true, info->dev, "%s: Total write %ld kbytes for Main Code\n", __func__, i / 1024);

	//=============================================================
	// Config Programming
	//=============================================================

	config_data = kzalloc(FTS_CONFIG_SIZE, GFP_KERNEL);
	if (!config_data) {
		input_err(true, info->dev, "%s: failed to alloc mem\n",
				__func__);
		return -ENOMEM;
	}

	memcpy(&config_data[0], &fw_data[sizeof(struct fts_header) + (252 * 1024)], FTS_CONFIG_SIZE);

	i = 0;
	size = FTS_CONFIG_SIZE;
	j = 0;
	while ((j < DRAM_LEN) && (j < size)) {		//DRAM_LEN = 64*1024
		writeAddr = j & 0xFFFF;

		cnt = 0;
		buf[cnt++] = 0xF8;
		buf[cnt++] = (writeAddr >> 8) & 0xFF;
		buf[cnt++] = (writeAddr >> 0) & 0xFF;

		memcpy(&buf[cnt], &config_data[i], WRITE_CHUNK_SIZE_D3);
		cnt += WRITE_CHUNK_SIZE_D3;

		info->fts_write_reg(info, &buf[0], cnt);

		i += WRITE_CHUNK_SIZE_D3;
		j += WRITE_CHUNK_SIZE_D3;
	}
	kfree(config_data);

	//===================configure flash DMA=====================
	len = j / 4 - 1;

	buf[0] = 0xFA;
	buf[1] = 0x06;
	buf[2] = 0x00;
	buf[3] = 0x00;
	buf[4] = (CONFIG_ADDR_START) & 0xFF;
	buf[5] = (CONFIG_ADDR_START >> 8) & 0xFF;
	buf[6] = (len >> 0) & 0xFF;    //DMA length in word
	buf[7] = (len >> 8) & 0xFF;    //DMA length in word
	buf[8] = 0x00;
	info->fts_write_reg(info, &buf[0], 9);

	fts_delay(100);

	//===================START FLASH DMA=====================
	buf[0] = 0xFA;
	buf[1] = 0x05;
	buf[2] = 0xC0;
	info->fts_write_reg(info, &buf[0], 3);

	rc = FTS_Check_DMA_Done(info);
	if (rc < 0)
		return rc;

	input_info(true, info->dev, "%s: Total write %ld kbytes for Config\n", __func__, i / 1024);

	//==================== System reset ====================
	//System Reset ==> F7 52 34
	regAdd[0] = 0xF7;		regAdd[1] = 0x52;		regAdd[2] = 0x34;
	info->fts_write_reg(info, &regAdd[0],3);

	return 0;
}

int fts_fw_wait_for_event(struct fts_ts_info *info, unsigned char eid)
{
	int rc;
	unsigned char regAdd;
	unsigned char data[FTS_EVENT_SIZE];
	int retry = 0;

	memset(data, 0x0, FTS_EVENT_SIZE);

	regAdd = READ_ONE_EVENT;
	rc = -1;
	while (info->fts_read_reg(info, &regAdd, 1, (unsigned char *)data, FTS_EVENT_SIZE)) {
		if (data[0] == EVENTID_STATUS_EVENT || data[0] == EVENTID_ERROR) {
			if ((data[0] == EVENTID_STATUS_EVENT) && (data[1] == eid)) {
				rc = 0;
				break;
			} else {
				input_info(true, info->dev, "%s: %2X,%2X,%2X,%2X\n", __func__, data[0],data[1],data[2],data[3]);
			}
		}
		if (retry++ > FTS_RETRY_COUNT * 15) {
			rc = -1;
			input_err(true, info->dev, "%s: Time Over (%2X,%2X,%2X,%2X)\n", __func__, data[0],data[1],data[2],data[3]);
			break;
		}
		fts_delay(20);
	}

	return rc;
}

int fts_fw_wait_for_event_D3(struct fts_ts_info *info, unsigned char eid0, unsigned char eid1)
{
	int rc;
	unsigned char regAdd;
	unsigned char data[FTS_EVENT_SIZE];
	int retry = 0;

	memset(data, 0x0, FTS_EVENT_SIZE);

	regAdd = READ_ONE_EVENT;
	rc = -1;
	while (info->fts_read_reg(info, &regAdd, 1, (unsigned char *)data, FTS_EVENT_SIZE)) {
		if (data[0] == EVENTID_STATUS_EVENT || data[0] == EVENTID_ERROR) {
			if ((data[0] == EVENTID_STATUS_EVENT) && (data[1] == eid0) && (data[2] == eid1)) {
				rc = 0;
				break;
			} else {
				input_info(true, info->dev, "%s: %2X,%2X,%2X,%2X\n", __func__, data[0],data[1],data[2],data[3]);
			}
		}
		if (retry++ > FTS_RETRY_COUNT * 15) {
			rc = -1;
			input_err(true, info->dev, "%s: Time Over (%2X,%2X,%2X,%2X)\n", __func__, data[0],data[1],data[2],data[3]);
			break;
		}
		fts_delay(20);
	}

	return rc;
}

int fts_fw_wait_for_specific_event(struct fts_ts_info *info, unsigned char eid0, unsigned char eid1, unsigned char eid2)
>>>>>>> origin/3.18.14.x
{
	int rc;
	unsigned char regAdd;
	unsigned char data[FTS_EVENT_SIZE];
	int retry = 0;

	memset(data, 0x0, FTS_EVENT_SIZE);

	regAdd = READ_ONE_EVENT;
	rc = -1;
<<<<<<< HEAD
	while (info->fts_read_reg
	       (info, &regAdd, 1, (unsigned char *)data, FTS_EVENT_SIZE)) {
		if ((data[0] == EVENTID_STATUS_EVENT) &&
			(data[1] == eid)) {
			rc = 0;
			break;
		}

		if (retry++ > FTS_RETRY_COUNT * 15) {
			rc = -1;
			tsp_debug_info(true, info->dev, "%s: Time Over\n", __func__);
			break;
		}
		msleep(20);
=======
	while (info->fts_read_reg(info, &regAdd, 1, (unsigned char *)data, FTS_EVENT_SIZE)) {
		if (data[0]) {
			if ((data[0] == eid0) && (data[1] == eid1) && (data[2] == eid2)) {
				rc = 0;
				break;
			} else {
				input_info(true, info->dev, "%s: %2X,%2X,%2X,%2X\n", __func__, data[0],data[1],data[2],data[3]);
			}
		}
		if (retry++ > FTS_RETRY_COUNT * 15) {
			rc = -1;
			input_err(true, info->dev, "%s: Time Over ( %2X,%2X,%2X,%2X )\n", __func__, data[0],data[1],data[2],data[3]);
			break;
		}
		fts_delay(20);
>>>>>>> origin/3.18.14.x
	}

	return rc;
}

void fts_execute_autotune(struct fts_ts_info *info)
{
<<<<<<< HEAD
	info->fts_command(info, CX_TUNNING);
	msleep(300);
	fts_fw_wait_for_event(info, STATUS_EVENT_MUTUAL_AUTOTUNE_DONE);

#ifdef FTS_SUPPORT_WATER_MODE
	fts_fw_wait_for_event (info, STATUS_EVENT_WATER_SELF_AUTOTUNE_DONE);
	fts_fw_wait_for_event(info, STATUS_EVENT_SELF_AUTOTUNE_DONE);
#endif
#ifdef FTS_SUPPORT_SELF_MODE
	info->fts_command(info, SELF_AUTO_TUNE);
	msleep(300);
	fts_fw_wait_for_event(info, STATUS_EVENT_SELF_AUTOTUNE_DONE);
#endif

	info->fts_command(info, FTS_CMD_SAVE_CX_TUNING);
	msleep(230);
	fts_fw_wait_for_event(info, STATUS_EVENT_FLASH_WRITE_CXTUNE_VALUE);
}

void fts_fw_init(struct fts_ts_info *info)
{
	info->fts_command(info, SLEEPOUT);
	msleep(50);

	if (info->digital_rev == FTS_DIGITAL_REV_2) {
		info->fts_command(info, FTS_CMD_TRIM_LOW_POWER_OSCILLATOR);
		msleep(300);
	}

	fts_execute_autotune(info);

	info->fts_command(info, SLEEPOUT);
	msleep(50);

	info->fts_command(info, SENSEON);

#ifdef FTS_SUPPORT_WATER_MODE
	fts_fw_wait_for_event(info, STATUS_EVENT_WATER_SELF_DONE);
#else
	fts_fw_wait_for_event (info, STATUS_EVENT_FORCE_CAL_DONE);
#endif
=======
	input_info(true, info->dev, "%s: start\n", __func__);

	info->fts_command(info, CX_TUNNING);
	fts_delay(300);
	fts_fw_wait_for_event_D3(info, STATUS_EVENT_MUTUAL_AUTOTUNE_DONE, 0x00);
	info->fts_command(info, SELF_AUTO_TUNE);
	fts_delay(300);
	fts_fw_wait_for_event_D3(info, STATUS_EVENT_SELF_AUTOTUNE_DONE_D3, 0x00);

	info->fts_command(info, FTS_CMD_SAVE_CX_TUNING);
	fts_delay(230);
	fts_fw_wait_for_event(info, STATUS_EVENT_FLASH_WRITE_CXTUNE_VALUE);

	info->fts_command(info, FTS_CMD_SAVE_FWCONFIG);
	fts_delay(230);
	fts_fw_wait_for_event(info, STATUS_EVENT_FLASH_WRITE_CONFIG);

	/* Reset FTS */
	info->fts_systemreset(info, 30);
}

void fts_fw_init(struct fts_ts_info *info, bool magic_cal)
{
	input_info(true, info->dev, "%s: magic_cal(%d)\n", __func__,magic_cal);

	info->fts_command(info, FTS_CMD_TRIM_LOW_POWER_OSCILLATOR);
	fts_delay(200);

#ifdef PAT_CONTROL
	/* count the number of calibration */
	fts_get_calibration_information(info);

	if ((info->cal_count == 0) || (info->cal_count == 0xFF) || (magic_cal == true)) {
		input_err(true, info->dev, "%s: RUN OFFSET CALIBRATION(0x%02X)\n", __func__, info->cal_count);
		fts_execute_autotune(info);
		if (magic_cal) {
			if (info->cal_count == 0x00 || info->cal_count == 0xFF)
				info->cal_count = PAT_MAGIC_NUMBER;
			else if (PAT_MAGIC_NUMBER <= info->cal_count && info->cal_count < PAT_MAX_MAGIC)
				info->cal_count++;
			input_info(true, info->dev, "%s: write to nvm %X\n",
						__func__, info->cal_count);
		}

		/* get ic information */
		info->fts_get_version_info(info);

		info->tune_fix_ver = info->fw_main_version_of_ic;
		fts_set_calibration_information(info, info->cal_count, info->fw_main_version_of_ic);

		fts_get_calibration_information(info);

		input_info(true, info->dev, "%s: tune_fix_ver [0x%4X]\n", __func__, info->tune_fix_ver);
	} else {
		input_err(true, info->dev, "%s: DO NOT CALIBRATION(0x%02X)\n", __func__, info->cal_count);
	}
#endif

	info->fts_command(info, SENSEON);
#ifdef FTS_SUPPORT_PRESSURE_SENSOR
	info->fts_command(info, FTS_CMD_PRESSURE_SENSE_ON);
#endif
	fts_fw_wait_for_event (info, STATUS_EVENT_FORCE_CAL_DONE_D3);
>>>>>>> origin/3.18.14.x

#ifdef FTS_SUPPORT_TOUCH_KEY
	if (info->board->support_mskey)
		info->fts_command(info, FTS_CMD_KEY_SENSE_ON);
#endif
<<<<<<< HEAD
}

const int fts_fw_updater(struct fts_ts_info *info, unsigned char *fw_data)
{
	const struct fts64_header *header;
	int retval;
	int retry;
	unsigned short fw_main_version;

	if (!fw_data) {
		tsp_debug_err(true, info->dev, "%s: Firmware data is NULL\n",
=======

	fts_interrupt_set(info, INT_ENABLE);
	fts_delay(20);
}

const int fts_fw_updater(struct fts_ts_info *info, unsigned char *fw_data, int restore_cal)
{
	const struct fts_header *header;
	int retval;
	int retry;
	unsigned short fw_main_version;
	bool magic_cal = false;

	if (!fw_data) {
		input_err(true, info->dev, "%s: Firmware data is NULL\n",
>>>>>>> origin/3.18.14.x
			__func__);
		return -ENODEV;
	}

<<<<<<< HEAD
	header = (struct fts64_header *)fw_data;
	fw_main_version = (header->released_ver[0] << 8) +
							(header->released_ver[1]);

	tsp_debug_info(true, info->dev,
		  "Starting firmware update : 0x%04X\n",
=======
	header = (struct fts_header *)fw_data;
	fw_main_version = (header->released_ver[0] << 8) + (header->released_ver[1]);

	input_info(true, info->dev,
		  "%s: Starting firmware update : 0x%04X\n", __func__,
>>>>>>> origin/3.18.14.x
		  fw_main_version);

	retry = 0;
	while (1) {
<<<<<<< HEAD
		retval = fts_fw_burn(info, fw_data);
=======

		retval = fts_fw_burn_d3(info, fw_data);
>>>>>>> origin/3.18.14.x
		if (retval >= 0) {
			info->fts_wait_for_ready(info);
			info->fts_get_version_info(info);

#ifdef FTS_SUPPORT_NOISE_PARAM
			info->fts_get_noise_param_address(info);
#endif

			if (fw_main_version == info->fw_main_version_of_ic) {
<<<<<<< HEAD
				tsp_debug_info(true, info->dev,
					  "%s: Success Firmware update\n",
					  __func__);
				fts_fw_init(info);
=======
				input_info(true, info->dev,
					  "%s: Success Firmware update\n",
					  __func__);

#ifdef PAT_CONTROL
				if (restore_cal) {
					if(info->board->pat_function == PAT_CONTROL_PAT_MAGIC) {
						/* NOT to control cal count that was marked on external factory ( E0~E5 )*/
						if ((info->cal_count >= PAT_MAGIC_NUMBER) && (info->cal_count < PAT_MAX_MAGIC))
							magic_cal = true;
					}
				}
				input_info(true, info->dev, "%s: cal_count(0x%02X) pat_function dt(%d) restore_cal(%d) magic_cal(%d)\n",
					__func__,info->cal_count,info->board->pat_function,restore_cal,magic_cal);
#endif
				fts_fw_init(info, magic_cal);
>>>>>>> origin/3.18.14.x
				retval = 0;
				break;
			}
		}

		if (retry++ > 3) {
<<<<<<< HEAD
			tsp_debug_err(true, info->dev, "%s: Fail Firmware update\n",
=======
			input_err(true, info->dev, "%s: Fail Firmware update\n",
>>>>>>> origin/3.18.14.x
				 __func__);
			retval = -1;
			break;
		}
	}

	return retval;
}
EXPORT_SYMBOL(fts_fw_updater);

<<<<<<< HEAD
#define FW_IMAGE_NAME_D2_TB_INTEG			"tsp_stm/stm_tb_integ.fw"
#define FW_IMAGE_NAME_D2_Z2A			"tsp_stm/stm_z2a.fw"
#define FW_IMAGE_NAME_D2_Z2I			"tsp_stm/stm_z2i.fw"
#define FW_IMAGE_NAME_D2_Z1			"tsp_stm/stm_z1.fw"
#define CONFIG_ID_D1_S				0x2C
#define CONFIG_ID_D2_TR				0x2E
#define CONFIG_ID_D2_TB				0x30
#define CONFIG_OFFSET_BIN_D1			0xf822
#define CONFIG_OFFSET_BIN_D2			0x1E822
#define RX_OFFSET_BIN_D2				0x1E834
#define TX_OFFSET_BIN_D2				0x1E835

static bool fts_skip_firmware_update(struct fts_ts_info *info, unsigned char *fw_data)
{
	int config_id, num_rx, num_tx;

	config_id = fw_data[CONFIG_OFFSET_BIN_D2];
	num_rx = fw_data[RX_OFFSET_BIN_D2];
	num_tx = fw_data[TX_OFFSET_BIN_D2];

	if (strncmp(info->board->project_name, "TB", 2) == 0) {
		if (config_id != CONFIG_ID_D2_TB) {
			tsp_debug_info(true, info->dev, "%s: Skip update because config ID(%0x2X) is mismatched.\n",
				__func__,config_id);
			// return true; // tempory blocking to update f/w for TB, it will be remove later.. Xtopher
		}

		if ((num_rx != info->board->SenseChannelLength)
			|| (num_tx != info->board->ForceChannelLength)) {
			tsp_debug_info(true, info->dev,
				"%s: Skip update because revision is mismatched. rx[%d] tx[%d]\n",
				__func__, num_rx, num_tx);
			//return true;  // tempory blocking to update f/w for TB, it will be remove later.. Xtopher
		}
	} else if (strncmp(info->board->project_name, "TR", 2) == 0) {
		if (config_id != CONFIG_ID_D2_TR) {
			tsp_debug_info(true, info->dev,
				"%s: Skip update because config ID is mismatched. config_id[%d]\n",
				__func__, config_id);
			return true;
		}
	} else
		goto out;

out:
	return false;
}
=======
#define FW_IMAGE_NAME_FTS8			"tsp_stm/fts8cd56_dream2.fw"
>>>>>>> origin/3.18.14.x

int fts_fw_update_on_probe(struct fts_ts_info *info)
{
	int retval = 0;
	const struct firmware *fw_entry = NULL;
	unsigned char *fw_data = NULL;
	char fw_path[FTS_MAX_FW_PATH];
<<<<<<< HEAD
	const struct fts64_header *header;
	unsigned char SYS_STAT[2] = {0, };
	int tspid = 0;

	if (info->board->firmware_name)
		info->firmware_name = info->board->firmware_name;
	else {
#if 0
		if ((lcdtype == S6E3HF2_WQXGA_ID1) || (lcdtype == S6E3HF2_WQXGA_ID2)) {
			info->firmware_name = FW_IMAGE_NAME_D2_TB_INTEG;
		} else {
			tspid = gpio_get_value(info->board->tspid);
			if (tspid)
				info->firmware_name = FW_IMAGE_NAME_D2_Z2I;
			else
				info->firmware_name = FW_IMAGE_NAME_D2_Z2A;
		}
#endif
		return -1;
	}

	snprintf(fw_path, FTS_MAX_FW_PATH, "%s", info->firmware_name);
	tsp_debug_info(true, info->dev, "%s: Load firmware : %s, Digital_rev : %d, TSP_ID : %d\n", __func__,
		  fw_path, info->digital_rev, tspid);
	retval = request_firmware(&fw_entry, fw_path, info->dev);
	if (retval) {
		tsp_debug_err(true, info->dev,
=======
	const struct fts_header *header;
	int restore_cal = 0;
	bool cal_clear = false;

	if (info->board->bringup == 1)
		return 0;

	if (info->board->firmware_name) {
		if (gpio_is_valid(info->board->device_id) && gpio_get_value(info->board->device_id)) {
			retval = fts_read_analog_chip_id(info, ANALOG_ID_FTS8);
			if (retval == 1)
				info->firmware_name = FW_IMAGE_NAME_FTS8;
			else
				info->firmware_name = info->board->firmware_name;
		}
		else
			info->firmware_name = info->board->firmware_name;
	}
	else {
		input_err(true, info->dev,"%s: firmware name does not declair in dts\n", __func__);
		goto exit_fwload;
	}

	snprintf(fw_path, FTS_MAX_FW_PATH, "%s", info->firmware_name);
	input_info(true, info->dev, "%s: Load firmware : %s, TSP_ID : %d\n", __func__, fw_path, info->board->tsp_id);

	retval = request_firmware(&fw_entry, fw_path, info->dev);
	if (retval) {
		input_err(true, info->dev,
>>>>>>> origin/3.18.14.x
			"%s: Firmware image %s not available\n", __func__,
			fw_path);
		goto done;
	}

<<<<<<< HEAD
	if (info->digital_rev == FTS_DIGITAL_REV_1 && fw_entry->size!=(FW_IMAGE_SIZE_D1 + sizeof(struct fts64_header))) {
		tsp_debug_err(true, info->dev,
			"%s: Firmware image %s not available for FTS D1\n", __func__,
			fw_path);
		goto done;
	}

	if (info->digital_rev == FTS_DIGITAL_REV_2 && fw_entry->size!=(FW_IMAGE_SIZE_D2 + sizeof(struct fts64_header))) {
		tsp_debug_err(true, info->dev,
			"%s: Firmware image %s not available for FTS D2\n", __func__,
=======
	if (fw_entry->size != (FW_IMAGE_SIZE_D3 + sizeof(struct fts_header))) {
		input_err(true, info->dev,
			"%s: Firmware image %s not available for FTS D3\n", __func__,
>>>>>>> origin/3.18.14.x
			fw_path);
		goto done;
	}

	fw_data = (unsigned char *)fw_entry->data;
<<<<<<< HEAD
	header = (struct fts64_header *)fw_data;

	info->fw_version_of_bin = (fw_data[5] << 8)+fw_data[4];
	info->fw_main_version_of_bin = (header->released_ver[0] << 8) +
								(header->released_ver[1]);
	if (info->digital_rev == FTS_DIGITAL_REV_1)
		info->config_version_of_bin = (fw_data[CONFIG_OFFSET_BIN_D1] << 8) + fw_data[CONFIG_OFFSET_BIN_D1 - 1];
	else
		info->config_version_of_bin = (fw_data[CONFIG_OFFSET_BIN_D2] << 8) + fw_data[CONFIG_OFFSET_BIN_D2 - 1];

	tsp_debug_info(true, info->dev,
					"Bin Firmware Version : 0x%04X "
					"Bin Config Version : 0x%04X "
					"Bin Main Version : 0x%04X\n",
					info->fw_version_of_bin,
					info->config_version_of_bin,
					info->fw_main_version_of_bin);

	if (fts_skip_firmware_update(info, fw_data))
		goto done;

	if ((info->fw_main_version_of_ic < info->fw_main_version_of_bin)
		|| ((info->config_version_of_ic < info->config_version_of_bin))
		|| ((info->fw_version_of_ic < info->fw_version_of_bin)))
		retval = fts_fw_updater(info, fw_data);
	else
		retval = FTS_NOT_ERROR;

	if (fts_get_system_status(info, &SYS_STAT[0], &SYS_STAT[1]) >= 0) {
		if (SYS_STAT[0] != SYS_STAT[1]) {
			info->fts_systemreset(info);
			msleep(20);
			info->fts_wait_for_ready(info);
			fts_fw_init(info);
		}
	}

done:
	if (fw_entry)
		release_firmware(fw_entry);

=======
	header = (struct fts_header *)fw_data;

	info->fw_version_of_bin = (fw_data[5] << 8)+fw_data[4];
	info->fw_main_version_of_bin = (header->released_ver[0] << 8) + (header->released_ver[1]);
	info->config_version_of_bin = (fw_data[CONFIG_OFFSET_BIN_D3] << 8) + fw_data[CONFIG_OFFSET_BIN_D3 - 1];

	input_info(true, info->dev,
		"%s: [BIN] Firmware Ver: 0x%04X, Config Ver: 0x%04X, Main Ver: 0x%04X\n", __func__,
		info->fw_version_of_bin,
		info->config_version_of_bin,
		info->fw_main_version_of_bin);

	if (info->board->bringup == 2) {
		input_err(true, info->dev, "%s: skip fw_update for bringup\n", __func__);
		retval = FTS_NOT_ERROR;
		goto done;
	}

#ifdef PAT_CONTROL
	if ((info->fw_main_version_of_ic < info->fw_main_version_of_bin)
		|| ((info->config_version_of_ic < info->config_version_of_bin))
		|| ((info->fw_version_of_ic < info->fw_version_of_bin)))
		retval = FTS_NEED_FW_UPDATE;
	else
		retval = FTS_NOT_ERROR;

	/* ic fw ver > bin fw ver && force is false*/
	if (retval != FTS_NEED_FW_UPDATE) {
		/* clear nv,  forced f/w update eventhough same f/w,  then apply pat magic */
		if (info->board->pat_function == PAT_CONTROL_FORCE_UPDATE) {
			input_info(true, info->dev, "%s: run forced f/w update and excute autotune\n", __func__);
		} else {
			input_info(true, info->dev, "%s: skip fw update & nv read\n", __func__);
			goto done;
		}
	}

	/* load  pat information variable from ic */
	fts_get_calibration_information(info);
	input_info(true, info->dev, "%s: tune_fix_ver [%04X] afe_base [%04X]\n", __func__, info->tune_fix_ver, info->board->afe_base);

	/* initialize default flash value from 0xff to 0x00  for stm */
	if (info->cal_count == 0xFF) {
		cal_clear = true;
		info->tune_fix_ver = 0x0000;
	}

	/* check dt to clear pat */
	if (info->board->pat_function == PAT_CONTROL_CLEAR_NV
		|| info->board->pat_function == PAT_CONTROL_FORCE_UPDATE) {
		cal_clear = true;
	}

	/* mismatch calibration - ic has too old calibration data after pat enabled*/
	if (info->board->afe_base > info->tune_fix_ver) {
		restore_cal = 1;
		cal_clear = true;
	}

	if (cal_clear) {
		fts_set_calibration_information(info, PAT_COUNT_ZERO, info->tune_fix_ver);
		fts_get_calibration_information(info);
	}

	retval = fts_fw_updater(info, fw_data, restore_cal);

	/* change cal_count from 0 to magic number to make virtual pure auto tune */
	if ((info->cal_count == 0 && info->board->pat_function == PAT_CONTROL_PAT_MAGIC )||
			info->board->pat_function == PAT_CONTROL_FORCE_UPDATE) {
		fts_set_calibration_information(info, PAT_MAGIC_NUMBER, info->tune_fix_ver);
		fts_get_calibration_information(info);
	}
#else
	if ((info->fw_main_version_of_ic < info->fw_main_version_of_bin)
		|| ((info->config_version_of_ic < info->config_version_of_bin))
		|| ((info->fw_version_of_ic < info->fw_version_of_bin)))
		retval = fts_fw_updater(info, fw_data, restore_cal);
	else
		retval = FTS_NOT_ERROR;
#endif

done:
	if (fw_entry)
		release_firmware(fw_entry);
exit_fwload:
>>>>>>> origin/3.18.14.x
	return retval;
}
EXPORT_SYMBOL(fts_fw_update_on_probe);

static int fts_load_fw_from_kernel(struct fts_ts_info *info,
				 const char *fw_path)
{
	int retval;
	const struct firmware *fw_entry = NULL;
	unsigned char *fw_data = NULL;

	if (!fw_path) {
<<<<<<< HEAD
		tsp_debug_err(true, info->dev, "%s: Firmware name is not defined\n",
=======
		input_err(true, info->dev, "%s: Firmware name is not defined\n",
>>>>>>> origin/3.18.14.x
			__func__);
		return -EINVAL;
	}

<<<<<<< HEAD
	tsp_debug_info(true, info->dev, "%s: Load firmware : %s\n", __func__,
=======
	input_info(true, info->dev, "%s: Load firmware : %s\n", __func__,
>>>>>>> origin/3.18.14.x
		 fw_path);

	retval = request_firmware(&fw_entry, fw_path, info->dev);

	if (retval) {
<<<<<<< HEAD
		tsp_debug_err(true, info->dev,
=======
		input_err(true, info->dev,
>>>>>>> origin/3.18.14.x
			"%s: Firmware image %s not available\n", __func__,
			fw_path);
		goto done;
	}

	fw_data = (unsigned char *)fw_entry->data;

<<<<<<< HEAD
	if (fts_skip_firmware_update(info, fw_data))
		goto done;

	if (info->irq)
		disable_irq(info->irq);
	else
		hrtimer_cancel(&info->timer);

	info->fts_systemreset(info);
	info->fts_wait_for_ready(info);

	retval = fts_fw_updater(info, fw_data);
	if (retval)
		tsp_debug_err(true, info->dev, "%s: failed update firmware\n",
			__func__);

	if (info->irq)
		enable_irq(info->irq);
	else
		hrtimer_start(&info->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
=======
	disable_irq(info->irq);

	info->fts_systemreset(info, 10);

	/* use virtual pat_control - magic cal 1 */
	retval = fts_fw_updater(info, fw_data, 1);
	if (retval)
		input_err(true, info->dev, "%s: failed update firmware\n",
			__func__);

	enable_irq(info->irq);
>>>>>>> origin/3.18.14.x
 done:
	if (fw_entry)
		release_firmware(fw_entry);

	return retval;
}

static int fts_load_fw_from_ums(struct fts_ts_info *info)
{
	struct file *fp;
	mm_segment_t old_fs;
	long fw_size, nread;
	int error = 0;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	fp = filp_open(FTS_DEFAULT_UMS_FW, O_RDONLY, S_IRUSR);
	if (IS_ERR(fp)) {
<<<<<<< HEAD
		tsp_debug_err(true, info->dev, "%s: failed to open %s.\n", __func__,
=======
		input_err(true, info->dev, "%s: failed to open %s.\n", __func__,
>>>>>>> origin/3.18.14.x
			FTS_DEFAULT_UMS_FW);
		error = -ENOENT;
		goto open_err;
	}

	fw_size = fp->f_path.dentry->d_inode->i_size;

	if (0 < fw_size) {
		unsigned char *fw_data;
<<<<<<< HEAD
		const struct fts64_header *header;
		fw_data = kzalloc(fw_size, GFP_KERNEL);
		nread = vfs_read(fp, (char __user *)fw_data,
				 fw_size, &fp->f_pos);

		tsp_debug_info(true, info->dev,
=======
		const struct fts_header *header;

		fw_data = kzalloc(fw_size, GFP_KERNEL);
		if (!fw_data) {
			input_err(true, info->dev, "%s: failed to alloc mem\n",
					__func__);
			error = -ENOMEM;
			goto alloc_err;
		}

		nread = vfs_read(fp, (char __user *)fw_data,
				 fw_size, &fp->f_pos);

		input_info(true, info->dev,
>>>>>>> origin/3.18.14.x
			 "%s: start, file path %s, size %ld Bytes\n",
			 __func__, FTS_DEFAULT_UMS_FW, fw_size);

		if (nread != fw_size) {
<<<<<<< HEAD
			tsp_debug_err(true, info->dev,
=======
			input_err(true, info->dev,
>>>>>>> origin/3.18.14.x
				"%s: failed to read firmware file, nread %ld Bytes\n",
				__func__, nread);
			error = -EIO;
		} else {
<<<<<<< HEAD
			if (fts_skip_firmware_update(info, fw_data))
				goto done;

			header = (struct fts64_header *)fw_data;
			if (header->signature == FTS64FILE_SIGNATURE) {
				if (info->irq)
					disable_irq(info->irq);
				else
					hrtimer_cancel(&info->timer);

				info->fts_systemreset(info);
				info->fts_wait_for_ready(info);

				tsp_debug_info(true, info->dev,
					"[UMS] Firmware Version : 0x%04X "
					"[UMS] Main Version : 0x%04X\n",
					(fw_data[5] << 8)+fw_data[4],
					(header->released_ver[0] << 8) +
							(header->released_ver[1]));

				error = fts_fw_updater(info, fw_data);

				if (info->irq)
					enable_irq(info->irq);
				else
					hrtimer_start(&info->timer,
						  ktime_set(1, 0),
						  HRTIMER_MODE_REL);
			} else {
				error = -1;
				tsp_debug_err(true, info->dev,
=======
			header = (struct fts_header *)fw_data;
			if (header->signature == FTSFILE_SIGNATURE) {

				disable_irq(info->irq);

				info->fts_systemreset(info, 10);

				input_info(true, info->dev,
					"%s: [UMS] Firmware Ver: 0x%04X, Main Version : 0x%04X\n",
					__func__, (fw_data[5] << 8)+fw_data[4],
					(header->released_ver[0] << 8) +
					(header->released_ver[1]));

				/* use virtual pat_control - magic cal 1 */
				error = fts_fw_updater(info, fw_data, 1);

				enable_irq(info->irq);

			} else {
				error = -1;
				input_err(true, info->dev,
>>>>>>> origin/3.18.14.x
					 "%s: File type is not match with FTS64 file. [%8x]\n",
					 __func__, header->signature);
			}
		}

		if (error < 0)
<<<<<<< HEAD
			tsp_debug_err(true, info->dev, "%s: failed update firmware\n",
				__func__);

done:
		kfree(fw_data);
	}

=======
			input_err(true, info->dev, "%s: failed update firmware\n",
				__func__);

		kfree(fw_data);
	}

alloc_err:
>>>>>>> origin/3.18.14.x
	filp_close(fp, NULL);

 open_err:
	set_fs(old_fs);
	return error;
}

static int fts_load_fw_from_ffu(struct fts_ts_info *info)
{
	int retval;
	const struct firmware *fw_entry = NULL;
	unsigned char *fw_data = NULL;
	const char *fw_path = FTS_DEFAULT_FFU_FW;
<<<<<<< HEAD
	const struct fts64_header *header;

	if (!fw_path) {
		tsp_debug_err(true, info->dev, "%s: Firmware name is not defined\n",
=======
	const struct fts_header *header;

	if (!fw_path) {
		input_err(true, info->dev, "%s: Firmware name is not defined\n",
>>>>>>> origin/3.18.14.x
			__func__);
		return -EINVAL;
	}

<<<<<<< HEAD
	tsp_debug_info(true, info->dev, "%s: Load firmware : %s\n", __func__,
=======
	input_info(true, info->dev, "%s: Load firmware : %s\n", __func__,
>>>>>>> origin/3.18.14.x
		 fw_path);

	retval = request_firmware(&fw_entry, fw_path, info->dev);

	if (retval) {
<<<<<<< HEAD
		tsp_debug_err(true, info->dev,
=======
		input_err(true, info->dev,
>>>>>>> origin/3.18.14.x
			"%s: Firmware image %s not available\n", __func__,
			fw_path);
		goto done;
	}

<<<<<<< HEAD
	if ((info->digital_rev == FTS_DIGITAL_REV_2) &&
		(fw_entry->size != (FW_IMAGE_SIZE_D2 + sizeof(struct fts64_header) + SIGNEDKEY_SIZE))) {
		tsp_debug_err(true, info->dev,
=======
	if (fw_entry->size != (FW_IMAGE_SIZE_D3 + sizeof(struct fts_header) + SIGNEDKEY_SIZE)) {
		input_err(true, info->dev,
>>>>>>> origin/3.18.14.x
			"%s: Unsigned firmware %s is not available, %ld\n", __func__,
			fw_path, fw_entry->size);
		retval = -EPERM;
		goto done;
	}

	fw_data = (unsigned char *)fw_entry->data;
<<<<<<< HEAD
	header = (struct fts64_header *)fw_data;

	info->fw_version_of_bin = (fw_data[5] << 8)+fw_data[4];
	info->fw_main_version_of_bin = (header->released_ver[0] << 8) +
								(header->released_ver[1]);
	if (info->digital_rev == FTS_DIGITAL_REV_1)
		info->config_version_of_bin = (fw_data[CONFIG_OFFSET_BIN_D1] << 8) + fw_data[CONFIG_OFFSET_BIN_D1 - 1];
	else
		info->config_version_of_bin = (fw_data[CONFIG_OFFSET_BIN_D2] << 8) + fw_data[CONFIG_OFFSET_BIN_D2 - 1];

	tsp_debug_info(true, info->dev,
					"FFU Firmware Version : 0x%04X "
					"FFU Config Version : 0x%04X "
					"FFU Main Version : 0x%04X\n",
					info->fw_version_of_bin,
					info->config_version_of_bin,
					info->fw_main_version_of_bin);

	/* TODO : if you need to check firmware version between IC and Binary,
	 * add it this position.
	 */

	if (info->irq)
		disable_irq(info->irq);
	else
		hrtimer_cancel(&info->timer);

	info->fts_systemreset(info);
	info->fts_wait_for_ready(info);

	retval = fts_fw_updater(info, fw_data);
	if (retval)
		tsp_debug_err(true, info->dev, "%s: failed update firmware\n",
			__func__);

	if (info->irq)
		enable_irq(info->irq);
	else
		hrtimer_start(&info->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
 done:
=======
	header = (struct fts_header *)fw_data;

	info->fw_version_of_bin = (fw_data[5] << 8)+fw_data[4];
	info->fw_main_version_of_bin = (header->released_ver[0] << 8) + (header->released_ver[1]);
	info->config_version_of_bin = (fw_data[CONFIG_OFFSET_BIN_D3] << 8) + fw_data[CONFIG_OFFSET_BIN_D3 - 1];

	input_info(true, info->dev,
		"%s: [FFU] Firmware Ver: 0x%04X, Config Ver: 0x%04X, Main Ver: 0x%04X\n",
		__func__, info->fw_version_of_bin, info->config_version_of_bin,
		info->fw_main_version_of_bin);

	disable_irq(info->irq);

	info->fts_systemreset(info, 10);

	/* use virtual pat_control - magic cal 0 */
	retval = fts_fw_updater(info, fw_data, 0);
	if (retval)
		input_err(true, info->dev, "%s: failed update firmware\n", __func__);

	enable_irq(info->irq);

done:
>>>>>>> origin/3.18.14.x
	if (fw_entry)
		release_firmware(fw_entry);

	return retval;
}

int fts_fw_update_on_hidden_menu(struct fts_ts_info *info, int update_type)
{
	int retval = 0;

	/* Factory cmd for firmware update
	 * argument represent what is source of firmware like below.
	 *
	 * 0 : [BUILT_IN] Getting firmware which is for user.
	 * 1 : [UMS] Getting firmware from sd card.
	 * 2 : none
	 * 3 : [FFU] Getting firmware from air.
	 */
	switch (update_type) {
	case BUILT_IN:
		retval = fts_load_fw_from_kernel(info, info->firmware_name);
		break;

	case UMS:
		retval = fts_load_fw_from_ums(info);
		break;

	case FFU:
		retval = fts_load_fw_from_ffu(info);
		break;

	default:
<<<<<<< HEAD
		tsp_debug_err(true, info->dev, "%s: Not support command[%d]\n",
=======
		input_err(true, info->dev, "%s: Not support command[%d]\n",
>>>>>>> origin/3.18.14.x
			__func__, update_type);
		break;
	}

	return retval;
}
EXPORT_SYMBOL(fts_fw_update_on_hidden_menu);
<<<<<<< HEAD
=======

>>>>>>> origin/3.18.14.x
