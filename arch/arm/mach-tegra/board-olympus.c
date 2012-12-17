/*
 * arch/arm/mach-tegra/board-mot.c
 *
 * Copyright (C) 2010 Google, Inc.
 * Copyright (C) 2010 NVIDIA Corporation
 * Copyright (C) 2010 Motorola, Inc.
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

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/serial_8250.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/pda_power.h>
#include <linux/io.h>
#include <linux/spi/cpcap.h>
#include <linux/spi/spi.h>
#include <linux/i2c.h>
#include <linux/memblock.h>
#include <linux/console.h>

#include <asm/bootinfo.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/time.h>
#include <asm/setup.h>

#include <mach/iomap.h>
#include <mach/irqs.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/gpio.h>
#include <linux/cpcap-accy.h>
#include <linux/reboot.h>

#include "clock.h"
#include "gpio-names.h"

#include <linux/qtouch_obp_ts.h>
#include <linux/isl29030.h>

#include <linux/leds-lm3530.h>
#include <linux/leds-lm3532.h>

#include "board.h"
#include "hwrev.h"

#include "board-olympus.h"

#define PWRUP_FACTORY_CABLE         0x00000020 /* Bit 5  */
#define PWRUP_INVALID               0xFFFFFFFF
#define PWRUP_BAREBOARD             0x00100000 /* Bit 20 */

static char oly_unused_pins_p3[] = {
        TEGRA_GPIO_PO1,
        TEGRA_GPIO_PO2,
        TEGRA_GPIO_PO3,
        TEGRA_GPIO_PO4,
        TEGRA_GPIO_PO5,
        TEGRA_GPIO_PO6,
        TEGRA_GPIO_PO7,
        TEGRA_GPIO_PO0,
        TEGRA_GPIO_PY0,
        TEGRA_GPIO_PY1,
        TEGRA_GPIO_PY2,
        TEGRA_GPIO_PY3,
        TEGRA_GPIO_PC1,        
        TEGRA_GPIO_PN5,
        TEGRA_GPIO_PN6,
        TEGRA_GPIO_PW1,
        TEGRA_GPIO_PB3,
        TEGRA_GPIO_PJ3,
        TEGRA_GPIO_PE5,
        TEGRA_GPIO_PE6,
        TEGRA_GPIO_PE7,
        TEGRA_GPIO_PF0,
        TEGRA_GPIO_PM6,
        TEGRA_GPIO_PM7,
        TEGRA_GPIO_PT4,
        TEGRA_GPIO_PL5,
        TEGRA_GPIO_PL6,
        TEGRA_GPIO_PL7,
        TEGRA_GPIO_PT2,
        TEGRA_GPIO_PD6,
        TEGRA_GPIO_PD7,
        TEGRA_GPIO_PR3,
        TEGRA_GPIO_PR4,
        TEGRA_GPIO_PR5,
        TEGRA_GPIO_PR6,
        TEGRA_GPIO_PR7,
        TEGRA_GPIO_PS0,
        TEGRA_GPIO_PS1,
        TEGRA_GPIO_PS2,
        TEGRA_GPIO_PQ3,
        TEGRA_GPIO_PQ4,
        TEGRA_GPIO_PQ5,
        TEGRA_GPIO_PBB0,
        TEGRA_GPIO_PZ5,
        /* TEGRA_GPIO_PK5, AKM8975_RESET_GPIO */
        TEGRA_GPIO_PK6,
        TEGRA_GPIO_PW5,
        TEGRA_GPIO_PD3,
        TEGRA_GPIO_PI7,
        TEGRA_GPIO_PJ0,
        TEGRA_GPIO_PJ2,
        TEGRA_GPIO_PK3,
        TEGRA_GPIO_PK4,
        TEGRA_GPIO_PK2,
        TEGRA_GPIO_PG0,
        TEGRA_GPIO_PG1,
        TEGRA_GPIO_PG2,
        TEGRA_GPIO_PG3,
        TEGRA_GPIO_PG4,
        TEGRA_GPIO_PG5,
        TEGRA_GPIO_PG6,
        TEGRA_GPIO_PG7,
        TEGRA_GPIO_PH0,
        TEGRA_GPIO_PH1,
        TEGRA_GPIO_PH2,
        TEGRA_GPIO_PH3,
        TEGRA_GPIO_PI0,
        TEGRA_GPIO_PI4,
        TEGRA_GPIO_PT5,
        TEGRA_GPIO_PT6,
        TEGRA_GPIO_PC7,
};

