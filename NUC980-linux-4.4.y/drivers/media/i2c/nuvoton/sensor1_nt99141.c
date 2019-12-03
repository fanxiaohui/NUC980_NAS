/*
 * Copyright (c) 2018 Nuvoton Technology Corporation.
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation;version 2 of the License.
 *
 */

#include <linux/delay.h>
#include <linux/module.h>
#include "nuc980_cap.h"

#define DISALBE_READ_ID

static struct nuvoton_vin_sensor cap1_nt99141;

struct OV_RegValue {
	__u16   uRegAddr;
	__u8    uValue;
};

#define _REG_TABLE_SIZE(nTableName) sizeof(nTableName)/sizeof(struct OV_RegValue)
/* NT99141, VGA, YUV422 */
static struct OV_RegValue Init_RegValue[] = {
	//{0x3021, 0x60},
#if 1
	/* [Inti] */
	{0x3109, 0x04},{0x3040, 0x04},{0x3041, 0x02},{0x3042, 0xFF},{0x3043, 0x08},
	{0x3052, 0xE0},{0x305F, 0x33},{0x3100, 0x07},{0x3106, 0x03},
	{0x3108, 0x00},{0x3110, 0x22},{0x3111, 0x57},{0x3112, 0x22},{0x3113, 0x55},
	{0x3114, 0x05},{0x3135, 0x00},{0x32F0, 0x01},{0x306a,0x01},
	// Initial AWB Gain */
	{0x3290, 0x01},{0x3291, 0x80},{0x3296, 0x01},{0x3297, 0x73},
	/* CA Ratio */
	{0x3250, 0x80},{0x3251, 0x03},{0x3252, 0xFF},{0x3253, 0x00},{0x3254, 0x03},
	{0x3255, 0xFF},{0x3256, 0x00},{0x3257, 0x50},
	/* Gamma */
	{0x3270, 0x00},{0x3271, 0x0C},{0x3272, 0x18},{0x3273, 0x32},{0x3274, 0x44},
	{0x3275, 0x54},{0x3276, 0x70},{0x3277, 0x88},{0x3278, 0x9D},{0x3279, 0xB0},
	{0x327A, 0xCF},{0x327B, 0xE2},{0x327C, 0xEF},{0x327D, 0xF7},{0x327E, 0xFF},
	/* Color Correction */
	{0x3302, 0x00},{0x3303, 0x40},{0x3304, 0x00},{0x3305, 0x96},{0x3306, 0x00},
	{0x3307, 0x29},{0x3308, 0x07},{0x3309, 0xBA},{0x330A, 0x06},{0x330B, 0xF5},
	{0x330C, 0x01},{0x330D, 0x51},{0x330E, 0x01},{0x330F, 0x30},{0x3310, 0x07},
	{0x3311, 0x16},{0x3312, 0x07},{0x3313, 0xBA},
	/* EExt */
	{0x3326, 0x02},{0x32F6, 0x0F},{0x32F9, 0x42},{0x32FA, 0x24},{0x3325, 0x4A},
	{0x3330, 0x00},{0x3331, 0x0A},{0x3332, 0xFF},{0x3338, 0x30},{0x3339, 0x84},
	{0x333A, 0x48},{0x333F, 0x07},
	/* Auto Function */
	{0x3360, 0x10},{0x3361, 0x18},{0x3362, 0x1f},{0x3363, 0x37},{0x3364, 0x80},
	{0x3365, 0x80},{0x3366, 0x68},{0x3367, 0x60},{0x3368, 0x30},{0x3369, 0x28},
	{0x336A, 0x20},{0x336B, 0x10},{0x336C, 0x00},{0x336D, 0x20},{0x336E, 0x1C},
	{0x336F, 0x18},{0x3370, 0x10},{0x3371, 0x38},{0x3372, 0x3C},{0x3373, 0x3F},
	{0x3374, 0x3F},{0x338A, 0x34},{0x338B, 0x7F},{0x338C, 0x10},{0x338D, 0x23},
	{0x338E, 0x7F},{0x338F, 0x14},{0x3375, 0x0A},{0x3376, 0x0C},{0x3377, 0x10},
	{0x3378, 0x14},
	{0x3012, 0x02},{0x3013, 0xD0},{0x3060, 0x01},
#endif
	/* [YUYV_640x480_25_Fps]---MCLK:12M hz PCLK:24M hz */
	{0x32BF, 0x60},{0x32C0, 0x60},{0x32C1, 0x60},{0x32C2, 0x60},{0x32C3, 0x00},
	{0x32C4, 0x20},{0x32C5, 0x20},{0x32C6, 0x20},{0x32C7, 0x00},{0x32C8, 0xB3},
	{0x32C9, 0x60},{0x32CA, 0x80},{0x32CB, 0x80},{0x32CC, 0x80},{0x32CD, 0x80},
	{0x32DB, 0x76},{0x32E0, 0x02},{0x32E1, 0x80},{0x32E2, 0x01},{0x32E3, 0xE0},
	{0x32E4, 0x00},{0x32E5, 0x80},{0x32E6, 0x00},{0x32E7, 0x80},{0x3200, 0x3E},
	{0x3201, 0x0F},{0x3028, 0x07},{0x3029, 0x00},{0x302A, 0x14},{0x3022, 0x25},
	{0x3023, 0x24},{0x3002, 0x00},{0x3003, 0xA4},{0x3004, 0x00},{0x3005, 0x04},
	{0x3006, 0x04},{0x3007, 0x63},{0x3008, 0x02},{0x3009, 0xD3},{0x300A, 0x05},
	{0x300B, 0x3C},{0x300C, 0x02},{0x300D, 0xE1},{0x300E, 0x03},{0x300F, 0xC0},
	{0x3010, 0x02},{0x3011, 0xD0},{0x32B8, 0x3F},{0x32B9, 0x31},{0x32BB, 0x87},
	{0x32BC, 0x38},{0x32BD, 0x3C},{0x32BE, 0x34},{0x3201, 0x7F},{0x3021, 0x06},
	{0x3060, 0x01},//{0x302A, 0x04},{0x3021, 0x00},{0x3021, 0x66},
};

