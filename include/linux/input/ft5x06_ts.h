/*
 *
 * FocalTech ft5x06 TouchScreen driver header file.
 *
 * Copyright (c) 2010  Focal tech Ltd.
 * Copyright (c) 2012, The Linux Foundation. All Rights Reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef __LINUX_FT5X06_TS_H__
#define __LINUX_FT5X06_TS_H__

struct ft5x06_ts_platform_data {
	unsigned long irqflags;
	u32 x_max;
	u32 y_max;
	u32 irq_gpio;
	u32 reset_gpio;
	int (*power_init) (bool);
	int (*power_on) (bool);
};
#define FT5X0X_NAME "ft5x06_ts"

#ifdef FT5X0X_CREATE_APK_DEBUG_CHANNEL
#define PROC_UPGRADE			0
#define PROC_READ_REGISTER		1
#define PROC_WRITE_REGISTER		2
#define PROC_RAWDATA			3
#define PROC_AUTOCLB			4

#define FT5X0X_DEBUG_IF_NAME	"ft5x0x-debug"




enum ft5x0x_ts_regs {
	FT5X0X_REG_THGROUP					= 0x80,     /* touch threshold, related to sensitivity */
	FT5X0X_REG_THPEAK						= 0x81,
	FT5X0X_REG_THCAL						= 0x82,
	FT5X0X_REG_THWATER					= 0x83,
	FT5X0X_REG_THTEMP					= 0x84,
	FT5X0X_REG_THDIFF						= 0x85,				
	FT5X0X_REG_CTRL						= 0x86,
	FT5X0X_REG_TIMEENTERMONITOR			= 0x87,
	FT5X0X_REG_PERIODACTIVE				= 0x88,      /* report rate */
	FT5X0X_REG_PERIODMONITOR			= 0x89,
	FT5X0X_REG_HEIGHT_B					= 0x8a,
	FT5X0X_REG_MAX_FRAME					= 0x8b,
	FT5X0X_REG_DIST_MOVE					= 0x8c,
	FT5X0X_REG_DIST_POINT				= 0x8d,
	FT5X0X_REG_FEG_FRAME					= 0x8e,
	FT5X0X_REG_SINGLE_CLICK_OFFSET		= 0x8f,
	FT5X0X_REG_DOUBLE_CLICK_TIME_MIN	= 0x90,
	FT5X0X_REG_SINGLE_CLICK_TIME			= 0x91,
	FT5X0X_REG_LEFT_RIGHT_OFFSET		= 0x92,
	FT5X0X_REG_UP_DOWN_OFFSET			= 0x93,
	FT5X0X_REG_DISTANCE_LEFT_RIGHT		= 0x94,
	FT5X0X_REG_DISTANCE_UP_DOWN		= 0x95,
	FT5X0X_REG_ZOOM_DIS_SQR				= 0x96,
	FT5X0X_REG_RADIAN_VALUE				=0x97,
	FT5X0X_REG_MAX_X_HIGH                       	= 0x98,
	FT5X0X_REG_MAX_X_LOW             			= 0x99,
	FT5X0X_REG_MAX_Y_HIGH            			= 0x9a,
	FT5X0X_REG_MAX_Y_LOW             			= 0x9b,
	FT5X0X_REG_K_X_HIGH            			= 0x9c,
	FT5X0X_REG_K_X_LOW             			= 0x9d,
	FT5X0X_REG_K_Y_HIGH            			= 0x9e,
	FT5X0X_REG_K_Y_LOW             			= 0x9f,
	FT5X0X_REG_AUTO_CLB_MODE			= 0xa0,
	FT5X0X_REG_LIB_VERSION_H 				= 0xa1,
	FT5X0X_REG_LIB_VERSION_L 				= 0xa2,		
	FT5X0X_REG_CIPHER						= 0xa3,
	FT5X0X_REG_MODE						= 0xa4,
	FT5X0X_REG_PMODE						= 0xa5,	  /* Power Consume Mode		*/	
	FT5X0X_REG_FIRMID						= 0xa6,   /* Firmware version */
	FT5X0X_REG_STATE						= 0xa7,
	FT5X0X_REG_FT5201ID					= 0xa8,
	FT5X0X_REG_ERR						= 0xa9,
	FT5X0X_REG_CLB						= 0xaa,
};


#define FT5X0X_REG_FIRMID 0xa6






enum{
	IC_FT5316	= 0x0A,
	IC_FT5306I 	= 0x55,
	IC_TYPE_MAX = 0xFF,
};

#define FT_UPGRADE_AA	0xAA
#define FT_UPGRADE_55 	0x55


/*upgrade config of FT5606*/
#define FT5606_UPGRADE_AA_DELAY 		50
#define FT5606_UPGRADE_55_DELAY 		10
#define FT5606_UPGRADE_ID_1				0x79
#define FT5606_UPGRADE_ID_2				0x06
#define FT5606_UPGRADE_READID_DELAY 	100
#define FT5606_UPGRADE_EARSE_DELAY		2000


/*upgrade config of FT5316*/
#define FT5316_UPGRADE_AA_DELAY 		50
#define FT5316_UPGRADE_55_DELAY 		30
#define FT5316_UPGRADE_ID_1				0x79
#define FT5316_UPGRADE_ID_2				0x07
#define FT5316_UPGRADE_READID_DELAY 	1
#define FT5316_UPGRADE_EARSE_DELAY		1500


/*upgrade config of FT5x06(x=2,3,4)*/
#define FT5X06_UPGRADE_AA_DELAY 		50
#define FT5X06_UPGRADE_55_DELAY 		30
#define FT5X06_UPGRADE_ID_1				0x79
#define FT5X06_UPGRADE_ID_2				0x03
#define FT5X06_UPGRADE_READID_DELAY 	1
#define FT5X06_UPGRADE_EARSE_DELAY		2000

/*upgrade config of FT6208*/
#define FT6208_UPGRADE_AA_DELAY 		60
#define FT6208_UPGRADE_55_DELAY 		10
#define FT6208_UPGRADE_ID_1				0x79
#define FT6208_UPGRADE_ID_2				0x05
#define FT6208_UPGRADE_READID_DELAY 	10
#define FT6208_UPGRADE_EARSE_DELAY		2000


#define FTS_PACKET_LENGTH        128
#define FTS_SETTING_BUF_LEN        128

#define FTS_UPGRADE_LOOP	3


#define FTS_FACTORYMODE_VALUE	0x40
#define FTS_WORKMODE_VALUE		0x00

#define FT5X0X_DETECTION_PIN_NUM		9

struct Upgrade_Info {
	u16 delay_aa;		/*delay of write FT_UPGRADE_AA */
	u16 delay_55;		/*delay of write FT_UPGRADE_55 */
	u8 upgrade_id_1;	/*upgrade id 1 */
	u8 upgrade_id_2;	/*upgrade id 2 */
	u16 delay_readid;	/*delay of read id */
	u16 delay_earse_flash; /*delay of earse flash*/
};
#endif /*!FT5X0X_CREATE_APK_DEBUG_CHANNEL*/

#endif

