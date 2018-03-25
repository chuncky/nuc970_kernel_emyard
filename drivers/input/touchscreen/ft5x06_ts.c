/*
 *
 * FocalTech ft5x06 TouchScreen driver.
 *
 * Copyright (c) 2010  Focal tech Ltd.
 * Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
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


#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>
//#include <linux/wakelock.h>//fgy add for TP_PS
#include <linux/input/ft5x06_ts.h>


#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
/* Early-suspend level */
#define FT5X06_SUSPEND_LEVEL 1
#endif

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




#define CFG_MAX_TOUCH_POINTS	1

#define FT_STARTUP_DLY		250
#define FT_RESET_DLY		20
#define FT_DELAY_DFLT		20
#define FT_NUM_RETRY		10

#define FT_PRESS		0x7F
#define FT_MAX_ID		0x0F
#define FT_TOUCH_STEP		6
#define FT_TOUCH_X_H_POS	3
#define FT_TOUCH_X_L_POS	4
#define FT_TOUCH_Y_H_POS	5
#define FT_TOUCH_Y_L_POS	6
#define FT_TOUCH_EVENT_POS	3
#define FT_TOUCH_ID_POS		5
#define FT_TOUCH_PS_STATUS		0x01
#define FT_TOUCH_PS_ONOFF		0xb0

#define POINT_READ_BUF	(3 + FT_TOUCH_STEP * CFG_MAX_TOUCH_POINTS)

/*register address*/
#define FT5X06_REG_IC_TYPE	0xA3
#define FT5X06_REG_PMODE	0xA5
#define FT5X06_REG_FW_VER	0xA6
#define FT5X06_REG_POINT_RATE	0x88
#define FT5X06_REG_THGROUP	0x80
#define FT5X06_REG_IC_ID	0xA8

/* power register bits*/
#define FT5X06_PMODE_ACTIVE		0x00
#define FT5X06_PMODE_MONITOR		0x01
#define FT5X06_PMODE_STANDBY		0x02
#define FT5X06_PMODE_HIBERNATE		0x03

#define FT5X06_VTG_MIN_UV	2600000
#define FT5X06_VTG_MAX_UV	3300000
#define FT5X06_I2C_VTG_MIN_UV	1800000
#define FT5X06_I2C_VTG_MAX_UV	1800000

#define FTS_POINT_UP		0x01
#define FTS_POINT_DOWN		0x00
#define FTS_POINT_CONTACT	0x02
// tp factory test by lxh
#define FT5X06_FACTORY_TEST
//fgy add for TP_Psensor at E351lq
//#define TP_PROXIMITY_SENSOR
//#define TP_DEBUG
/*fgy add for TP_PS*/
#ifdef TP_PROXIMITY_SENSOR
#define TS_TPPS_NAME "tp_ps"
#endif//TP_PROXIMITY_SENSOR
//#define FIRMWARE_UPGRADE  // developer use only.

 /*-  FW file name for Yeji -*/
#ifdef FIRMWARE_UPGRADE
static unsigned char CTPM_FW_ID_80[]=
{
        #include "yeji_ft6206_pf_e351lq.i"  // vendor-ic-strcuture-proj
};

static unsigned char CTPM_FW_ID_A0[]=
{
        #include "shenyue_ft6206_pf_e351lq.i"  
};
 
 struct focal_fw_st{
        unsigned char* fw;
	unsigned          len;
	unsigned  char  ver;
};

 static struct focal_fw_st  CTPM_FW;
#endif

struct ts_event {
	u16 au16_x[CFG_MAX_TOUCH_POINTS];	/*x coordinate */
	u16 au16_y[CFG_MAX_TOUCH_POINTS];	/*y coordinate */
	u8 au8_touch_event[CFG_MAX_TOUCH_POINTS];	/*touch event:
					0 -- down; 1-- contact; 2 -- contact */
	u8 au8_finger_id[CFG_MAX_TOUCH_POINTS];	/*touch ID */
	u16 pressure;
	u8 touch_point;
};

struct ft5x06_ts_data {
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct ts_event event;
	const struct ft5x06_ts_platform_data *pdata;
	struct regulator *vdd;
	struct regulator *vcc_i2c;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
	struct work_struct ft5x06_wq;
/*fgy add for TP_PS*/
#ifdef TP_PROXIMITY_SENSOR
        struct input_dev *ps_input_dev;  
        int ps_last_value;
        struct device *dev;
#endif//TP_PROXIMITY_SENSOR
};
static struct i2c_client * this_client;
//#define TIMER_READ_OPERATION
#ifdef TIMER_READ_OPERATION
static struct delayed_work timer_read_workqueue;	
#endif

#ifdef TP_PROXIMITY_SENSOR
/*fgy add for ts_ps begin*/
static struct wake_lock psensor_wake_lock;
static int psensor_irq_wake = 0;
static bool rgt_ps_mode=false;
static bool rgt_ps_mode_one = false;
char rgt_ps_data=1;//far away
struct i2c_client *ps_i2c_client = NULL;
struct ft5x06_ts_data *ps_dev;
static struct class *tp_ps_class;
#endif  //TP_PROXIMITY_SENSOR
/*fgy dd fot TP_PS end*/

int touch_ic_flag = 0;
/*fgy add for tp_ps begin*/
#ifdef  TP_PROXIMITY_SENSOR
void tp_ps_enable(int32_t en)
{
    u8 ps_data;
    int err = 0;

    if (en) {
        ps_data = 0x01;
    } else {
        ps_data = 0x00;
    }
    
    err = i2c_smbus_write_i2c_block_data(ps_i2c_client, 0xB0, 1, &ps_data);//en=1 enable || en=0 disable
    if (err==0) {
        if(en) {
            printk("tp_ps: enable TP_PS sucess\n");
            rgt_ps_mode = true;
        }else{
            printk("tp_ps: disable TP_PS sucess\n");
            rgt_ps_mode = false;
        }
    } else{
        printk("tp_ps: enable/disable write I2C fail\n");
    }    

}

/*fgy add tp_ps_report for report tp ps infor to hal*/
inline void tp_ps_report(struct input_dev* dev,int32_t report_value)
{

    ps_dev->ps_last_value = report_value;
    input_report_abs(dev, ABS_DISTANCE, report_value);
    input_sync(dev);

    printk("tp_ps:===tp_ps_report:: %d====\n",report_value);
}

#endif  //TP_PROXIMITY_SENSOR
/*fgy add for tp_ps end*/

