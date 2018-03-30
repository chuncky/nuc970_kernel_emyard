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

#ifndef __NUC970FB_H
#define __NUC970FB_H
#include <linux/fb.h>
#include <mach/map.h>
#include <linux/platform_data/video-nuc970fb.h>

#define PALETTE_BUFFER_SIZE	256
#define PALETTE_BUFF_CLEAR 	(0x80000000) /* entry is clear/invalid */

struct nuc970fb_info {
	struct platform_device  *pdev;
	struct device		*dev;
	struct clk			*clk;
	struct fb_info		*info;
	bool			enabled;
	struct resource		*mem;
	void __iomem		*io;
	void __iomem		*irq_base;
	int 				drv_type;
	unsigned long		clk_rate;
	struct completion   completion;
#ifdef CONFIG_PM    
    int                 powerdown;
#endif
#ifdef CONFIG_CPU_FREQ
	struct notifier_block	freq_transition;
#endif

	struct nuc970fb_hw			regs;
	struct nuc970fb_mach_info 	*mach_info;
    
	/* keep these registers in case we need to re-write palette */
	u32			palette_buffer[PALETTE_BUFFER_SIZE];
	u32			pseudo_pal[16];
	void (*lcd_power)(int);
	void (*backlight_power)(int);
	
};

int nuc970fb_init(void);


#define VIDEO_DISPLAY_ON								_IOW('v', 24, unsigned int)	//display on
#define VIDEO_DISPLAY_OFF								_IOW('v', 25, unsigned int)	//display off
#define IOCTLCLEARSCREEN						    _IOW('v', 26, unsigned int)	//clear screen










#endif /* __NUC970FB_H */