static char oly_unused_pins_p2[] = {
        TEGRA_GPIO_PO1,
        TEGRA_GPIO_PO2,
        TEGRA_GPIO_PO3,
        TEGRA_GPIO_PO4,
        TEGRA_GPIO_PO5,
        TEGRA_GPIO_PO6,
        TEGRA_GPIO_PO7,
        TEGRA_GPIO_PO0,
        TEGRA_GPIO_PY0,
        TEGRA_GPIO_PY1,
        TEGRA_GPIO_PY2,
        TEGRA_GPIO_PY3,
        TEGRA_GPIO_PC1,        
        TEGRA_GPIO_PN5,
        TEGRA_GPIO_PN6,
        TEGRA_GPIO_PW1,
        TEGRA_GPIO_PB3,
        TEGRA_GPIO_PJ3,
        TEGRA_GPIO_PE7,
        TEGRA_GPIO_PF0,
        TEGRA_GPIO_PM6,
        TEGRA_GPIO_PM7,
        TEGRA_GPIO_PT4,
        TEGRA_GPIO_PL5,
        TEGRA_GPIO_PL6,
        TEGRA_GPIO_PL7,
        TEGRA_GPIO_PT2,
        TEGRA_GPIO_PD6,
        TEGRA_GPIO_PD7,
        TEGRA_GPIO_PR3,
        TEGRA_GPIO_PR4,
        TEGRA_GPIO_PR5,
        TEGRA_GPIO_PR6,
        TEGRA_GPIO_PR7,
        TEGRA_GPIO_PS0,
        TEGRA_GPIO_PS1,
        TEGRA_GPIO_PS2,
        TEGRA_GPIO_PQ3,
        TEGRA_GPIO_PQ4,
        TEGRA_GPIO_PQ5,
        TEGRA_GPIO_PBB0,
        TEGRA_GPIO_PZ5,
        TEGRA_GPIO_PK5,
        TEGRA_GPIO_PK6,
        TEGRA_GPIO_PW5,
        TEGRA_GPIO_PD3,
        TEGRA_GPIO_PI7,
        TEGRA_GPIO_PJ0,
        TEGRA_GPIO_PJ2,
        TEGRA_GPIO_PK3,
        TEGRA_GPIO_PK4,
        TEGRA_GPIO_PK2,
        TEGRA_GPIO_PG0,
        TEGRA_GPIO_PG1,
        TEGRA_GPIO_PG2,
        TEGRA_GPIO_PG3,
        TEGRA_GPIO_PG4,
        TEGRA_GPIO_PG5,
        TEGRA_GPIO_PG6,
        TEGRA_GPIO_PG7,
        TEGRA_GPIO_PH0,
        TEGRA_GPIO_PH1,
        TEGRA_GPIO_PH2,
        TEGRA_GPIO_PH3,
        TEGRA_GPIO_PI0,
        TEGRA_GPIO_PI4,
        TEGRA_GPIO_PT5,
        TEGRA_GPIO_PT6,
        TEGRA_GPIO_PC7,
        TEGRA_GPIO_PD1,
};