static int ft5x06_i2c_read(const struct i2c_client *client, char *writebuf,
			   int writelen, char *readbuf, int readlen)
{
	int ret;

	if (writelen > 0) {
		struct i2c_msg msgs[] = {
			{
				 .addr = client->addr,
				 .flags = 0,
				 .len = writelen,
				 .buf = writebuf,
			 },
			{
				 .addr = client->addr,
				 .flags = I2C_M_RD,
				 .len = readlen,
				 .buf = readbuf,
			 },
		};
		ret = i2c_transfer(client->adapter, msgs, 2);
		if (ret < 0)
			dev_err(&client->dev, "%s: i2c read error.\n",
				__func__);
	} else {
		struct i2c_msg msgs[] = {
			{
				 .addr = client->addr,
				 .flags = I2C_M_RD,
				 .len = readlen,
				 .buf = readbuf,
			 },
		};
		ret = i2c_transfer(client->adapter, msgs, 1);
		if (ret < 0)
			dev_err(&client->dev, "%s:i2c read error.\n", __func__);
	}
	return ret;
}

static int ft5x06_i2c_write(const struct i2c_client *client, char *writebuf,
			    int writelen)
{
	int ret;

	struct i2c_msg msgs[] = {
		{
			 .addr = client->addr,
			 .flags = 0,
			 .len = writelen,
			 .buf = writebuf,
		 },
	};
	ret = i2c_transfer(client->adapter, msgs, 1);
	if (ret < 0)
		dev_err(&client->dev, "%s: i2c write error.\n", __func__);

	return ret;
}


/***********************************************************************************************
Name	:	ft5x0x_read_reg 

Input	:	addr
                     pdata

Output	:	

function	:	read register of ft5x0x

***********************************************************************************************/
static int ft5x0x_read_reg(u8 addr, u8 *pdata)
{
	int ret;
	u8 buf[2];
	struct i2c_msg msgs[2];

    //
	buf[0] = addr;    //register address
	
	msgs[0].addr = this_client->addr;
	msgs[0].flags = 0;
	msgs[0].len = 1;
	msgs[0].buf = buf;
	msgs[1].addr = this_client->addr;
	msgs[1].flags = I2C_M_RD;
	msgs[1].len = 1;
	msgs[1].buf = buf;

	ret = i2c_transfer(this_client->adapter, msgs, 2);
	if (ret < 0)
		pr_err("msg %s i2c read error: %d\n", __func__, ret);

	*pdata = buf[0];
	return ret;
  
}


/***********************************************************************************************
Name	:	 ft5x0x_read_fw_ver

Input	:	 void
                     

Output	:	 firmware version 	

function	:	 read TP firmware version

***********************************************************************************************/
static unsigned char ft5x0x_read_fw_ver(void)
{
	unsigned char ver;
	ft5x0x_read_reg(FT5X0X_REG_FIRMID, &ver);
	return(ver);
}










#ifdef FIRMWARE_UPGRADE
/***********************************************************************************************
Name	:	 

Input	:	
                     

Output	:	

function	:	

***********************************************************************************************/
static int ft5x0x_i2c_txdata(char *txdata, int length)
{
	int ret;

	struct i2c_msg msg[] = {
		{
			.addr	= this_client->addr,
			.flags	= 0,
			.len	= length,
			.buf	= txdata,
		},
	};

   	//msleep(1);
	ret = i2c_transfer(this_client->adapter, msg, 1);
	if (ret < 0)
		pr_err("%s i2c write error: %d\n", __func__, ret);

	return ret;
}


/***********************************************************************************************
Name	:	 ft5x0x_write_reg

Input	:	addr -- address
                     para -- parameter

Output	:	

function	:	write register of ft5x0x

***********************************************************************************************/
static int ft5x0x_write_reg(u8 addr, u8 para)
{
    u8 buf[3];
    int ret = -1;

    buf[0] = addr;
    buf[1] = para;
    printk("guowenbinrt0 ft5x0x_write_reg,buf[0]=%d,buf[1]=%d\n",buf[0],buf[1]);
    ret = ft5x0x_i2c_txdata(buf, 2);
    if (ret < 0) {
        pr_err("write reg failed! %#x ret: %d", buf[0], ret);
        return -1;
    }
    
    return 0;
}

typedef enum
{
    ERR_OK,
    ERR_MODE,
    ERR_READID,
    ERR_ERASE,
    ERR_STATUS,
    ERR_ECC,
    ERR_DL_ERASE_FAIL,
    ERR_DL_PROGRAM_FAIL,
    ERR_DL_VERIFY_FAIL
}E_UPGRADE_ERR_TYPE;

typedef unsigned char         FTS_BYTE;     //8 bit
typedef unsigned short        FTS_WORD;    //16 bit
typedef unsigned int          FTS_DWRD;    //16 bit
typedef unsigned char         FTS_BOOL;    //8 bit

#define FTS_NULL                0x0
#define FTS_TRUE                0x01
#define FTS_FALSE              0x0

#define I2C_CTPM_ADDRESS       0x38//0x70


void delay_qt_ms(unsigned long  w_ms)
{
    unsigned long i;
    unsigned long j;

    for (i = 0; i < w_ms; i++)
    {
        for (j = 0; j < 1000; j++)
        {
            udelay(1);
        }
    }
}


/*
[function]: 
    callback: read data from ctpm by i2c interface,implemented by special user;
[parameters]:
    bt_ctpm_addr[in]    :the address of the ctpm;
    pbt_buf[out]        :data buffer;
    dw_lenth[in]        :the length of the data buffer;
[return]:
    FTS_TRUE     :success;
    FTS_FALSE    :fail;
*/
FTS_BOOL i2c_read_interface(FTS_BYTE bt_ctpm_addr, FTS_BYTE* pbt_buf, FTS_DWRD dw_lenth)
{
    int ret;
    
    ret=i2c_master_recv(this_client, pbt_buf, dw_lenth);

    if(ret<=0)
    {
        printk("[TSP]i2c_read_interface error\n");
        return FTS_FALSE;
    }
  
    return FTS_TRUE;
}

/*
[function]: 
    callback: write data to ctpm by i2c interface,implemented by special user;
[parameters]:
    bt_ctpm_addr[in]    :the address of the ctpm;
    pbt_buf[in]        :data buffer;
    dw_lenth[in]        :the length of the data buffer;
[return]:
    FTS_TRUE     :success;
    FTS_FALSE    :fail;
*/
FTS_BOOL i2c_write_interface(FTS_BYTE bt_ctpm_addr, FTS_BYTE* pbt_buf, FTS_DWRD dw_lenth)
{
    int ret;
    ret=i2c_master_send(this_client, pbt_buf, dw_lenth);
    if(ret<=0)
    {
        printk("[TSP]i2c_write_interface error line = %d, ret = %d\n", __LINE__, ret);
        return FTS_FALSE;
    }

    return FTS_TRUE;
}