/************  I2C  *****************/
static struct i2c_client *save_client1;
static char sensor1_inited = 0;

#ifndef DISALBE_READ_ID
static int sensor1_read_nt99141(u16 reg,u8 *val)
{
	int ret;
	/* We have 16-bit i2c addresses - care for endianess */
	unsigned char data[2] = { reg >> 8, reg & 0xff };

	ret = i2c_master_send(save_client1, data, 2);
	if (ret < 2) {
		dev_err(&save_client1->dev, "%s: i2c read error, reg: 0x%x\n",
		        __func__, reg);
		return ret < 0 ? ret : -EIO;
	}

	ret = i2c_master_recv(save_client1, val, 1);
	if (ret < 1) {
		dev_err(&save_client1->dev, "%s: i2c read error, reg: 0x%x\n",__func__, reg);
		return ret < 0 ? ret : -EIO;
	}
	return 0;
}
#endif

static int sensor1_write_nt99141(u16 reg, u8 val)
{
	int ret;
	unsigned char data[3] = { reg >> 8, reg & 0xff, val };

	ret = i2c_master_send(save_client1, data, 3);
	if (ret < 3) {
		dev_err(&save_client1->dev, "%s: i2c write error, reg: 0x%x\n",
		        __func__, reg);
		return ret < 0 ? ret : -EIO;
	}

	return 0;
}

static int sensor1_probe(struct i2c_client *client,const struct i2c_device_id *id)
{
	ENTRY();
	if(i2c_adapter_id(client->adapter)!=1)
		return -ENODEV;
	sensor1_inited = 1;
	client->flags = I2C_CLIENT_SCCB;
	save_client1 = client;
	LEAVE();
	return 0;
}
static int sensor1_remove(struct i2c_client *client)
{
	ENTRY();
	LEAVE();
	return 0;
}

static int cap1_nt99141_init(struct nuvoton_vin_device* cam)
{
	int err = 0;
	ENTRY();
	LEAVE();
	return err;
}

static struct nuvoton_vin_sensor cap1_nt99141 = {
	.name = "cap1_nt99141",
	.init = &cap1_nt99141_init,
	.infmtord = (INORD_YUYV | INFMT_YCbCr | INTYPE_CCIR601),
	.polarity = (VSP_LO | HSP_LO | PCLKP_HI),
	.cropstart = ( 0 | 0<<16 ), /*( Vertical | Horizontal<<16 ) */
	.cropcap = {
		.bounds = {
			.left = 0,
			.top = 0,
			.width = 640,
			.height = 480,
		},
		.defrect = {
			.left = 0,
			.top = 0,
			.width = 800,
			.height = 480,
		},
	},
	.pix_format  = {
		.width = 640,
		.height = 480,
		.pixelformat = V4L2_PIX_FMT_YUYV,
		.priv = 16,
		.colorspace = V4L2_COLORSPACE_JPEG,
	},
};

int nuvoton_vin1_probe_nt99141(struct nuvoton_vin_device* cam)
{
	int i,ret = 0;
#ifndef DISALBE_READ_ID
	__u8 SensorID[2];
#endif
	struct OV_RegValue *psRegValue;

	ENTRY();

	nuvoton_vin1_attach_sensor(cam, &cap1_nt99141);


	// if i2c module isn't loaded at this time
	if(!sensor1_inited)
		return -1;

	psRegValue=Init_RegValue;
	for(i=0; i<_REG_TABLE_SIZE(Init_RegValue); i++, psRegValue++) {
		printk(".");
		ret = sensor1_write_nt99141((psRegValue->uRegAddr), (psRegValue->uValue));
		udelay(200);
	}

	//----------Read sensor id-------------------------------------
#ifndef DISALBE_READ_ID
	sensor1_read_nt99141(0x3000,&SensorID[0]);  /* Chip_Version_H 0x14 */
	sensor1_read_nt99141(0x3001,&SensorID[1]);  /* Chip_Version_L 0x10 */
	printk("\nSensor Chip_Version_H = 0x%02x(0x14) Chip_Version_L = 0x%02x(0x10)\n", SensorID[0],SensorID[1]);
#endif
	//-------------------------------------------------------------
	printk("\n");
	if(ret>=0)
		printk("driver i2c initial done\n");
	else
		printk("driver i2c initial fail\n");
	LEAVE();
	return ret;
}

static const struct i2c_device_id sensor1_id[] = {
	{ "cap1_nt99141", 0 },
};
MODULE_DEVICE_TABLE(i2c, sensor1_id);

#ifdef CONFIG_USE_OF
static struct of_device_id cap1_nt99141_match_table[] = {
	{ .compatible = "nuvoton,cap1-nt99141",},
	{},
};
#else
#define cap1_nt99141_match_table NULL
#endif

static struct i2c_driver sensor1_i2c_driver = {
	.driver = {
		.name = "cap1_nt99141",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(cap1_nt99141_match_table),
	},
	.probe    = sensor1_probe,
	.remove   = sensor1_remove,
	.id_table = sensor1_id,
};
module_i2c_driver(sensor1_i2c_driver);