static char oly_unused_pins_p1[] = {
        TEGRA_GPIO_PO1,
        TEGRA_GPIO_PO2,
        TEGRA_GPIO_PO3,
        TEGRA_GPIO_PO4,
        TEGRA_GPIO_PO5,
        TEGRA_GPIO_PO6,
        TEGRA_GPIO_PO7,
        TEGRA_GPIO_PO0,
        TEGRA_GPIO_PY0,
        TEGRA_GPIO_PY1,
        TEGRA_GPIO_PY2,
        TEGRA_GPIO_PY3,        
        TEGRA_GPIO_PN5,
        TEGRA_GPIO_PN6,
        TEGRA_GPIO_PW1,
        TEGRA_GPIO_PB3,
        TEGRA_GPIO_PJ3,
        TEGRA_GPIO_PF0,
        TEGRA_GPIO_PM6,
        TEGRA_GPIO_PM7,
        TEGRA_GPIO_PL5,
        TEGRA_GPIO_PL6,
        TEGRA_GPIO_PL7,
        TEGRA_GPIO_PT2,
        TEGRA_GPIO_PD6,
        TEGRA_GPIO_PD7,
        TEGRA_GPIO_PR3,
        TEGRA_GPIO_PR4,
        TEGRA_GPIO_PR5,
        TEGRA_GPIO_PR6,
        TEGRA_GPIO_PR7,
        TEGRA_GPIO_PS1,
        TEGRA_GPIO_PQ3,
        TEGRA_GPIO_PQ4,
        TEGRA_GPIO_PQ5,
        TEGRA_GPIO_PBB0,
        TEGRA_GPIO_PZ5,
        TEGRA_GPIO_PK5,
        TEGRA_GPIO_PK6,
        TEGRA_GPIO_PW5,
        TEGRA_GPIO_PD3,
        TEGRA_GPIO_PI7,
        TEGRA_GPIO_PJ0,
        TEGRA_GPIO_PJ2,
        TEGRA_GPIO_PK3,
        TEGRA_GPIO_PK4,
        TEGRA_GPIO_PK2,
        TEGRA_GPIO_PG0,
        TEGRA_GPIO_PG1,
        TEGRA_GPIO_PG2,
        TEGRA_GPIO_PG3,
        TEGRA_GPIO_PG4,
        TEGRA_GPIO_PG5,
        TEGRA_GPIO_PG6,
        TEGRA_GPIO_PG7,
        TEGRA_GPIO_PH0,
        TEGRA_GPIO_PH1,
        TEGRA_GPIO_PH2,
        TEGRA_GPIO_PH3,
        TEGRA_GPIO_PI0,
        TEGRA_GPIO_PI4,
        TEGRA_GPIO_PT5,
        TEGRA_GPIO_PT6,
        TEGRA_GPIO_PC7,
        TEGRA_GPIO_PV7,
        TEGRA_GPIO_PD1,
};

extern void __init tegra_setup_nvodm(bool standard_i2c, bool standard_spi);
extern void __init tegra_register_socdev(void);

static struct resource ram_console_resource[] = {
	{
		.flags = IORESOURCE_MEM,
	}
};

static struct platform_device ram_console_device = {
	.name = "ram_console",
	.id = -1,
	.num_resources = ARRAY_SIZE(ram_console_resource),
	.resource = ram_console_resource,
};

/* 
 * List of i2c devices
 */
/* IMPORTANT: backlight has to be first, don't move it!!! */
static struct i2c_board_info tegra_i2c_bus0_board_info[] = {
	[BACKLIGHT_DEV] = { /* Display backlight */
		I2C_BOARD_INFO(LM3532_NAME, LM3532_I2C_ADDR),
		.platform_data = &lm3532_pdata,
		/*.irq = ..., */
	},
	[TOUCHSCREEN_DEV] = {
		/* XMegaT touchscreen driver */
		I2C_BOARD_INFO(QTOUCH_TS_NAME, XMEGAT_I2C_ADDR),
		.irq = TOUCH_GPIO_INTR,
		.platform_data = &ts_platform_olympus_p_1_43,
	},

#if defined(CONFIG_TEGRA_ODM_OLYMPUS)
	{
		/*  ISL 29030 (prox/ALS) driver */
		I2C_BOARD_INFO(LD_ISL29030_NAME, 0x44),
		.platform_data = &isl29030_als_ir_data_Olympus,
		.irq = 180,
	},
#endif

};
static struct i2c_board_info tegra_i2c_bus3_board_info[] = {
	{
		I2C_BOARD_INFO("akm8975", 0x0C),
		.platform_data = &akm8975_data,
		.irq = TEGRA_GPIO_TO_IRQ(TEGRA_GPIO_PE2),
	},
	{
		I2C_BOARD_INFO("kxtf9", 0x0F),
		.platform_data = &kxtf9_data,
	},
};