/*
[function]: 
    send a command to ctpm.
[parameters]:
    btcmd[in]        :command code;
    btPara1[in]    :parameter 1;    
    btPara2[in]    :parameter 2;    
    btPara3[in]    :parameter 3;    
    num[in]        :the valid input parameter numbers, if only command code needed and no parameters followed,then the num is 1;    
[return]:
    FTS_TRUE    :success;
    FTS_FALSE    :io fail;
*/
FTS_BOOL cmd_write(FTS_BYTE btcmd,FTS_BYTE btPara1,FTS_BYTE btPara2,FTS_BYTE btPara3,FTS_BYTE num)
{
    FTS_BYTE write_cmd[4] = {0};

    write_cmd[0] = btcmd;
    write_cmd[1] = btPara1;
    write_cmd[2] = btPara2;
    write_cmd[3] = btPara3;
    return i2c_write_interface(I2C_CTPM_ADDRESS, write_cmd, num);
}

/*
[function]: 
    write data to ctpm , the destination address is 0.
[parameters]:
    pbt_buf[in]    :point to data buffer;
    bt_len[in]        :the data numbers;    
[return]:
    FTS_TRUE    :success;
    FTS_FALSE    :io fail;
*/
FTS_BOOL byte_write(FTS_BYTE* pbt_buf, FTS_DWRD dw_len)
{
    
    return i2c_write_interface(I2C_CTPM_ADDRESS, pbt_buf, dw_len);
}

/*
[function]: 
    read out data from ctpm,the destination address is 0.
[parameters]:
    pbt_buf[out]    :point to data buffer;
    bt_len[in]        :the data numbers;    
[return]:
    FTS_TRUE    :success;
    FTS_FALSE    :io fail;
*/
FTS_BOOL byte_read(FTS_BYTE* pbt_buf, FTS_BYTE bt_len)
{
    return i2c_read_interface(I2C_CTPM_ADDRESS, pbt_buf, bt_len);
}


/*
[function]: 
    burn the FW to ctpm.
[parameters]:(ref. SPEC)
    pbt_buf[in]    :point to Head+FW ;
    dw_lenth[in]:the length of the FW + 6(the Head length);    
    bt_ecc[in]    :the ECC of the FW
[return]:
    ERR_OK        :no error;
    ERR_MODE    :fail to switch to UPDATE mode;
    ERR_READID    :read id fail;
    ERR_ERASE    :erase chip fail;
    ERR_STATUS    :status error;
    ERR_ECC        :ecc error.
*/


#define    FTS_PACKET_LENGTH        128
#if 0
static unsigned char CTPM_FW_1[]=  // first supplier :Yeji
{
#include "FT6206_BYD_Aimer_Yeji_TF0414A_0x05_app.i"
};

static unsigned char CTPM_FW_2[]=  // first supplier :shenyue
{
#include "FT6206_BYD_Aimer_Yeji_TF0414A_0x05_app.i"
};  
#endif

