/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#define LOG_TAG "LCM"

#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/kernel.h>
#endif

#ifdef CONFIG_MTK_ROUND_CORNER_SUPPORT
#include "data_hw_roundedpattern.h"
#endif

#include "lcm_drv.h"

#ifdef BUILD_LK
#include <platform/upmu_common.h>
#include <platform/mt_gpio.h>
#include <platform/mt_i2c.h>
#include <platform/mt_pmic.h>
#include <string.h>
#elif defined(BUILD_UBOOT)
#include <asm/arch/mt_gpio.h>
#endif

#ifdef BUILD_LK
#define LCM_LOGI(string, args...)  dprintf(0, "[LK/"LOG_TAG"]"string, ##args)
#define LCM_LOGD(string, args...)  dprintf(1, "[LK/"LOG_TAG"]"string, ##args)
#else
#define LCM_LOGI(fmt, args...)  pr_debug("[KERNEL/"LOG_TAG"]"fmt, ##args)
#define LCM_LOGD(fmt, args...)  pr_debug("[KERNEL/"LOG_TAG"]"fmt, ##args)
#endif

static struct LCM_UTIL_FUNCS lcm_util;

#define SET_RESET_PIN(v)	(lcm_util.set_reset_pin((v)))
#define MDELAY(n)		(lcm_util.mdelay(n))
#define UDELAY(n)		(lcm_util.udelay(n))

#define dsi_set_cmdq_V22(cmdq, cmd, count, ppara, force_update) \
		lcm_util.dsi_set_cmdq_V22(cmdq, cmd, count, ppara, force_update)
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update) \
		lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update) \
		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd) lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums) \
		lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)	lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size) \
		lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

#ifndef BUILD_LK
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#endif

#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "lcm_i2c.h"

#define FRAME_WIDTH			(1080)
#define FRAME_HEIGHT			(2520)

/* physical size in um */
#define LCM_PHYSICAL_WIDTH		(64500)
#define LCM_PHYSICAL_HEIGHT		(129000)
#define LCM_DENSITY			(480)

#define REGFLAG_DELAY			0xFFFC
#define REGFLAG_UDELAY			0xFFFB
#define REGFLAG_END_OF_TABLE		0xFFFD
#define REGFLAG_RESET_LOW		0xFFFE
#define REGFLAG_RESET_HIGH		0xFFFF

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#include "disp_dts_gpio.h"

/* i2c control start */

#define LCM_I2C_ADDR 0x3E
#define LCM_I2C_BUSNUM  1	/* for I2C channel 0 */
#define LCM_I2C_ID_NAME "I2C_LCD_BIAS"


/*****************************************************************************
 * Function Prototype
 *****************************************************************************/
static int _lcm_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id);
static int _lcm_i2c_remove(struct i2c_client *client);


/*****************************************************************************
 * Data Structure
 *****************************************************************************/
struct _lcm_i2c_dev {
	struct i2c_client *client;

};

static const struct of_device_id _lcm_i2c_of_match[] = {
	{.compatible = "mediatek,I2C_LCD_BIAS",},
	{},
};

static const struct i2c_device_id _lcm_i2c_id[] = {
	{LCM_I2C_ID_NAME, 0},
	{}
};

static struct i2c_driver _lcm_i2c_driver = {
	.id_table = _lcm_i2c_id,
	.probe = _lcm_i2c_probe,
	.remove = _lcm_i2c_remove,
	/* .detect               = _lcm_i2c_detect, */
	.driver = {
		   .owner = THIS_MODULE,
		   .name = LCM_I2C_ID_NAME,
		   .of_match_table = _lcm_i2c_of_match,
		   },

};

/*****************************************************************************
 * Function
 *****************************************************************************/
static int _lcm_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	pr_debug("[LCM][I2C] %s\n", __func__);
	pr_debug(
		"[LCM][I2C] NT: info==>name=%s addr=0x%x\n",
		client->name, client->addr);
	_lcm_i2c_client = client;
	return 0;
}