static __initdata struct tegra_clk_init_table olympus_clk_init_table[] = {
	/* name		parent		rate		enabled */        
	{ "uartb",	"pll_p",	216000000,	true},
	{ "uartc",	"pll_m",	600000000,	true},
	{ "uartd",	"pll_p",	216000000,	true},
	{ "emc",	"pll_m",	600000000,	true},
	{ "pll_m",	NULL,		600000000,	true},
	{ "mpe",	"pll_m",	250000000,	false},
	{ "pll_a",	NULL,		56448000,	false},
	{ "pll_a_out0",	NULL,		11289600,	false},
	{ "i2s1",	"pll_a_out0",	2822400,	false},
	{ "i2s2",	"clk_m",	26000000,	false},
	{ "sdmmc4",	"pll_p",	48000000,	true},
	{ "sdmmc2",	"pll_p",	24000000,	false},
	{ "spdif_out",	"pll_a_out0",	11289600,	false},
	{ NULL,		NULL,		0,		0},
};

#if 1
static noinline void __init tegra_setup_bluesleep(void)
{
       struct platform_device *pDev = NULL;
       struct resource *res;

       pDev = platform_device_alloc("bluesleep", 0);
       if (!pDev) {
               pr_err("unable to allocate platform device for bluesleep");
               goto fail;
       }

       res = kzalloc(sizeof(struct resource)*3, GFP_KERNEL);
       if (!res) {
               pr_err("unable to allocate resource for bluesleep\n");
               goto fail;
       }

       res[0].name   = "gpio_host_wake";
       res[0].start  = TEGRA_GPIO_PU6;
       res[0].end    = TEGRA_GPIO_PU6;
       res[0].flags  = IORESOURCE_IO;

       res[1].name   = "gpio_ext_wake";
       res[1].start  = TEGRA_GPIO_PU1;
       res[1].end    = TEGRA_GPIO_PU1;
       res[1].flags  = IORESOURCE_IO;

       res[2].name   = "host_wake";
       res[2].start  = gpio_to_irq(TEGRA_GPIO_PU6);
       res[2].end    = gpio_to_irq(TEGRA_GPIO_PU6);
       res[2].flags  = IORESOURCE_IRQ;

       if (platform_device_add_resources(pDev, res, 3)) {
               pr_err("unable to add resources to bluesleep device\n");
               goto fail;
       }

       if (platform_device_add(pDev)) {
               pr_err("unable to add bluesleep device\n");
               goto fail;
       }

fail:
       if (pDev)
               return;
}
#else
static inline void tegra_setup_bluesleep(void) { }
#endif

static int config_unused_pins(char *pins, int num)
{
        int i, ret = 0;
        
        pr_info("%s: ENTRY\n", __func__);

        for (i = 0; i < num; i++) {
                ret = gpio_request(pins[i], "unused");
                if (ret) {
                        printk(KERN_ERR "%s: Error (%d) - gpio_reqest failed for unused GPIO %d\n", __func__,ret, pins[i]);
                } else {                
                        ret = gpio_direction_output(pins[i], 1);
                        if (ret) {
                                printk(KERN_ERR "%s: Error (%d)- gpio_direction failed for unused GPIO %d\n", __func__,ret, pins[i]);
                        }
                }
        }
        
        pr_info("%s: EXIT\n", __func__);

        return ret;
}

static struct platform_device *olympus_devices[] __initdata = {
	&ram_console_device,
};

