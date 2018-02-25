/*
 *
 * Copyright (c) 2009 Nuvoton technology corporation
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 *   Author:
 *        Wang Qiang(rurality.linux@gmail.com)  2009/12/16
 */

#ifndef __VIDEO_SWITCH_H
#define __VIDEO_SWITCH_H

#include <mach/map.h>

#define SWITCH_IOCTL_MAGIC		'S'

#define VIDEO_SWITCH_GETDEVICE          _IOWR(SWITCH_IOCTL_MAGIC, 20, unsigned int)	// get/set device id
#define VIDEO_SWITCH_SETONECHANNEL	_IOW(SWITCH_IOCTL_MAGIC, 30, unsigned int)	// Change One Video
#define VIDEO_SWITCH_GETONECHANNEL	_IOR(SWITCH_IOCTL_MAGIC, 31, unsigned int)	//get One Video

#define VIDEO_SWITCH_SETALLCHANNEL	_IOW(SWITCH_IOCTL_MAGIC, 301, unsigned int)	// Change all Video



#define SV_8X8  0x1
#define SV_10X10 0x2
#define SV_9X9 0x3
#define SV_20X20 0x4
#define SV_18X18 0x5
#define SV_16X16 0x6
#define SV_36X36 0x8
#define SV_72X72 0xa
#define SV_144X144 0xc

typedef void (*I2C_Write)(unsigned char i2c_addr,unsigned char Wdata,unsigned int RomAddress);
typedef unsigned char (*I2C_Read)(unsigned char i2c_addr,unsigned char RomAddress);
typedef void (*Up_One_Data)(unsigned char output,unsigned char input);
typedef void (*Up_All_Data)(unsigned char channels, unsigned char iMtxState[]);
typedef void (*DCS_Init)(void);
typedef void (*sw_reset)(void);

typedef struct __SW__INFO
{
	unsigned char output_maxnum;
	unsigned char input_maxnum;
	unsigned char *XPT_MP0;
	unsigned char *Data_IN;
	unsigned char *Data_OUT;
	unsigned char * iMtxState;

	unsigned int rs;
	unsigned int scl;
	unsigned int sda;
	unsigned int update;


	sw_reset SW_Reset;
        DCS_Init DCS_ALL_Init;
	I2C_Write SW_I2C_Write;
	I2C_Read SW_I2C_Read;
	Up_One_Data Up_Config_One_Data;
	Up_All_Data Up_Config_All_Data;


}Switch_Info;


extern Switch_Info sw_info,sw10x10_info;
extern void init_switch_info(Switch_Info* sw_info, sw_reset SW_Reset,DCS_Init DCS_ALL_Init, I2C_Write SW_I2C_Write,I2C_Read SW_I2C_Read);


#endif /* __VIDEO_SWITCH_H */