static int _lcm_i2c_remove(struct i2c_client *client)
{
	pr_debug("[LCM][I2C] %s\n", __func__);
	_lcm_i2c_client = NULL;
	i2c_unregister_device(client);
	return 0;
}


static int _lcm_i2c_write_bytes(unsigned char addr, unsigned char value)
{
	int ret = 0;
	struct i2c_client *client = _lcm_i2c_client;
	char write_data[2] = { 0 };

	if (client == NULL) {
		pr_debug("ERROR!! _lcm_i2c_client is null\n");
		return 0;
	}

	write_data[0] = addr;
	write_data[1] = value;
	ret = i2c_master_send(client, write_data, 2);
	if (ret < 0)
		pr_info("[LCM][ERROR] _lcm_i2c write data fail !!\n");

	return ret;
}

/*
 * module load/unload record keeping
 */
static int __init _lcm_i2c_init(void)
{
	pr_debug("[LCM][I2C] %s\n", __func__);
	i2c_add_driver(&_lcm_i2c_driver);
	pr_debug("[LCM][I2C] %s success\n", __func__);
	return 0;
}

static void __exit _lcm_i2c_exit(void)
{
	pr_debug("[LCM][I2C] %s\n", __func__);
	i2c_del_driver(&_lcm_i2c_driver);
}

module_init(_lcm_i2c_init);
module_exit(_lcm_i2c_exit);
/* i2c control end */

struct LCM_setting_table {
	unsigned int cmd;
	unsigned char count;
	unsigned char para_list[200];
};

static struct LCM_setting_table lcm_suspend_setting[] = {
	{0x28, 0, {} },
	{REGFLAG_DELAY, 50, {} },
	{0x10, 0, {} },
	{REGFLAG_DELAY, 150, {} },
};