static void __init tegra_mot_init(void)
{

	tegra_clk_init_from_table(olympus_clk_init_table);

	olympus_pinmux_init();

/*	tegra_common_init();*/
	tegra_setup_nvodm(true, true);
	tegra_register_socdev();

#if 0
#ifdef CONFIG_APANIC_RAM
	apanic_ram_init();
#endif


#ifdef CONFIG_APANIC_MMC
	apanic_mmc_init();
#endif
#endif

	mot_setup_power();
/*	mot_setup_lights(&tegra_i2c_bus0_board_info[BACKLIGHT_DEV]);
	mot_setup_touch(&tegra_i2c_bus0_board_info[TOUCHSCREEN_DEV]);*/
/*	mot_sec_init();
	mot_tcmd_init();*/

	platform_add_devices(olympus_devices, ARRAY_SIZE(olympus_devices)); // ram console

	mot_setup_gadget();

/*	tegra_uart_platform[UART_IPC_OLYMPUS].uart_ipc = 1;
	tegra_uart_platform[UART_IPC_OLYMPUS].uart_wake_host = TEGRA_GPIO_PA0;
	tegra_uart_platform[UART_IPC_OLYMPUS].uart_wake_request = TEGRA_GPIO_PF1;
	tegra_uart_platform[UART_IPC_OLYMPUS].peer_register = mot_mdm_ctrl_peer_register;*/

	if( (bi_powerup_reason() & PWRUP_FACTORY_CABLE) &&
	    (bi_powerup_reason() != PWRUP_INVALID) ){
#ifdef NEED_FACT_BUSY_HINT
		FactoryBusyHint(); //factory workaround no longer needed
#endif
	}

/*	mot_modem_init();*/

	(void) platform_driver_register(&cpcap_usb_connected_driver);

/*	mot_wlan_init();
	mot_sensors_init();

	mot_nvodmcam_init();*/

	printk("%s: registering i2c devices...\n", __func__);

	if(!(bi_powerup_reason() & PWRUP_BAREBOARD)) {
		printk("bus 0: %d devices\n", ARRAY_SIZE(tegra_i2c_bus0_board_info));
		i2c_register_board_info(0, tegra_i2c_bus0_board_info, 
								ARRAY_SIZE(tegra_i2c_bus0_board_info));
	}
	printk("bus 3: %d devices\n", ARRAY_SIZE(tegra_i2c_bus3_board_info));
	i2c_register_board_info(3, tegra_i2c_bus3_board_info, 
							ARRAY_SIZE(tegra_i2c_bus3_board_info));


	pm_power_off = mot_system_power_off;
	tegra_setup_bluesleep();

	/* Configure SPDIF_OUT as GPIO by default, it can be later controlled
	   as needed. When SPDIF_OUT is enabled and if HDMI is connected, it
	   can interefere with CPCAP ID pin, as SPDIF_OUT and ID are coupled.
	*/

	tegra_gpio_enable(TEGRA_GPIO_PD4);
	gpio_request(TEGRA_GPIO_PD4, "spdif_enable");
	gpio_direction_output(TEGRA_GPIO_PD4, 0);
	gpio_export(TEGRA_GPIO_PD4, false);

	if ((HWREV_TYPE_IS_PORTABLE(system_rev) || HWREV_TYPE_IS_FINAL(system_rev)))
	{
		if (HWREV_REV(system_rev) >= HWREV_REV_1 && HWREV_REV(system_rev) < HWREV_REV_2)
		{
			// Olympus P1
			config_unused_pins(oly_unused_pins_p1, ARRAY_SIZE(oly_unused_pins_p1));
		}
		else if (HWREV_REV(system_rev) >= HWREV_REV_2 && HWREV_REV(system_rev) < HWREV_REV_3)
		{
			// Olympus P2
			config_unused_pins(oly_unused_pins_p2, ARRAY_SIZE(oly_unused_pins_p2));
		}
		else if (HWREV_REV(system_rev) >= HWREV_REV_3 || HWREV_TYPE_IS_FINAL(system_rev))
		{
			// Olympus P3 and newer
			config_unused_pins(oly_unused_pins_p3, ARRAY_SIZE(oly_unused_pins_p3));
		}
	}

	tegra_release_bootloader_fb();
}