static E_UPGRADE_ERR_TYPE  fts_ctpm_fw_upgrade(FTS_BYTE* pbt_buf, FTS_DWRD dw_lenth)
{
    FTS_BYTE reg_val[2] = {0};
    FTS_DWRD i = 0;
	  FTS_DWRD k = 0;


    FTS_DWRD  packet_number;
    FTS_DWRD  j;
    FTS_DWRD  temp;
    FTS_DWRD  lenght;
    FTS_BYTE  packet_buf[FTS_PACKET_LENGTH + 6];
    FTS_BYTE  auc_i2c_write_buf[10];
    FTS_BYTE bt_ecc;
    int      i_ret;
for(k = 0;k <10;k ++){
    /*********Step 1:Reset  CTPM *****/
    /*write 0xaa to register 0xfc*/
    ft5x0x_write_reg(0xbc,0xaa);
    delay_qt_ms(100);
     /*write 0x55 to register 0xfc*/
    ft5x0x_write_reg(0xbc,0x55);
    printk("[TSP] Step 1: Reset CTPM test\n");
   
    delay_qt_ms(10*(k + 1));   

//delay_qt_ms(20); 
    /*********Step 2:Enter upgrade mode *****/
    
    auc_i2c_write_buf[0] = 0x55;
    auc_i2c_write_buf[1] = 0xaa;
	i = 0;
    do
    {
        i ++;
        i_ret = ft5x0x_i2c_txdata(auc_i2c_write_buf, 2);
        delay_qt_ms(5);
    }while(i_ret <= 0 && i < 5 );
printk("lxh:check READ-ID\n");
    /*********Step 3:check READ-ID***********************/        
	delay_qt_ms(10);
	cmd_write(0x90,0x00,0x00,0x00,4);
    byte_read(reg_val,2);
    printk("lxh:Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0],reg_val[1]);
    if (reg_val[0] == 0x79 && reg_val[1] == 0x8)
    {
        printk("lxh:[TSP] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0],reg_val[1]);
		break;
    }
    else
    {

     printk("lxh:ERR_READID \n");
        //return ERR_READID;
        //i_is_new_protocol = 1;
    }
}
     /*********Step 4:erase app*******************************/
    cmd_write(0x61,0x00,0x00,0x00,1);
   
    delay_qt_ms(2000);
    printk("lxh:[TSP] Step 4: erase. \n");

    /*********Step 5:write firmware(FW) to ctpm flash*********/
    bt_ecc = 0;
    printk("lxh:[TSP] Step 5: start upgrade. \n");
    dw_lenth = dw_lenth - 8;
    packet_number = (dw_lenth) / FTS_PACKET_LENGTH;
    packet_buf[0] = 0xbf;
    packet_buf[1] = 0x00;
    for (j=0;j<packet_number;j++)
    {
        temp = j * FTS_PACKET_LENGTH;
        packet_buf[2] = (FTS_BYTE)(temp>>8);
        packet_buf[3] = (FTS_BYTE)temp;
        lenght = FTS_PACKET_LENGTH;
        packet_buf[4] = (FTS_BYTE)(lenght>>8);
        packet_buf[5] = (FTS_BYTE)lenght;

        for (i=0;i<FTS_PACKET_LENGTH;i++)
        {
            packet_buf[6+i] = pbt_buf[j*FTS_PACKET_LENGTH + i]; 
            bt_ecc ^= packet_buf[6+i];
        }
        
        byte_write(&packet_buf[0],FTS_PACKET_LENGTH + 6);
        delay_qt_ms(FTS_PACKET_LENGTH/6 + 1);
        if ((j * FTS_PACKET_LENGTH % 1024) == 0)
        {
              printk("[TSP] upgrade the 0x%x th byte.\n", ((unsigned int)j) * FTS_PACKET_LENGTH);
        }
    }

    if ((dw_lenth) % FTS_PACKET_LENGTH > 0)
    {
        temp = packet_number * FTS_PACKET_LENGTH;
        packet_buf[2] = (FTS_BYTE)(temp>>8);
        packet_buf[3] = (FTS_BYTE)temp;

        temp = (dw_lenth) % FTS_PACKET_LENGTH;
        packet_buf[4] = (FTS_BYTE)(temp>>8);
        packet_buf[5] = (FTS_BYTE)temp;

        for (i=0;i<temp;i++)
        {
            packet_buf[6+i] = pbt_buf[ packet_number*FTS_PACKET_LENGTH + i]; 
            bt_ecc ^= packet_buf[6+i];
        }

        byte_write(&packet_buf[0],temp+6);    
        delay_qt_ms(20);
    }

    //send the last six byte
    for (i = 0; i<6; i++)
    {
        temp = 0x6ffa + i;
        packet_buf[2] = (FTS_BYTE)(temp>>8);
        packet_buf[3] = (FTS_BYTE)temp;
        temp =1;
        packet_buf[4] = (FTS_BYTE)(temp>>8);
        packet_buf[5] = (FTS_BYTE)temp;
        packet_buf[6] = pbt_buf[ dw_lenth + i]; 
        bt_ecc ^= packet_buf[6];

        byte_write(&packet_buf[0],7);  
        delay_qt_ms(20);
    }

    /*********Step 6: read out checksum***********************/
    /*send the opration head*/
    cmd_write(0xcc,0x00,0x00,0x00,1);
    byte_read(reg_val,1);
    printk("lxh:[TSP] Step 6:  ecc read 0x%x, new firmware 0x%x. \n", reg_val[0], bt_ecc);
    if(reg_val[0] != bt_ecc)
    {
        return ERR_ECC;
    }

    /*********Step 7: reset the new FW***********************/
    cmd_write(0x07,0x00,0x00,0x00,1);

    return ERR_OK;
}

static int fts_ctpm_fw_upgrade_with_i_file(void)
{
   FTS_BYTE*     pbt_buf = FTS_NULL;
   int i_ret;
    printk(KERN_ERR"%s",__func__);
    //=========FW upgrade========================*/
   pbt_buf = CTPM_FW.fw;
	mdelay(300);
   /*call the upgrade function*/
   i_ret =  fts_ctpm_fw_upgrade(pbt_buf, CTPM_FW.len);
   if (i_ret != 0)
   {
       //error handling ...
       //TBD
   }

   return i_ret;
}

static int focal_ts_read(struct i2c_client *client, u8 reg, u8 *buf, int num)
{
	struct i2c_msg xfer_msg[2];

	xfer_msg[0].addr = client->addr;
	xfer_msg[0].len = 1;
	xfer_msg[0].flags = 0;
	xfer_msg[0].buf = &reg;

	xfer_msg[1].addr = client->addr;
	xfer_msg[1].len = num;
	xfer_msg[1].flags = I2C_M_RD;
	xfer_msg[1].buf = buf;
	return i2c_transfer(client->adapter, xfer_msg, 2);
}  

static void fts_ctpm_get_i_file_ver(unsigned char tp_ID)
{

	if(tp_ID == 0x80)
	{
		CTPM_FW.fw	=	CTPM_FW_ID_80;
		CTPM_FW.len	=	sizeof(CTPM_FW_ID_80);
		CTPM_FW.ver  = CTPM_FW.fw[CTPM_FW.len - 2];
		
	}
	else if(tp_ID == 0xa0)
	{
		CTPM_FW.fw	=	CTPM_FW_ID_A0;
		CTPM_FW.len	=	sizeof(CTPM_FW_ID_A0);
		CTPM_FW.ver  = CTPM_FW.fw[CTPM_FW.len - 2];
	}
	/*else
	{
		CTPM_FW.fw	=	CTPM_FW_ID_94;
		CTPM_FW.len	=	sizeof(CTPM_FW_ID_94);
		CTPM_FW.ver  = CTPM_FW.fw[CTPM_FW.len - 2];
	}   */
	

}

static int fts_ctpm_auto_upg(struct i2c_client *client)
{
    unsigned char uc_tp_fm_ver;
    int           i_ret;
    int ret = 0;
   // unsigned char tp_ID
   u8 reg_id;
   u8 reg_addr;
   int tries;
   int err;

	ret = focal_ts_read(client,0xa6, &uc_tp_fm_ver, 1); //only read two fingers' data
	if (ret<0)
	{
		printk(KERN_ERR "%s: i2c_transfer failed\n", __func__);
	}
	printk(KERN_ERR " lxh:ft5x0x_read_fw_ver   uc_tp_fm_ver=%x\n",uc_tp_fm_ver);
	
	//uc_tp_fm_ver = ft5x0x_read_fw_ver();
	reg_addr = FT5X06_REG_IC_ID;
	tries = FT_NUM_RETRY;
	do {
		err = ft5x06_i2c_read(client, &reg_addr, 1, &reg_id, 1);
		msleep(FT_DELAY_DFLT);
	} while ((err < 0) && (tries--));
    if (err < 0)
    	{     
    	    printk("lxh: read FT5X06_REG_IC_ID  fail\n");
	    return -1;
    	} else {  
         	printk("lxh:read FT5X06_REG_IC_ID  is 0x%x \n", reg_id);
	}
	
    fts_ctpm_get_i_file_ver(reg_id);
    printk(KERN_ERR "lxh:[FTS] upgrade to new version 0x%x -> 0x%x\n",uc_tp_fm_ver, CTPM_FW.ver);
    if  ( uc_tp_fm_ver == 0xa6  ||   //the firmware in touch panel maybe corrupted
         uc_tp_fm_ver < CTPM_FW.ver //the firmware in host flash is new, need upgrade
        )
    {
        msleep(100);
        printk(KERN_ERR "lxha:[FTS] uc_tp_fm_ver = 0x%x, uc_host_fm_ver = 0x%x\n",
            uc_tp_fm_ver, CTPM_FW.ver);
        i_ret = fts_ctpm_fw_upgrade_with_i_file();    
        if (i_ret == 0)
        {
            msleep(300);
            printk(KERN_ERR "lxhb:[FTS] upgrade to new version 0x%x -> 0x%x\n",uc_tp_fm_ver, CTPM_FW.ver);
        }
        else
        {
            printk(KERN_ERR "lxhc:[FTS] upgrade failed ret=%d.\n", i_ret);
        }
    }
   else {
           printk("lxh:** already the latest firmware **\n");
    }	
 
    return 0;
}




#endif

/*Read touch point information when the interrupt  is asserted.*/
static int ft5x06_read_Touchdata(struct ft5x06_ts_data *data)
{
	struct ts_event *event = &data->event;
	u8 buf[POINT_READ_BUF] = { 0 };
	int ret = -1;
	int i = 0;
	u8 pointid = FT_MAX_ID;
	
	ret = ft5x06_i2c_read(data->client, buf, 1, buf, POINT_READ_BUF);
	if (ret < 0) {
		dev_err(&data->client->dev, "%s read touchdata failed.\n",
			__func__);
		return ret;
	}
	memset(event, 0, sizeof(struct ts_event));

	event->touch_point = 0;
	for (i = 0; i < CFG_MAX_TOUCH_POINTS; i++) {
		pointid = (buf[FT_TOUCH_ID_POS + FT_TOUCH_STEP * i]) >> 4;
		if (pointid >= FT_MAX_ID)
			break;
		else
			event->touch_point++;
		event->au16_x[i] =
		    (s16) (buf[FT_TOUCH_X_H_POS + FT_TOUCH_STEP * i] & 0x0F) <<
		    8 | (s16) buf[FT_TOUCH_X_L_POS + FT_TOUCH_STEP * i];
		event->au16_y[i] =
		    (s16) (buf[FT_TOUCH_Y_H_POS + FT_TOUCH_STEP * i] & 0x0F) <<
		    8 | (s16) buf[FT_TOUCH_Y_L_POS + FT_TOUCH_STEP * i];
		event->au8_touch_event[i] =
		    buf[FT_TOUCH_EVENT_POS + FT_TOUCH_STEP * i] >> 6;
		event->au8_finger_id[i] =
		    (buf[FT_TOUCH_ID_POS + FT_TOUCH_STEP * i]) >> 4;
	}

	event->pressure = FT_PRESS;

	return 0;
}

/*
*report the point information
*/
static void ft5x06_report_value(struct ft5x06_ts_data *data)
{
	struct ts_event *event = &data->event;
	int i = 0;
	int up_point = 0;
	//int touch_point = 0;

	for (i = 0; i < event->touch_point; i++) {
		/* LCD view area */

			input_report_abs(data->input_dev, ABS_MT_POSITION_X,
					 event->au16_x[i]);
			input_report_abs(data->input_dev, ABS_MT_POSITION_Y,
					 event->au16_y[i]);
			input_report_abs(data->input_dev, ABS_X, event->au16_x[i]);
			input_report_abs(data->input_dev, ABS_Y, event->au16_y[i]);

			if (event->au8_touch_event[i] == FTS_POINT_DOWN
			    || event->au8_touch_event[i] == FTS_POINT_CONTACT){
				input_report_abs(data->input_dev,
						 ABS_MT_TOUCH_MAJOR,
						 event->pressure);
				input_report_key(data->input_dev, BTN_TOUCH, 1);
				input_report_abs(data->input_dev, ABS_PRESSURE, event->pressure);

			}
			else {
				input_report_abs(data->input_dev,
						 ABS_MT_TOUCH_MAJOR, 0);
				input_report_key(data->input_dev, BTN_TOUCH, 0);
				input_report_abs(data->input_dev, ABS_PRESSURE, 0);
				up_point++;
			}
			//printk(KERN_ERR"p = %d,x = %d,y = %d \n",i,event->au16_x[i],event->au16_y[i]);
			input_mt_sync(data->input_dev);
	}
	input_sync(data->input_dev);
}

static void ft5x06_data_disposal(struct work_struct *work)
{
	struct ft5x06_ts_data *data = container_of(work, struct ft5x06_ts_data, ft5x06_wq);
	
	int ret = 0;
	ret = ft5x06_read_Touchdata(data);
	if (ret == 0)
		ft5x06_report_value(data);

	enable_irq(data->client -> irq);

}
/*The ft5x0x device will signal the host about TRIGGER_FALLING.
*Processed when the interrupt is asserted.
*/
static irqreturn_t ft5x06_ts_interrupt(int irq, void *dev_id)
{

	struct ft5x06_ts_data *ft5x06_ts = dev_id;
	disable_irq_nosync(ft5x06_ts->client -> irq);
	schedule_work(&ft5x06_ts ->ft5x06_wq);

	return IRQ_HANDLED;
}

static int ft5x06_power_on(struct ft5x06_ts_data *data, bool on)
{
	int rc;

	if (!on)
		goto power_off;

	rc = regulator_enable(data->vdd);
	if (rc) {
		dev_err(&data->client->dev,
			"Regulator vdd enable failed rc=%d\n", rc);
		return rc;
	}

	rc = regulator_enable(data->vcc_i2c);
	if (rc) {
		dev_err(&data->client->dev,
			"Regulator vcc_i2c enable failed rc=%d\n", rc);
		regulator_disable(data->vdd);
	}

	return rc;

power_off:
	rc = regulator_disable(data->vdd);
	if (rc) {
		dev_err(&data->client->dev,
			"Regulator vdd disable failed rc=%d\n", rc);
		return rc;
	}

	rc = regulator_disable(data->vcc_i2c);
	if (rc) {
		dev_err(&data->client->dev,
			"Regulator vcc_i2c disable failed rc=%d\n", rc);
		regulator_enable(data->vdd);
	}

	return rc;
}

static int ft5x06_power_init(struct ft5x06_ts_data *data, bool on)
{
	int rc;

	if (!on)
		goto pwr_deinit;

	data->vdd = regulator_get(&data->client->dev, "vdd");
	if (IS_ERR(data->vdd)) {
		rc = PTR_ERR(data->vdd);
		dev_err(&data->client->dev,
			"Regulator get failed vdd rc=%d\n", rc);
		return rc;
	}

	if (regulator_count_voltages(data->vdd) > 0) {
		rc = regulator_set_voltage(data->vdd, FT5X06_VTG_MIN_UV,
					   FT5X06_VTG_MAX_UV);
		if (rc) {
			dev_err(&data->client->dev,
				"Regulator set_vtg failed vdd rc=%d\n", rc);
			goto reg_vdd_put;
		}
	}

	data->vcc_i2c = regulator_get(&data->client->dev, "vcc_i2c");
	if (IS_ERR(data->vcc_i2c)) {
		rc = PTR_ERR(data->vcc_i2c);
		dev_err(&data->client->dev,
			"Regulator get failed vcc_i2c rc=%d\n", rc);
		goto reg_vdd_set_vtg;
	}

	if (regulator_count_voltages(data->vcc_i2c) > 0) {
		rc = regulator_set_voltage(data->vcc_i2c, FT5X06_I2C_VTG_MIN_UV,
					   FT5X06_I2C_VTG_MAX_UV);
		if (rc) {
			dev_err(&data->client->dev,
			"Regulator set_vtg failed vcc_i2c rc=%d\n", rc);
			goto reg_vcc_i2c_put;
		}
	}

	return 0;

reg_vcc_i2c_put:
	regulator_put(data->vcc_i2c);
reg_vdd_set_vtg:
	if (regulator_count_voltages(data->vdd) > 0)
		regulator_set_voltage(data->vdd, 0, FT5X06_VTG_MAX_UV);
reg_vdd_put:
	regulator_put(data->vdd);
	return rc;

pwr_deinit:
	if (regulator_count_voltages(data->vdd) > 0)
		regulator_set_voltage(data->vdd, 0, FT5X06_VTG_MAX_UV);

	regulator_put(data->vdd);

	if (regulator_count_voltages(data->vcc_i2c) > 0)
		regulator_set_voltage(data->vcc_i2c, 0, FT5X06_I2C_VTG_MAX_UV);

	regulator_put(data->vcc_i2c);
	return 0;
}

/*fgy add for TP_PS begin*/
#ifdef TP_PROXIMITY_SENSOR
static ssize_t  tp_pssensor_store(
	struct device *dev, struct device_attribute *attr,
	char const *buf, size_t count)
{
	unsigned long en = 0;
	int error;
        int en_p; 
	
	error = strict_strtoul(buf, 10, &en);
        en_p = en;
	if (error)
		return error;

	tp_ps_enable(en);
        
        printk("tp_ps:tp_pssensor_store: en=%d.\n",en_p);
        
	return count;
}
static ssize_t  tp_pssensor_show(
	struct device *dev, struct device_attribute *attr,
	char *buf)
{	
	int tp_ps_enable = 0;
        if (rgt_ps_mode) {
        	tp_ps_enable=1;
        } else {
        	tp_ps_enable=0;
        }
	return sprintf(buf, "tp_ps_enable status: %d\n", tp_ps_enable);
}
static DEVICE_ATTR(tp_pssensor, 0664, tp_pssensor_show,tp_pssensor_store);

static int create_tp_ps_class(void)
{
	if (!tp_ps_class) {
		tp_ps_class = class_create(THIS_MODULE, "tp_ps_psensor");
		if (IS_ERR(tp_ps_class))
			return PTR_ERR(tp_ps_class);
	}
	return 0;
}

static int tp_ps_dev_register(struct device *parent, struct ft5x06_ts_data *pdata)
{
	int ret = -1;

	ret = create_tp_ps_class();
	
	if (ret ) {
		printk("TP_PS:create class fail\n");
		return ret;
	}
    
	pdata->dev = device_create(tp_ps_class, parent, 0, NULL, "tp_ps");
	
	if(IS_ERR(pdata->dev))
	{
		printk("%s:rohm1772 device_create fail\n", __func__);
	    	return PTR_ERR(pdata->dev);
	}

		ret = device_create_file(pdata->dev, &dev_attr_tp_pssensor);
		
	if(ret)
		return ret;
    
	dev_set_drvdata(pdata->dev, pdata);

       return 0;
}

static void tp_ps_dev_unregister(struct ft5x06_ts_data *pdev)
{
      	device_remove_file(pdev->dev, &dev_attr_tp_pssensor);
      	device_unregister(pdev->dev);
	class_destroy(tp_ps_class);
        kfree(pdev);
}

static int tp_ps_dev_init(void)
{
	struct ft5x06_ts_data *tp_ps_dev;
        int ret =0;

	tp_ps_dev = kzalloc(sizeof(*tp_ps_dev), GFP_KERNEL);
	if (NULL == tp_ps_dev) {
            printk("tp_ps:malloc dev fail");
	}

   	ret = tp_ps_dev_register(tp_ps_dev->dev, tp_ps_dev);
        if (ret) {
             printk("tp_ps:register dev fail");
             tp_ps_dev_unregister(tp_ps_dev);
        }
    
    return ret; 
    
}
#endif //TP_PROXIMITY_SENSOR
/*fgy add for TP_PS end*/
#define FT5X06_RESET_GPIO	26

#ifdef TIMER_READ_OPERATION
static void tp_timer_read_work(struct work_struct *work)
{
   u8 reg_value = 0;
   int ret=0; 
   u8  reg_addr = FT5X06_REG_POINT_RATE;  // 0x88

ret = ft5x06_i2c_read(this_client, &reg_addr, 1, &reg_value, 1);
#ifdef TP_DEBUG
	printk("lxh: ** reg_value is 0x%X **\n", reg_value);
#endif

          if (( ret < 0) ||(reg_value < 5) || (reg_value > 10)) {
	    if (gpio_is_valid(FT5X06_RESET_GPIO)) {
		gpio_set_value_cansleep(FT5X06_RESET_GPIO, 0);
		msleep(FT_RESET_DLY);
		gpio_set_value_cansleep(FT5X06_RESET_GPIO, 1);
 		msleep(FT_STARTUP_DLY);
	}
     }   
   schedule_delayed_work(&timer_read_workqueue, msecs_to_jiffies(1000));
}
#endif  

#ifdef CONFIG_PM

#ifdef CONFIG_BYD//andytest
extern uint32_t get_key_power_state(void);
extern void set_key_power_state(uint32_t val);
#endif

static int ft5x06_ts_suspend(struct device *dev)
{
	struct ft5x06_ts_data *data = dev_get_drvdata(dev);
	char txbuf[2];
    
#ifdef TIMER_READ_OPERATION
      cancel_delayed_work(&timer_read_workqueue);
#endif
/*fgy add for TP_Ps,forbid sleep when enable psensor begin*/
#ifdef TP_PROXIMITY_SENSOR
#ifdef CONFIG_BYD//andytest
	if (1==get_key_power_state())
	{   
		set_key_power_state(0);
	}
	else
#endif
	{
    if (rgt_ps_mode)
    {
	    rgt_ps_mode_one = 1;
	    enable_irq_wake(psensor_irq_wake);
	    return 0;
    }
	}
#endif
/*fgy add for TP_Ps,forbid sleep when enable psensor end*/
	disable_irq(data->client->irq);

	if (gpio_is_valid(data->pdata->reset_gpio)) {
		txbuf[0] = FT5X06_REG_PMODE;
		txbuf[1] = FT5X06_PMODE_HIBERNATE;
		ft5x06_i2c_write(data->client, txbuf, sizeof(txbuf));
	}
       printk("lxh: ft5x06_ts_suspend done\n");  // lxh:test

	return 0;
}

static int ft5x06_ts_resume(struct device *dev)
{
	struct ft5x06_ts_data *data = dev_get_drvdata(dev);
/*fgy add,when enbale ps and return suspend,return resume begin*/
#ifdef TIMER_READ_OPERATION
    schedule_delayed_work(&timer_read_workqueue,msecs_to_jiffies(1000));
#endif

#ifdef TP_PROXIMITY_SENSOR
#ifdef CONFIG_BYD//andytest
if(1==get_key_power_state())
{
	set_key_power_state(0);
}
else
#endif
{
    if (rgt_ps_mode)
   {
	    if(rgt_ps_mode_one)
        {
            rgt_ps_mode_one = 0;
	     disable_irq_wake(psensor_irq_wake);	
            return 0;
        }       
    }
}
#endif
/*fgy add,when enbale ps and return suspend,return resume end*/


	if (gpio_is_valid(data->pdata->reset_gpio)) {
		gpio_set_value_cansleep(data->pdata->reset_gpio, 0);
		msleep(FT_RESET_DLY);
		gpio_set_value_cansleep(data->pdata->reset_gpio, 1);
		msleep(FT_STARTUP_DLY);
	}
	enable_irq(data->client->irq);
        printk("lxh: ft5x06_ts_resume done \n");  // lxh:test

	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void ft5x06_ts_early_suspend(struct early_suspend *handler)
{
	struct ft5x06_ts_data *data = container_of(handler,
						   struct ft5x06_ts_data,
						   early_suspend);

	ft5x06_ts_suspend(&data->client->dev);
}
static void ft5x06_ts_late_resume(struct early_suspend *handler)
{
	struct ft5x06_ts_data *data = container_of(handler,
						   struct ft5x06_ts_data,
						   early_suspend);

	ft5x06_ts_resume(&data->client->dev);
}
#endif

static const struct dev_pm_ops ft5x06_ts_pm_ops = {
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend = ft5x06_ts_suspend,
	.resume = ft5x06_ts_resume,
#endif
};
#endif

#ifdef FT5X06_FACTORY_TEST
static u8 ft5x06_flag[10];
struct class *ft5x06_firmware_class;
struct device *ft5x06_firmware_cmd_dev;

static ssize_t ft5x06_factory_test_show(struct device *dev,
                                  struct device_attribute *attr, char *buf)
{
	size_t count = 0;
	count = sprintf(buf, "%s\n", ft5x06_flag);
	return count;
}
static ssize_t ft5x06_factory_test_store(struct device *dev,
                                   struct device_attribute *attr, const char *buf, size_t size)
{
	memcpy(ft5x06_flag,buf,10);
	return 0;
}
static DEVICE_ATTR(ft5x06_factory_test, 0774, ft5x06_factory_test_show, ft5x06_factory_test_store);

#endif
static int ft5x06_ts_probe(struct i2c_client *client,
			   const struct i2c_device_id *id)
{
	const struct ft5x06_ts_platform_data *pdata = client->dev.platform_data;
	struct ft5x06_ts_data *data;
	struct input_dev *input_dev;
	u8 reg_value;
	u8 reg_addr;
	int err;
	int tries;
	unsigned char uc_reg_value; 

	
	if (!pdata) {
		dev_err(&client->dev, "Invalid pdata\n");
		return -EINVAL;
	}

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "I2C not supported\n");
		return -ENODEV;
	}

	data = kzalloc(sizeof(struct ft5x06_ts_data), GFP_KERNEL);
	if (!data) {
		dev_err(&client->dev, "Not enough memory\n");
		return -ENOMEM;
	}
	this_client = client;

	input_dev = input_allocate_device();
	if (!input_dev) {
		err = -ENOMEM;
		dev_err(&client->dev, "failed to allocate input device\n");
		goto free_mem;
	}

	data->input_dev = input_dev;
	data->client = client;
	data->pdata = pdata;
	input_dev->name = "ft5x06_ts";
	input_dev->id.bustype = BUS_I2C;
	input_dev->dev.parent = &client->dev;

	input_set_drvdata(input_dev, data);
	i2c_set_clientdata(client, data);
	
	set_bit(ABS_MT_TOUCH_MAJOR, input_dev->absbit);
	set_bit(ABS_MT_POSITION_X, input_dev->absbit);
	set_bit(ABS_MT_POSITION_Y, input_dev->absbit);
	
	set_bit(EV_ABS, input_dev->evbit);
	set_bit(EV_SYN, input_dev->evbit);
	set_bit(EV_KEY, input_dev->evbit);

	set_bit(KEY_BACK, input_dev->keybit);
	set_bit(KEY_MENU, input_dev->keybit);
	set_bit(KEY_HOME, input_dev->keybit);
	set_bit(BTN_TOUCH, input_dev->keybit);


	printk("pdata->x_max=%d,pdata->y_max=%d\n",pdata->x_max,pdata->y_max);
	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0,
			     pdata->x_max, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0,
			     pdata->y_max, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, FT_PRESS, 0, 0);


    	input_set_abs_params(input_dev, ABS_X, 0, 
				pdata->x_max, 0, 0);
    	input_set_abs_params(input_dev, ABS_Y, 0, 
				pdata->y_max, 0, 0);
    	input_set_abs_params(input_dev, ABS_PRESSURE, 0, FT_PRESS, 0, 0);


	err = input_register_device(input_dev);
	if (err) {
		dev_err(&client->dev, "Input device registration failed\n");
		goto free_inputdev;
	}

	if (gpio_is_valid(pdata->irq_gpio)) {
		err = gpio_request(pdata->irq_gpio, "ft5x06_irq_gpio");
		if (err) {
			dev_err(&client->dev, "irq gpio request failed");
			goto pwr_off;
		} else {
                        printk("lxh:**irq gpio request ok**\n");
                }
		err = gpio_direction_input(pdata->irq_gpio);
		if (err) {
			dev_err(&client->dev,
				"set_direction for irq gpio failed\n");
			goto free_irq_gpio;
		}   else {
		        printk("lxh:**set_direction for irq ok**\n");
                }
        }

	if (gpio_is_valid(pdata->reset_gpio)) {
		err = gpio_request(pdata->reset_gpio, "ft5x06_reset_gpio");
		if (err) {
			dev_err(&client->dev, "reset gpio request failed");
			goto free_irq_gpio;
		} else {
                        printk("lxh:**reset gpio request ok**\n");
                }

		err = gpio_direction_output(pdata->reset_gpio, 0);
		if (err) {
			dev_err(&client->dev,
				"set_direction for reset gpio failed\n");
			goto free_reset_gpio;
		} else {
                        printk("lxh:**set_direction for reset gpio ok**\n");
                }
		msleep(FT_RESET_DLY);
		gpio_set_value_cansleep(data->pdata->reset_gpio, 1);
	}

	/* make sure CTP already finish startup process */
	msleep(FT_STARTUP_DLY);

	/*get some register information */

    uc_reg_value = ft5x0x_read_fw_ver();
    printk("[FTS] Firmware version = 0x%x\n", uc_reg_value);
    ft5x0x_read_reg(FT5X0X_REG_PERIODACTIVE, &uc_reg_value);
    printk("[FTS] report rate is %dHz.\n", uc_reg_value * 10);
    ft5x0x_read_reg(FT5X0X_REG_THGROUP, &uc_reg_value);
    printk("[FTS] touch threshold is %d.\n", uc_reg_value * 4);

	/* read firmware version */
	reg_addr = FT5X06_REG_IC_TYPE;
	tries = FT_NUM_RETRY;
	do {
		err = ft5x06_i2c_read(client, &reg_addr, 1, &reg_value, 1);
		msleep(FT_DELAY_DFLT);
	} while ((err < 0) && tries--);

	if (err < 0) {
		dev_err(&client->dev, "lxh:ft6306 probe failed_20130902_2! \n");
		goto free_reset_gpio;
	}
	dev_info(&client->dev, "lxh: ft6306 probe ok with chip_id 0x%x_20130902_2\n", reg_value);
	client->irq=gpio_to_irq(pdata->irq_gpio);

	printk("irq_gpio=0x%x,client->irq=%d\n",pdata->irq_gpio,client->irq);

#ifdef FIRMWARE_UPGRADE
     fts_ctpm_auto_upg(client);	
#endif

	/* Requesting irq */
	INIT_WORK(&data -> ft5x06_wq, ft5x06_data_disposal);

	err = request_irq(client->irq, ft5x06_ts_interrupt, pdata->irqflags,
				   client->dev.driver->name, data);

	if (err) {
		dev_err(&client->dev, "request irq failed\n");
		goto free_reset_gpio;
	} else {
                printk("lxh:**request irq ok**\n");   
        }
	

#ifdef TIMER_READ_OPERATION
 	INIT_DELAYED_WORK(&timer_read_workqueue, tp_timer_read_work);	
	schedule_delayed_work(&timer_read_workqueue,msecs_to_jiffies(1000));
#endif

#ifdef CONFIG_HAS_EARLYSUSPEND
	data->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN +
	    FT5X06_SUSPEND_LEVEL;
	data->early_suspend.suspend = ft5x06_ts_early_suspend;
	data->early_suspend.resume = ft5x06_ts_late_resume;
	register_early_suspend(&data->early_suspend);
#endif

#ifdef FT5X06_FACTORY_TEST
    ft5x06_firmware_class = class_create(THIS_MODULE, "ft_touchscreen_ft5x06");
    if (IS_ERR(ft5x06_firmware_class))
        pr_err("Failed to create class(firmware)!\n");
	
     ft5x06_firmware_cmd_dev = device_create(ft5x06_firmware_class, NULL, 0, NULL, "device");
    if (IS_ERR(ft5x06_firmware_cmd_dev))
        pr_err("Failed to create device(firmware_cmd_dev)!\n");

     if (device_create_file(ft5x06_firmware_cmd_dev, &dev_attr_ft5x06_factory_test) < 0)
    {
        pr_err("Failed to create device file(%s)!\n", dev_attr_ft5x06_factory_test.attr.name);
    }
    dev_set_drvdata(ft5x06_firmware_cmd_dev, NULL);

#endif
    touch_ic_flag = 1;
	return 0;

free_reset_gpio:
	if (gpio_is_valid(pdata->reset_gpio))
		gpio_free(pdata->reset_gpio);
free_irq_gpio:
	if (gpio_is_valid(pdata->irq_gpio))
		gpio_free(pdata->irq_gpio);
pwr_off:
	if (pdata->power_on)
		pdata->power_on(false);
	else
		ft5x06_power_on(data, false);
//pwr_deinit:
	if (pdata->power_init)
		pdata->power_init(false);
	else
		ft5x06_power_init(data, false);
//unreg_inputdev:
	input_unregister_device(input_dev);
	input_dev = NULL;
free_inputdev:
	input_free_device(input_dev);
free_mem:
	kfree(data);
	return err;
}

