/* linux/arch/arm/plat-s3c24xx/common-smdk.c
 *
 * Copyright (c) 2006 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *
 * Common code for SMDK2410 and SMDK2440 boards
 *
 * http://www.fluff.org/ben/smdk2440/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/device.h>
#include <linux/platform_device.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>
#include <linux/io.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <asm/mach-types.h>
#include <mach/hardware.h>
#include <asm/irq.h>

#include <mach/regs-gpio.h>
#include <mach/gpio-samsung.h>
#include <linux/platform_data/leds-s3c24xx.h>
#include <linux/platform_data/mtd-nand-s3c2410.h>

#include <plat/gpio-cfg.h>
#include <plat/devs.h>
#include <plat/pm.h>

#include "common-smdk.h"
#include <linux/dm9000.h>

/* LED devices */

static struct s3c24xx_led_platdata smdk_pdata_led4 = {
	.gpio		= S3C2410_GPF(4),
	.flags		= S3C24XX_LEDF_ACTLOW | S3C24XX_LEDF_TRISTATE,
	.name		= "led4",
	.def_trigger	= "timer",
};

static struct s3c24xx_led_platdata smdk_pdata_led5 = {
	.gpio		= S3C2410_GPF(5),
	.flags		= S3C24XX_LEDF_ACTLOW | S3C24XX_LEDF_TRISTATE,
	.name		= "led5",
	.def_trigger	= "nand-disk",
};

static struct s3c24xx_led_platdata smdk_pdata_led6 = {
	.gpio		= S3C2410_GPF(6),
	.flags		= S3C24XX_LEDF_ACTLOW | S3C24XX_LEDF_TRISTATE,
	.name		= "led6",
};

static struct s3c24xx_led_platdata smdk_pdata_led7 = {
	.gpio		= S3C2410_GPF(7),
	.flags		= S3C24XX_LEDF_ACTLOW | S3C24XX_LEDF_TRISTATE,
	.name		= "led7",
};

static struct platform_device smdk_led4 = {
	.name		= "s3c24xx_led",
	.id		= 0,
	.dev		= {
		.platform_data = &smdk_pdata_led4,
	},
};

static struct platform_device smdk_led5 = {
	.name		= "s3c24xx_led",
	.id		= 1,
	.dev		= {
		.platform_data = &smdk_pdata_led5,
	},
};

static struct platform_device smdk_led6 = {
	.name		= "s3c24xx_led",
	.id		= 2,
	.dev		= {
		.platform_data = &smdk_pdata_led6,
	},
};

static struct platform_device smdk_led7 = {
	.name		= "s3c24xx_led",
	.id		= 3,
	.dev		= {
		.platform_data = &smdk_pdata_led7,
	},
};

/* NAND parititon from 2.4.18-swl5 */

static struct mtd_partition smdk_default_nand_part[] = {
	[0] = {
		.name	= "deyou_uboot",
		.size	= SZ_4M,	//4M
		.offset	= 0,
	},
	[1] = {
		.name	= "deyou_kernel",
		.offset = SZ_4M,	
		.size	= 0x00500000,	//5M
	},
	[2] = {
		.name	= "none",
		.offset = 0x00900000,
		.size	= MTDPART_SIZ_FULL,
	}
};

static struct s3c2410_nand_set smdk_nand_sets[] = {
	[0] = {
		.name		= "NAND",
		.nr_chips	= 1,
		.nr_partitions	= ARRAY_SIZE(smdk_default_nand_part),
		.partitions	= smdk_default_nand_part,
	},
};

/* choose a set of timings which should suit most 512Mbit
 * chips and beyond.
*/

static struct s3c2410_platform_nand smdk_nand_info = {
	.tacls		= 20,
	.twrph0		= 60,
	.twrph1		= 20,
	.nr_sets	= ARRAY_SIZE(smdk_nand_sets),
	.sets		= smdk_nand_sets,
};

/*DM9000
 *deyou@2017.08.20
*/
static struct resource s3c_dm9000e_resource[] = {
	[0] = {
		.start = S3C2410_CS4,	//ADDR2 = 0,发送地址时使用这个地址。
		.end = S3C2410_CS4 + 3,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = S3C2410_CS4 + 4,	//ADDR2 =1 ,发送地址时使用这个地址
		.end = S3C2410_CS4 + 4 + 3,
		.flags = IORESOURCE_MEM,
	},
	[2] = {
		.start = IRQ_EINT7,
		.end = IRQ_EINT7,
		.flags = IORESOURCE_IRQ|IRQF_TRIGGER_RISING,
	}
};

/* for the moment we limit ourselves to 16 bit IO until some better
 * IO routines can be written and tested
 */
static struct dm9000_plat_data s3c_dm9000e_platdata = {
	.flags = DM9000_PLATF_16BITONLY,
};

static struct platform_device s3c_device_dm9000e = {
	.name = "dm9000",
	.id = -1,
	.num_resources = ARRAY_SIZE(s3c_dm9000e_resource),
	.resource = s3c_dm9000e_resource,
	.dev = {
		.platform_data = &s3c_dm9000e_platdata,
	}

};
/* dm9000e deyou */

/* devices we initialise */

static struct platform_device __initdata *smdk_devs[] = {
	&s3c_device_nand,
	&smdk_led4,
	&smdk_led5,
	&smdk_led6,
	&smdk_led7,
	&s3c_device_dm9000e,
};

static const struct gpio smdk_led_gpios[] = {
	{ S3C2410_GPF(4), GPIOF_OUT_INIT_HIGH, NULL },
	{ S3C2410_GPF(5), GPIOF_OUT_INIT_HIGH, NULL },
	{ S3C2410_GPF(6), GPIOF_OUT_INIT_HIGH, NULL },
	{ S3C2410_GPF(7), GPIOF_OUT_INIT_HIGH, NULL },
};

void __init smdk_machine_init(void)
{
	/* Configure the LEDs (even if we have no LED support)*/

	int ret = gpio_request_array(smdk_led_gpios,
				     ARRAY_SIZE(smdk_led_gpios));
	if (!WARN_ON(ret < 0))
		gpio_free_array(smdk_led_gpios, ARRAY_SIZE(smdk_led_gpios));

	if (machine_is_smdk2443())
		smdk_nand_info.twrph0 = 50;

	s3c_nand_set_platdata(&smdk_nand_info);

	platform_add_devices(smdk_devs, ARRAY_SIZE(smdk_devs));

	s3c_pm_init();
}