static struct LCM_setting_table init_setting_vdo[] = {
	{0xB0, 0x1, {0x00} },
	{0xB6, 0x5, {0x30, 0x6b, 0x00, 0x02, 0x03} },
	{0xB7, 0x4, {0x51, 0x00, 0x00, 0x00} },
	{0xB8, 0x7, {0x57, 0x3d, 0x19, 0xbe, 0x1e, 0x0a, 0x0a} },
	{0xB9, 0x7, {0x6f, 0x3d, 0x28, 0xbe, 0x3c, 0x14, 0x0a} },
	{0xBA, 0x7, {0xb5, 0x33, 0x41, 0xbe, 0x64, 0x23, 0x0a} },
	{0xBB, 0xB, {0x44, 0x26, 0xc3, 0x1f, 0x19, 0x06, 0x03, 0xc0, 0x00,
		     0x00, 0x10} },
	{0xBC, 0xB, {0x32, 0x4c, 0xc3, 0x52, 0x32, 0x1f, 0x03, 0xf2, 0x00,
		     0x00, 0x13} },
	{0xBD, 0xB, {0x24, 0x68, 0xc3, 0xaa, 0x3f, 0x32, 0x03, 0xff, 0x00,
		     0x00, 0x25} },
	{0xBE, 0xC, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		     0x00, 0x00, 0x00} },
	{0xC0, 0xE, {0x00, 0xbf, 0x01, 0x2c, 0x07, 0x09, 0xd8, 0x00, 0x05,
		     0x00, 0x00, 0x08, 0x00, 0x00} },
	{0xC1, 0x29,
		{0xC1, 0x30, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x22,
		0x00, 0x05, 0x20, 0x00, 0x80, 0xfa, 0x40, 0x00, 0x80, 0x0f,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00} },
	{0xC2, 0x81,
		{0x05, 0xf0, 0x5f, 0x01, 0x03, 0x10, 0x04, 0x02, 0x00,
		0x01, 0x20, 0xbf, 0x01, 0x02, 0x09, 0xe1, 0x01, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x10,
		0xbf, 0x04, 0x04, 0x01, 0x01, 0xc1, 0x00, 0x40, 0x04, 0x00,
		0x00, 0x01, 0x09, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x40,
		0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0x00, 0x00, 0x00, 0x03} },
	{0xC3, 0x6C,
		{0x01, 0x20, 0x12, 0x01, 0x00, 0x20, 0x03, 0x04, 0x01,
		 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		 0x00, 0x01, 0x00, 0xaa, 0xaa, 0xaa, 0x00, 0x00, 0x00, 0x00,
		 0x00, 0x00, 0x0d, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00,
		 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
	{0xC4, 0x61,
		{0x00, 0x00, 0x00, 0x00, 0x4f, 0x00, 0x3e, 0x3f, 0x4f,
		0x00, 0x00, 0xc4, 0x06, 0x02, 0x10, 0x10, 0x0e, 0x0e, 0x61,
		0x61, 0x5f, 0x5f, 0x5d, 0x5d, 0x00, 0x00, 0x00, 0x00, 0x4f,
		0x00, 0x3e, 0x3f, 0x4f, 0x00, 0x00, 0xc4, 0x06, 0x02, 0x11,
		0x11, 0x0f, 0x0f, 0x61, 0x61, 0x5f, 0x5f, 0x5d, 0x5d, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xf7, 0xff, 0xe0, 0xf7,
		0xff, 0x10, 0x08, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00, 0xfc,
		0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04,
		0x04, 0x00, 0x00, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
	{0xC5, 0x5, {0x08, 0x00, 0x00, 0x00, 0x00} },
	{0xC6, 0x3E,
		{0x00, 0x0a, 0x08, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
		0x10, 0x01, 0xff, 0x0f, 0x01, 0x1a, 0x32, 0x37, 0x32, 0x00,
		0x00, 0x00, 0x01, 0x05, 0x09, 0x28, 0x28, 0x01, 0x1a, 0x32,
		0x37, 0x32, 0x00, 0x00, 0x00, 0x01, 0x09, 0x00, 0x00, 0x00,
		0x1e, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20,
		0x00, 0x00, 0x00, 0x00, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00,
		0x00, 0x10, 0x00} },

	//GAMMA2.2_20180522
	{0xC7, 0x4C,
		{0x00, 0xF6, 0x01, 0x8D, 0x01, 0xE3, 0x01, 0xE7, 0x01,
		0xE4, 0x01, 0xD6, 0x01, 0xC4, 0x01, 0xA7, 0x01, 0xBC, 0x01,
		0x83, 0x01, 0xC0, 0x01, 0x6C, 0x01, 0xA1, 0x01, 0x4B, 0x01,
		0xA8, 0x01, 0xA1, 0x02, 0x20, 0x02, 0x6C, 0x02, 0x70, 0x00,
		0xF6, 0x01, 0x8D, 0x01, 0xE3, 0x01, 0xE7, 0x01, 0xE4, 0x01,
		0xD6, 0x01, 0xC4, 0x01, 0xA7, 0x01, 0xBC, 0x01, 0x83, 0x01,
		0xC0, 0x01, 0x6C, 0x01, 0xA1, 0x01, 0x4B, 0x01, 0xA8, 0x01,
		0xA1, 0x02, 0x20, 0x02, 0x6C, 0x02, 0x70} },

	{0xCB, 0xE,
		{0xCB, 0xa0, 0x80, 0x70, 0x00, 0x20, 0x00, 0x00, 0x2d, 0x41,
		0x00, 0x00, 0x00, 0x00, 0xff} },
	{0xCE, 0x21,
		{0x5d, 0x40, 0x49, 0x53, 0x59, 0x5e, 0x63, 0x68, 0x6e,
		0x74, 0x7e, 0x8a, 0x98, 0xa8, 0xbb, 0xd0, 0xe7, 0xff, 0x04,
		0x00, 0x04, 0x04, 0x42, 0x00, 0x69, 0x5a, 0x40, 0x40, 0x00,
		0x00, 0x04, 0xfa, 0x00} },
	{0xCF, 0x6, {0x00, 0x00, 0x80, 0xa1, 0x6a, 0x00} },
	{0xD0, 0x12,
		{0xc7, 0x1e, 0x8a, 0x66, 0x09, 0x90, 0x00, 0xcc, 0x0f,
		0x05, 0xc7, 0x14, 0x12, 0xfe, 0x09, 0x09, 0xcc, 0x00} },
	{0xD1, 0x1E,
		{0xdb, 0xdb, 0x1b, 0xb0, 0x07, 0x07, 0x3b, 0x11, 0xf1,
		0x11, 0xf1, 0x05, 0x33, 0x73, 0x07, 0x33, 0x33, 0x70, 0xd3,
		0xd0, 0x06, 0x96, 0x13, 0x93, 0x22, 0x22, 0x22, 0xb3, 0xbb,
		0x00} },
	{0xD2, 0x11,
		{0x00, 0x00, 0x00, 0x02, 0x7f, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
	{0xD3, 0x99,
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0xff, 0xf7, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xf7, 0xff, 0xff,
		0xf7, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xf7,
		0xff, 0xff, 0xf7, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xf7, 0xff,
		0xff, 0xf7, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xf7, 0xff, 0xff,
		0xf7, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xf7,
		0xff, 0xff, 0xf7, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xf7, 0xff,
		0xff, 0xf7, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xf7, 0xff, 0xff,
		0xf7, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xf7,
		0xff, 0xff, 0xf7, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xf7, 0xff,
		0xff, 0xf7, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xf7, 0xff, 0xff,
		0xf7, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xf7,
		0xff, 0xff, 0xf7, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xf7, 0xff,
		0xff, 0xf7, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xf7, 0xff, 0xff,
		0xf7, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xf7, 0xff, 0xff, 0xf7,
		0xff, 0xff, 0xf7, 0xff} },
	{0xD4, 0x17,
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00} },

	//And,VCOM,reference,setting,
	{0xE5, 0x1, {0x03} },
	{0xD5, 0x2, {0x00, 0x26} },	//26

	{0xD7, 0x4A,
		{0x21, 0x10, 0x52, 0x52, 0x00, 0xbf, 0x00, 0x05, 0x00,
		0xb6, 0x04, 0xfd, 0x01, 0x00, 0x03, 0x00, 0x05, 0x05, 0x05,
		0x07, 0x04, 0x05, 0x06, 0x07, 0x00, 0x02, 0x02, 0x08, 0x03,
		0x03, 0x08, 0x04, 0x08, 0x08, 0x0c, 0x0b, 0x0a, 0x0a, 0x0a,
		0x07, 0x08, 0x0a, 0x06, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x02, 0x04,
		0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x06,
		0x06, 0x00, 0x00, 0x00, 0x00} },
	{0xD8, 0x3E,
		{0x00, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00} },
	{0xDD, 0x4, {0x30, 0x06, 0x23, 0x65} },
	{0xDE, 0xA,
		{0x00, 0x00, 0x00, 0x0f, 0xff, 0x00, 0x00, 0x00, 0x00,
		 0x10} },
	{0xE8, 0x4, {0x00, 0x30, 0x63, 0x00} },
	{0xEA, 0x1D,
		{0x01, 0x0e, 0x01, 0x40, 0x0c, 0x00, 0x00, 0x00, 0x09,
		0x00, 0x02, 0xae, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x04, 0xc2, 0x00, 0x11, 0x00, 0xbf, 0x0b, 0xf0, 0x86} },
	{0xEB, 0x7, {0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x11} },
	{0xEC, 0xA,
		{0x04, 0xe0, 0x00, 0x10, 0x30, 0x0c, 0x00, 0x00, 0x02,
		0x3a} },
	{0xED, 0x20,
		{0x01, 0x01, 0x02, 0x02, 0x08, 0x08, 0x09, 0x09, 0x00,
		0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x12, 0x02,
		0x8c, 0x00, 0x00, 0x00, 0x50, 0x00, 0x02, 0x8c, 0x00, 0x00,
		0xa0, 0x10, 0x00} },
	{0xEE, 0x60,
		{0x03, 0x3f, 0xf0, 0x03, 0x00, 0xf0, 0x03, 0x00, 0x00,
		0x00, 0x00, 0xf2, 0x3f, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x10,
		0x02, 0x10, 0x00, 0x08, 0x00, 0x09, 0xd5, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
	{0xEF, 0x66,
		{0x01, 0xf0, 0x56, 0x08, 0xd0, 0x00, 0x00, 0x00, 0x00,
		0x28, 0x28, 0x28, 0x00, 0x00, 0x00, 0x02, 0x02, 0x02, 0x00,
		0x00, 0x00, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x01, 0xf0,
		0x56, 0x08, 0xd0, 0x00, 0x00, 0x00, 0x00, 0x28, 0x28, 0x28,
		0x00, 0x00, 0x00, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x02,
		0x02, 0x02, 0x00, 0x00, 0x00, 0x10, 0x03, 0x10, 0x02, 0x02,
		0x10, 0x07, 0x10, 0x00, 0x0a, 0x06, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x63, 0x00, 0x03, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x03, 0x00, 0x02} },
	{0xF9, 0x5, {0x44, 0x3f, 0x00, 0x8d, 0xbf} },
	{0xB0, 0x1, {0x03} },
	//Unlock,Manufacture,Command,Access,Protect,of,group,M,and,A
	{0xB0, 0x1, {0x00} },
	{0x36, 0x1, {0xF1} },	//inversion
	{0xD6, 0x1, {0x00} },	//OTP/Flash,Load,setting
	{0xB0, 0x1, {0x03} },	//Manufacture,Command,Access,Protect

	//TE ON
	{0x35, 0x1, {0x00} },

	//CABC
	{0x51, 0x1, {0xFF} },	//Write_Display_Brightness
	{0x53, 0x1, {0x0C} },	//Write_CTRL_Display
	{0x55, 0x1, {0x00} },	//Write_CABC

	{0x11, 0, {} },
	{REGFLAG_DELAY, 150, {} },	//Delay 150ms
	{0x29, 0, {} },

	{REGFLAG_DELAY, 20, {} },	//Delay 20ms
};

static struct LCM_setting_table
__maybe_unused lcm_deep_sleep_mode_in_setting[] = {
	{0x28, 1, {0x00} },
	{REGFLAG_DELAY, 50, {} },
	{0x10, 1, {0x00} },
	{REGFLAG_DELAY, 150, {} },
};

static struct LCM_setting_table
	__maybe_unused lcm_sleep_out_setting[] = {
	{0x11, 1, {0x00} },
	{REGFLAG_DELAY, 120, {} },
	{0x29, 1, {0x00} },
	{REGFLAG_DELAY, 50, {} },
};

static struct LCM_setting_table bl_level[] = {
	{0x51, 1, {0xFF} },
	{REGFLAG_END_OF_TABLE, 0x00, {} }
};

static void push_table(void *cmdq,
		struct LCM_setting_table *table,
		unsigned int count, unsigned char force_update)
{
	unsigned int i;
	unsigned int cmd;

	for (i = 0; i < count; i++) {
		cmd = table[i].cmd;
		switch (cmd) {
		case REGFLAG_DELAY:
			if (table[i].count <= 10)
				MDELAY(table[i].count);
			else
				MDELAY(table[i].count);
			break;
		case REGFLAG_UDELAY:
			UDELAY(table[i].count);
			break;
		case REGFLAG_END_OF_TABLE:
			break;
		default:
			dsi_set_cmdq_V22(cmdq, cmd, table[i].count,
					 table[i].para_list, force_update);
			break;
		}
	}
}

static void lcm_set_util_funcs(const struct LCM_UTIL_FUNCS *util)
{
	memcpy(&lcm_util, util, sizeof(struct LCM_UTIL_FUNCS));
}

static void lcm_get_params(struct LCM_PARAMS *params)
{
	memset(params, 0, sizeof(struct LCM_PARAMS));

	params->type = LCM_TYPE_DSI;

	params->width = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;
	params->physical_width = LCM_PHYSICAL_WIDTH / 1000;
	params->physical_height = LCM_PHYSICAL_HEIGHT / 1000;
	params->physical_width_um = LCM_PHYSICAL_WIDTH;
	params->physical_height_um = LCM_PHYSICAL_HEIGHT;
	params->density = LCM_DENSITY;

	params->dsi.mode = SYNC_PULSE_VDO_MODE;
	params->dsi.switch_mode = CMD_MODE;
	lcm_dsi_mode = SYNC_PULSE_VDO_MODE;
	LCM_LOGI("%s: lcm_dsi_mode %d\n", __func__, lcm_dsi_mode);
	params->dsi.switch_mode_enable = 0;

	/* DSI */
	/* Command mode setting */
	params->dsi.LANE_NUM = LCM_FOUR_LANE;
	/* The following defined the fomat for data coming from LCD engine. */
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format = LCM_DSI_FORMAT_RGB888;

	/* Highly depends on LCD driver capability. */
	params->dsi.packet_size = 256;
	/* video mode timing */

	params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.vertical_sync_active = 4;
	params->dsi.vertical_backporch = 60;
	params->dsi.vertical_frontporch = 20;
	params->dsi.vertical_frontporch_for_low_power = 750;
	params->dsi.vertical_active_line = FRAME_HEIGHT;

	params->dsi.horizontal_sync_active = 10;
	params->dsi.horizontal_backporch = 20;
	params->dsi.horizontal_frontporch = 40;
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;
	/* params->dsi.ssc_disable = 1; */
#ifndef CONFIG_FPGA_EARLY_PORTING
	/* this value must be in MTK suggested table */
	params->dsi.PLL_CLOCK = 590;
	params->dsi.PLL_CK_CMD = 480;
#else
	params->dsi.pll_div1 = 0;
	params->dsi.pll_div2 = 0;
	params->dsi.fbk_div = 0x1;
#endif
	params->dsi.CLK_HS_POST = 36;
	params->dsi.clk_lp_per_line_enable = 0;
	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 0;
	params->dsi.lcm_esd_check_table[0].cmd = 0x0a;
	params->dsi.lcm_esd_check_table[0].count = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x1C;

	/* for ARR 2.0 */
	params->max_refresh_rate = 60;
	params->min_refresh_rate = 45;

#ifdef CONFIG_MTK_ROUND_CORNER_SUPPORT
	params->round_corner_en = 1;
	params->corner_pattern_height = ROUND_CORNER_H_TOP;
	params->corner_pattern_height_bot = ROUND_CORNER_H_BOT;
	params->corner_pattern_tp_size = sizeof(top_rc_pattern);
	params->corner_pattern_lt_addr = (void *)top_rc_pattern;
#endif
}

/* turn on gate ic & control voltage to 5.5V */
static void lcm_init_power(void)
{
	if (lcm_util.set_gpio_lcd_enp_bias) {
		lcm_util.set_gpio_lcd_enp_bias(1);

		_lcm_i2c_write_bytes(0x0, 0xf);
		_lcm_i2c_write_bytes(0x1, 0xf);
	} else
		LCM_LOGI("set_gpio_lcd_enp_bias not defined...\n");
}

static void lcm_suspend_power(void)
{
	SET_RESET_PIN(0);
	if (lcm_util.set_gpio_lcd_enp_bias)
		lcm_util.set_gpio_lcd_enp_bias(0);
	else
		LCM_LOGI("set_gpio_lcd_enp_bias not defined...\n");
}

/* turn on gate ic & control voltage to 5.5V */
static void lcm_resume_power(void)
{
	SET_RESET_PIN(0);
	lcm_init_power();
}

static void lcm_init(void)
{
	/* set TP rst high */
	disp_dts_gpio_select_state(DTS_GPIO_STATE_TP_RST_OUT1);

	SET_RESET_PIN(1);
	MDELAY(1);
	SET_RESET_PIN(0);
	MDELAY(10);

	SET_RESET_PIN(1);
	MDELAY(5);

	push_table(NULL, init_setting_vdo, ARRAY_SIZE(init_setting_vdo), 1);
	LCM_LOGI(
		"td4320_fhdp-tps6132-lcm vdo mode:%d\n", lcm_dsi_mode);
}

static void lcm_suspend(void)
{
	push_table(NULL,
		lcm_suspend_setting, ARRAY_SIZE(lcm_suspend_setting), 1);
}

static void lcm_resume(void)
{
	lcm_init();
}

static unsigned int lcm_ata_check(unsigned char *buffer)
{
#ifndef BUILD_LK
	unsigned int ret = 0;
	unsigned int id[3] = { 0x83, 0x11, 0x2B };
	unsigned int data_array[3];
	unsigned char read_buf[3];

	data_array[0] = 0x00033700;	/* set max return size = 3 */
	dsi_set_cmdq(data_array, 1, 1);

	read_reg_v2(0x04, read_buf, 3);	/* read lcm id */

	LCM_LOGI(
		"ATA read = 0x%x, 0x%x, 0x%x\n",
		read_buf[0], read_buf[1], read_buf[2]);

	if ((read_buf[0] == id[0]) &&
		(read_buf[1] == id[1]) &&
		(read_buf[2] == id[2]))
		ret = 1;
	else
		ret = 0;

	return ret;
#else
	return 0;
#endif
}

static void lcm_setbacklight_cmdq(void *handle, unsigned int level)
{
	LCM_LOGI("%s,td4320 backlight: level = %d\n", __func__, level);

	bl_level[0].para_list[0] = level;

	push_table(handle, bl_level, ARRAY_SIZE(bl_level), 1);
}

static void lcm_update(unsigned int x,
	unsigned int y, unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0 >> 8) & 0xFF);
	unsigned char x0_LSB = (x0 & 0xFF);
	unsigned char x1_MSB = ((x1 >> 8) & 0xFF);
	unsigned char x1_LSB = (x1 & 0xFF);
	unsigned char y0_MSB = ((y0 >> 8) & 0xFF);
	unsigned char y0_LSB = (y0 & 0xFF);
	unsigned char y1_MSB = ((y1 >> 8) & 0xFF);
	unsigned char y1_LSB = (y1 & 0xFF);

	unsigned int data_array[16];

#ifdef LCM_SET_DISPLAY_ON_DELAY
	lcm_set_display_on();
#endif

	data_array[0] = 0x00053902;
	data_array[1] = (x1_MSB << 24) | (x0_LSB << 16) | (x0_MSB << 8) | 0x2a;
	data_array[2] = (x1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0] = 0x00053902;
	data_array[1] = (y1_MSB << 24) | (y0_LSB << 16) | (y0_MSB << 8) | 0x2b;
	data_array[2] = (y1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0] = 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);
}

struct LCM_DRIVER td4320_fhdp_dsi_vdo_auo_rt4801_lcm_drv = {
	.name = "td4320_fhdp_dsi_vdo_auo_rt4801_drv",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params = lcm_get_params,
	.init = lcm_init,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
	.init_power = lcm_init_power,
	.resume_power = lcm_resume_power,
	.suspend_power = lcm_suspend_power,
	.set_backlight_cmdq = lcm_setbacklight_cmdq,
	.ata_check = lcm_ata_check,
	.update = lcm_update,
};