static int  ft5x06_ts_remove(struct i2c_client *client)
{
	struct ft5x06_ts_data *data = i2c_get_clientdata(client);

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&data->early_suspend);
#endif
	free_irq(client->irq, data);

	if (gpio_is_valid(data->pdata->reset_gpio))
		gpio_free(data->pdata->reset_gpio);

	if (gpio_is_valid(data->pdata->irq_gpio))
		gpio_free(data->pdata->reset_gpio);

	if (data->pdata->power_on)
		data->pdata->power_on(false);
	else
		ft5x06_power_on(data, false);

	if (data->pdata->power_init)
		data->pdata->power_init(false);
	else
		ft5x06_power_init(data, false);

	input_unregister_device(data->input_dev);
	kfree(data);

	return 0;
}

static const struct i2c_device_id ft5x06_ts_id[] = {
	{"ft5x06_ts", 0},
	{},
};

MODULE_DEVICE_TABLE(i2c, ft5x06_ts_id);

static struct i2c_driver ft5x06_ts_driver = {
	.probe = ft5x06_ts_probe,
	.remove = ft5x06_ts_remove,
	.driver = {
		   .name = "ft5x06_ts",
		   .owner = THIS_MODULE,
#ifdef CONFIG_PM
		   .pm = &ft5x06_ts_pm_ops,
#endif
		   },
	.id_table = ft5x06_ts_id,
};

static int __init ft5x06_ts_init(void)
{ 
    if (touch_ic_flag ==1) { 
       printk("lxh: **already goodix's touch no need for focaltech**\n");
    } else {
    	return i2c_add_driver(&ft5x06_ts_driver);
    }
    return 0;
}
module_init(ft5x06_ts_init);

static void __exit ft5x06_ts_exit(void)
{
	i2c_del_driver(&ft5x06_ts_driver);
}
module_exit(ft5x06_ts_exit);

MODULE_DESCRIPTION("FocalTech ft5x06 TouchScreen driver");
MODULE_LICENSE("GPL v2");