static void __init mot_fixup(struct machine_desc *desc, struct tag *tags,
                 char **cmdline, struct meminfo *mi)
{
	struct tag *t;
	int i;

	/*
	 * Dump some key ATAGs
	 */
	for (t=tags; t->hdr.size; t = tag_next(t)) {
		switch (t->hdr.tag) {
		case ATAG_WLAN_MAC:        // 57464d41 parsed in board-mot-wlan.c
		case ATAG_BLDEBUG:         // 41000811 same, in board-mot-misc.c
		case ATAG_POWERUP_REASON:  // F1000401 ex: 0x4000, parsed after... ignore
			break;
		case ATAG_CORE:     // 54410001
			printk("%s: atag_core hdr.size=%d\n", __func__, t->hdr.size);
			break;
		case ATAG_CMDLINE:
			printk("%s: atag_cmdline=\"%s\"\n", __func__, t->u.cmdline.cmdline);
			break;
		case ATAG_REVISION: // 54410007
			printk("%s: atag_revision=0x%x\n", __func__, t->u.revision.rev);
			break;
		case ATAG_SERIAL:   // 54410006
			printk("%s: atag_serial=%x%x\n", __func__, t->u.serialnr.low, t->u.serialnr.high);
			break;
		case ATAG_INITRD2:  // 54420005
			printk("%s: atag_initrd2=0x%x size=0x%x\n", __func__, t->u.initrd.start, t->u.initrd.size);
			break;
		case ATAG_MEM:
			printk("%s: atag_mem.start=0x%x, mem.size=0x%x\n", __func__, t->u.mem.start, t->u.mem.size);
			break;
#ifdef CONFIG_MACH_OLYMPUS
		case ATAG_MOTOROLA: // 41000810
			printk("%s: atag_moto allow_fb=%d\n", __func__, t->u.motorola.allow_fb_open);
			break;
#endif
		case ATAG_NVIDIA_TEGRA: // 41000801
			printk("%s: atag_tegra=0x%X\n", __func__, t->u.tegra.bootarg_key);
			break;
		default:
			printk("%s: ATAG %X\n", __func__, t->hdr.tag);
		}
	}

	/*
	 * Dump memory nodes
	 */
	for (i=0; i<mi->nr_banks; i++) {
		printk("%s: bank[%d]=%lx@%lx\n", __func__, i, mi->bank[i].size, mi->bank[i].start);
	}
}

int __init tegra_olympus_protected_aperture_init(void)
{
	tegra_protected_aperture_init(tegra_grhost_aperture);
	return 0;
}
late_initcall(tegra_olympus_protected_aperture_init);

void __init tegra_olympus_reserve(void)
{
	u64 ram_console_start;
	int ret;

	if (memblock_reserve(0x0, 4096) < 0)
		pr_warn("Cannot reserve first 4K of memory for safety\n");

	tegra_reserve(SZ_256M, SZ_8M, SZ_16M);

	/*!!!!!!!!!!!!! Reserve memory for the ram console. !!!!!!!!!!!!!!!!!!!!!!*/
	ram_console_start = memblock_end_of_DRAM() - SZ_1M;

	ret = memblock_remove(ram_console_start, SZ_1M);
	if (ret < 0) {
		pr_err("Failed to reserve 0x%x bytes for ram_console at "
			"0x%llx, err = %d.\n",
			SZ_1M, ram_console_start, ret);
	} else {
		ram_console_resource[0].start = ram_console_start;
		ram_console_resource[0].end = ram_console_start + SZ_1M - 1;
	}
}

MACHINE_START(OLYMPUS, "Olympus")

    .boot_params  = 0x00000100,
    .fixup        = mot_fixup,
    .map_io       = tegra_map_common_io,
/*    .reserve	  = tegra_olympus_reserve,*/
    .init_early	  = tegra_init_early,
    .init_irq     = tegra_init_irq,
    .init_machine = tegra_mot_init,
    .timer        = &tegra_timer,

MACHINE_END

