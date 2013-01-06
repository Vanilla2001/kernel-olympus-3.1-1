/*
 * arch/arm/mach-tegra/board-nvodm.c
 *
 * Converts data from ODM query library into platform data
 *
 * Copyright (c) 2009-2010, NVIDIA Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/serial_8250.h>
#include <linux/dma-mapping.h>
#include <linux/fsl_devices.h>
#include <linux/regulator/machine.h>
#include <linux/lbee9qmb-rfkill.h>
#include <linux/pda_power.h>
#include <linux/gpio.h>
#include <linux/console.h>
#include <linux/reboot.h>
#include <linux/tegra_uart.h>
#include <linux/spi-tegra.h>
#include <linux/i2c-tegra.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/platform_data/tegra_usb.h>
#include <linux/platform_data/tegra_nor.h>

#include <asm/mach/time.h>

#include <mach/clk.h>
#include <mach/gpio.h>
#include <mach/io.h>
#include <mach/iomap.h>
#include <mach/irqs.h>
#include <mach/kbc.h>
#include <mach/nand.h>
#include <mach/pinmux.h>
#include <mach/sdhci.h>
#include <mach/spi.h>
#include <mach/w1.h>
#include <mach/usb_phy.h>

#include <asm/mach-types.h>

#include "clock.h"
#include "devices.h"
#include "gpio-names.h"
#include "pm.h"
#include "board.h"
#include "hwrev.h"
#include "board-olympus.h"

# define BT_RESET 0
# define BT_SHUTDOWN 1

#include <linux/mmc/host.h>

static u64 tegra_dma_mask = DMA_BIT_MASK(32);

static struct plat_serial8250_port debug_uart_platform_data[] = {
	{
		.membase	= IO_ADDRESS(TEGRA_UARTB_BASE),
		.mapbase	= TEGRA_UARTB_BASE,
		.irq		= INT_UARTB,
		.flags		= UPF_BOOT_AUTOCONF,
		.iotype		= UPIO_MEM,
		.regshift	= 2,
		.uartclk	= 0, /* filled in by tegra_olympus_init */
	}, {
		.flags		= 0
	}
};

static struct platform_device debug_uart = {
	.name = "serial8250",
	.id = PLAT8250_DEV_PLATFORM,
	.dev = {
		.platform_data = debug_uart_platform_data,
	},
};

static void __init olympus_debug_uart_init(void)
{

	struct clk *clk;
	printk(KERN_INFO "pICS_%s",__func__);
	clk = tegra_get_clock_by_name("uartb");
	debug_uart_platform_data[0].uartclk = clk_get_rate(clk);

	platform_device_register(&debug_uart);
}

/* PDA power */
static struct pda_power_pdata pda_power_pdata = {
};

static struct platform_device pda_power_device = {
	.name   = "pda_power",
	.id     = -1,
	.dev    = {
		.platform_data  = &pda_power_pdata,
	},
};

static void __init olympus_pdapower_init(void)
{
	
	printk(KERN_INFO "pICS_%s: Starting...",__func__);
	
	platform_device_register(&pda_power_device);

	printk(KERN_INFO "pICS_%s: Ending...",__func__);
}

/* 
 * SDHCI init
 */

extern struct tegra_nand_platform tegra_nand_plat;

static struct tegra_sdhci_platform_data tegra_sdhci_platform[] = {
	[0] = { /* SDHCI 1 - WIFI*/
		.mmc_data = {
			.built_in = 1,
		},
		.wp_gpio = -1,
		.cd_gpio = -1,
		.power_gpio = -1,
		.max_clk_limit = 50000000,
	},
	[1] = {
		
	},
	[2] = {
		.mmc_data = {
			.built_in = 0,
			.card_present = 0,
		},
		.wp_gpio = -1,
		.cd_gpio = 69,
		.power_gpio = -1,
		.max_clk_limit = 50000000,
	},
	[3] = {
		.mmc_data = {
			.built_in = 1,
		},
		.wp_gpio = -1,
		.cd_gpio = -1,
		.power_gpio = -1,
		.is_8bit = 1,
		.max_clk_limit = 50000000,
	},
};
static struct resource tegra_sdhci_resources[][2] = {
	[0] = {
		[0] = {
			.start = TEGRA_SDMMC1_BASE,
			.end = TEGRA_SDMMC1_BASE + TEGRA_SDMMC1_SIZE - 1,
			.flags = IORESOURCE_MEM,
		},
		[1] = {
			.start = INT_SDMMC1,
			.end = INT_SDMMC1,
			.flags = IORESOURCE_IRQ,
		},
	},
	[1] = {
		[0] = {
			.start = TEGRA_SDMMC2_BASE,
			.end = TEGRA_SDMMC2_BASE + TEGRA_SDMMC2_SIZE - 1,
			.flags = IORESOURCE_MEM,
		},
		[1] = {
			.start = INT_SDMMC2,
			.end = INT_SDMMC2,
			.flags = IORESOURCE_IRQ,
		},
	},
	[2] = {
		[0] = {
			.start = TEGRA_SDMMC3_BASE,
			.end = TEGRA_SDMMC3_BASE + TEGRA_SDMMC3_SIZE - 1,
			.flags = IORESOURCE_MEM,
		},
		[1] = {
			.start = INT_SDMMC3,
			.end = INT_SDMMC3,
			.flags = IORESOURCE_IRQ,
		},
	},
	[3] = {
		[0] = {
			.start = TEGRA_SDMMC4_BASE,
			.end = TEGRA_SDMMC4_BASE + TEGRA_SDMMC4_SIZE - 1,
			.flags = IORESOURCE_MEM,
		},
		[1] = {
			.start = INT_SDMMC4,
			.end = INT_SDMMC4,
			.flags = IORESOURCE_IRQ,
		},
	},
};
struct platform_device tegra_sdhci_devices[] = {
	[0] = {
		.id = 0,
		.name = "tegra-sdhci",
		.resource = tegra_sdhci_resources[0],
		.num_resources = ARRAY_SIZE(tegra_sdhci_resources[0]),
		.dev = {
			.platform_data = &tegra_sdhci_platform[0],
			.coherent_dma_mask = DMA_BIT_MASK(32),
			.dma_mask = &tegra_dma_mask,
		},
	},
	[1] = {
		.id = 1,
		.name = "tegra-sdhci",
		.resource = tegra_sdhci_resources[1],
		.num_resources = ARRAY_SIZE(tegra_sdhci_resources[1]),
		.dev = {
			.platform_data = &tegra_sdhci_platform[1],
			.coherent_dma_mask = DMA_BIT_MASK(32),
			.dma_mask = &tegra_dma_mask,
		},
	},
	[2] = {
		.id = 2,
		.name = "tegra-sdhci",
		.resource = tegra_sdhci_resources[2],
		.num_resources = ARRAY_SIZE(tegra_sdhci_resources[2]),
		.dev = {
			.platform_data = &tegra_sdhci_platform[2],
			.coherent_dma_mask = DMA_BIT_MASK(32),
			.dma_mask = &tegra_dma_mask,
		},
	},
	[3] = {
		.id = 3,
		.name = "tegra-sdhci",
		.resource = tegra_sdhci_resources[3],
		.num_resources = ARRAY_SIZE(tegra_sdhci_resources[3]),
		.dev = {
			.platform_data = &tegra_sdhci_platform[3],
			.coherent_dma_mask = DMA_BIT_MASK(32),
			.dma_mask = &tegra_dma_mask,
		},
	},
};

static const char tegra_sdio_ext_reg_str[] = "vsdio_ext";
int tegra_sdhci_boot_device = -1;

static void __init olympus_sdhci_init(void)
{
	int i;

	printk(KERN_INFO "pICS_%s: Starting...",__func__);
	
	/* Olympus P3+, Etna P2+, Etna S3+, Daytona and Sunfire
	   can handle shutting down the external SD card. */
	if ( (HWREV_TYPE_IS_FINAL(system_rev) || (HWREV_TYPE_IS_PORTABLE(system_rev) && (HWREV_REV(system_rev) >= HWREV_REV_3)))) 			{
		tegra_sdhci_platform[2].regulator_str = (char *)tegra_sdio_ext_reg_str;
		}

		/* check if an "MBR" partition was parsed from the tegra partition
		 * command line, and store it in sdhci.3's offset field */
	for (i=0; i<tegra_nand_plat.nr_parts; i++) {
		if (strcmp("mbr", tegra_nand_plat.parts[i].name))
			continue;
		tegra_sdhci_platform[3].offset = tegra_nand_plat.parts[i].offset;
		printk(KERN_INFO "pICS_%s: tegra_sdhci_boot_device plat->offset = 0x%llx ",__func__, tegra_nand_plat.parts[i].offset);		
		}


	platform_device_register(&tegra_sdhci_devices[3]);
	platform_device_register(&tegra_sdhci_devices[0]);
	platform_device_register(&tegra_sdhci_devices[2]);

	printk(KERN_INFO "pICS_%s: Ending...",__func__);
}


struct plat_serial8250_port tegra_uart_platform[] = {
	{
		.membase = IO_ADDRESS(TEGRA_UARTA_BASE),
		.mapbase = TEGRA_UARTA_BASE,
		.irq = INT_UARTA,
	},
	{
		.membase = IO_ADDRESS(TEGRA_UARTB_BASE),
		.mapbase = TEGRA_UARTB_BASE,
		.irq = INT_UARTB,

	},
	{
		.membase = IO_ADDRESS(TEGRA_UARTC_BASE),
		.mapbase = TEGRA_UARTC_BASE,
		.irq = INT_UARTC,

	},
	{
		.membase = IO_ADDRESS(TEGRA_UARTD_BASE),
		.mapbase = TEGRA_UARTD_BASE,
		.irq = INT_UARTD,

	},
	{
		.membase = IO_ADDRESS(TEGRA_UARTE_BASE),
		.mapbase = TEGRA_UARTE_BASE,
		.irq = INT_UARTE,
	},
};
static struct platform_device tegra_uart[] = {
	{
		.name = "tegra_uart",
		.id = 0,
		.dev = {
			.platform_data = &tegra_uart_platform[0],
			.coherent_dma_mask = DMA_BIT_MASK(32),
			.dma_mask = &tegra_dma_mask,
		},
	},
	{
		.name = "tegra_uart",
		.id = 1,
		.dev = {
			.platform_data = &tegra_uart_platform[1],
			.coherent_dma_mask = DMA_BIT_MASK(32),
			.dma_mask = &tegra_dma_mask,
		},
	},
	{
		.name = "tegra_uart",
		.id = 2,
		.dev = {
			.platform_data = &tegra_uart_platform[2],
			.coherent_dma_mask = DMA_BIT_MASK(32),
			.dma_mask = &tegra_dma_mask,
		},
	},
	{
		.name = "tegra_uart",
		.id = 3,
		.dev = {
			.platform_data = &tegra_uart_platform[3],
			.coherent_dma_mask = DMA_BIT_MASK(32),
			.dma_mask = &tegra_dma_mask,
		},
	},
	{
		.name = "tegra_uart",
		.id = 4,
		.dev = {
			.platform_data = &tegra_uart_platform[4],
			.coherent_dma_mask = DMA_BIT_MASK(32),
			.dma_mask = &tegra_dma_mask,
		},
	},

};
static void __init olympus_hsuart_init(void)
{
	printk(KERN_INFO "pICS_%s: Starting...",__func__);

	if (platform_device_register(&tegra_uart[0])) 
			pr_err("%s: failed to register %s.%d\n",
			       __func__, tegra_uart[0].name, tegra_uart[0].id);
	if (platform_device_register(&tegra_uart[2])) 
			pr_err("%s: failed to register %s.%d\n",
			       __func__, tegra_uart[2].name, tegra_uart[2].id);
	if (platform_device_register(&tegra_uart[3])) 
			pr_err("%s: failed to register %s.%d\n",
			       __func__, tegra_uart[3].name, tegra_uart[3].id);
	if (platform_device_register(&tegra_uart[4])) 
			pr_err("%s: failed to register %s.%d\n",
			       __func__, tegra_uart[4].name, tegra_uart[4].id);

	printk(KERN_INFO "pICS_%s: Ending...",__func__);

};

static struct tegra_usb_platform_data tegra_udc_pdata = {
	.port_otg = false,
	.has_hostpc = false,
	.phy_intf = TEGRA_USB_PHY_INTF_UTMI,
	.op_mode = TEGRA_USB_OPMODE_DEVICE,
	.u_data.dev = {
		.vbus_pmu_irq = 0,
		.vbus_gpio = -1,
		.charging_supported = false,
		.remote_wakeup_supported = false,
	},
	.u_cfg.utmi = {
		.hssync_start_delay = 0,
		.elastic_limit = 16,
		.idle_wait_delay = 17,
		.term_range_adj = 6,
		.xcvr_setup = 8,
		.xcvr_lsfslew = 2,
		.xcvr_lsrslew = 2,
		.xcvr_setup_offset = 0,
		.xcvr_use_fuses = 1,
	},
};

static struct tegra_usb_platform_data tegra_ehci1_utmi_pdata = {
	.port_otg = false,
	.has_hostpc = false,
	.phy_intf = TEGRA_USB_PHY_INTF_UTMI,
	.op_mode = TEGRA_USB_OPMODE_HOST,
	.u_data.host = {
		.vbus_gpio = TEGRA_GPIO_PD3,
		.vbus_reg = NULL,
		.hot_plug = true,
		.remote_wakeup_supported = false,
		.power_off_on_suspend = true,
	},
	.u_cfg.utmi = {
		.hssync_start_delay = 9,
		.elastic_limit = 16,
		.idle_wait_delay = 17,
		.term_range_adj = 6,
		.xcvr_setup = 8,
		.xcvr_lsfslew = 2,
		.xcvr_lsrslew = 2,
	},
};

static struct tegra_usb_otg_data tegra_otg_pdata = {
	.ehci_device = &tegra_ehci1_device,
	.ehci_pdata = &tegra_ehci1_utmi_pdata,
};


static void __init olympus_usb_init(void)
{
	printk(KERN_INFO "pICS_%s: Starting...",__func__);

	tegra_udc_device.dev.platform_data = &tegra_udc_pdata;

	tegra_otg_device.dev.platform_data = &tegra_otg_pdata;
	platform_device_register(&tegra_otg_device);


/*	platform_device_register(&tegra_otg);

	tegra_ehci_platform[0].otg_mode = 1;

	tegra_ehci_platform[2].otg_mode = 0;
	tegra_ehci_platform[2].fast_wakeup = 1;

	platform_device_register(&tegra_ehci[0]);
	platform_device_register(&tegra_ehci[2]);*/

	printk(KERN_INFO "pICS_%s: Ending...",__func__);

}
#if 0
static const u32 olympus_keymap[] = {
	KEY(0, 0, KEY_VOLUMEUP),
	KEY(0, 1, KEY_VOLUMEDOWN),
	KEY(0, 2, KEY_AUX),
	KEY(1, 0, KEY_CAMERA_FOCUS),
	KEY(1, 1, KEY_CAMERA),
	KEY(1, 2, KEY_SEARCH),
	KEY(2, 0, KEY_MENU),
	KEY(2, 1, KEY_HOME),
	KEY(2, 2, KEY_BACK),
};

static const struct matrix_keymap_data olympus_keymap_data = {
	.keymap = olympus_keymap,
	.keymap_size = ARRAY_SIZE(olympus_keymap),
};

struct tegra_kbc_platform_data tegra_kbc_platform;

static noinline void __init olympus_kbc_init(void)
{

	tegra_kbc_platform.wake_cnt = 5; /* 0:wake on any key >1:wake on wake_cfg */

	/* debounce time is reported from ODM in terms of clock ticks. */
	tegra_kbc_platform.debounce_cnt = 10;

	/* repeat cycle is reported from ODM in milliseconds,
	 * but needs to be specified in 32KHz ticks */
	tegra_kbc_platform.repeat_cnt = 1024;

	tegra_kbc_platform.pin_cfg[0].num = 0;
	tegra_kbc_platform.pin_cfg[0].is_row = true;
	tegra_kbc_platform.pin_cfg[1].num = 1;
	tegra_kbc_platform.pin_cfg[1].is_row = true;
	tegra_kbc_platform.pin_cfg[2].num = 2;
	tegra_kbc_platform.pin_cfg[2].is_row = true;
	tegra_kbc_platform.pin_cfg[16].num = 0;
	tegra_kbc_platform.pin_cfg[16].en = true; /*ICS check if is_col = en*/
	tegra_kbc_platform.pin_cfg[17].num = 1;
	tegra_kbc_platform.pin_cfg[17].en = true;
	tegra_kbc_platform.pin_cfg[18].num = 2;
	tegra_kbc_platform.pin_cfg[18].en = true;

	tegra_kbc_platform.keymap_data = &olympus_keymap_data;
/* ICS check if scancodes are needed
	tegra_kbc_platform.keymap[0]=115;
	tegra_kbc_platform.keymap[1]=114;
	tegra_kbc_platform.keymap[2]=152;
	tegra_kbc_platform.keymap[16]=211;
	tegra_kbc_platform.keymap[17]=212;
	tegra_kbc_platform.keymap[18]=217;
	tegra_kbc_platform.keymap[32]=139;
	tegra_kbc_platform.keymap[33]=102;
	tegra_kbc_platform.keymap[34]=158;
*/
}
#endif
#ifdef CONFIG_LBEE9QMB_RFKILL
static struct lbee9qmb_platform_data lbee9qmb_platform;
static struct platform_device lbee9qmb_device = {
	.name = "lbee9qmb-rfkill",
	.dev = {
		.platform_data = &lbee9qmb_platform,
	},
};
static noinline void __init olympus_rfkill_init(void)
{

	lbee9qmb_platform.delay=5;
	lbee9qmb_platform.gpio_pwr=-1;
	lbee9qmb_platform.gpio_reset = 160;

	if (platform_device_register(&lbee9qmb_device))
					pr_err("%s: registration failed\n", __func__);
				return;
}
#else
static void olympus_rfkill_init(void) { }
#endif

static struct tegra_spi_platform_data tegra_spi_platform[] = {
	[0] = {
		/*.is_slink = true,*/
	},
	[1] = {
		/*.is_slink = true,*/
	},
	[2] = {
		/*.is_slink = true,*/
	},
	[3] = {
		/*.is_slink = true,*/
	},
	[4] = {
		/*.is_slink = false,*/
	},
};
static struct platform_device tegra_spi_devices[] = {
	[0] = {
		.name = "tegra_spi_slave",
		.id = 0,
		.dev = {
			.platform_data = &tegra_spi_platform[0],
		},
	},
	[1] = {
		.name = "tegra_spi",
		.id = 1,
		.dev = {
			.platform_data = &tegra_spi_platform[1],
		},
	},
	[2] = {
		.name = "tegra_spi",
		.id = 2,
		.dev = {
			.platform_data = &tegra_spi_platform[2],
		},
	},
	[3] = {
		.name = "tegra_spi",
		.id = 3,
		.dev = {
			.platform_data = &tegra_spi_platform[3],
		},
	},
	[4] = {
		.name = "tegra_spi",
		.id = 4,
		.dev = {
			.platform_data = &tegra_spi_platform[4],
		},
	},
};
static void __init olympus_spi_init(void)
{


	int rc;

	rc = platform_device_register(&tegra_spi_devices[0]);

	if (rc) {
		pr_err("%s: registration of %s.%d failed\n",
		       __func__, tegra_spi_devices[0].name, tegra_spi_devices[0].id);
	}

	rc = platform_device_register(&tegra_spi_devices[1]);
	if (rc) {
		pr_err("%s: registration of %s.%d failed\n",
		       __func__, tegra_spi_devices[1].name, tegra_spi_devices[1].id);
	}

	rc = platform_device_register(&tegra_spi_devices[2]);
	if (rc) {
		pr_err("%s: registration of %s.%d failed\n",
		       __func__, tegra_spi_devices[2].name, tegra_spi_devices[2].id);
	}

	printk(KERN_INFO "pICS_%s: Ending...",__func__);
}

static struct tegra_i2c_platform_data tegra_i2c_platform[] = {
	[0] = {
		.adapter_nr = 0,
		.bus_count = 1,
		.bus_mux = { 0, 0 },
		.bus_clk_rate = { 100000, 0 }, /* default to 100KHz */
		.is_dvc = false,
	},
	[1] = {
		.adapter_nr = 1,
		.bus_count = 1,
		.bus_mux = { 0, 0 },
		.bus_clk_rate = { 100000, 0 },
		.is_dvc = false,
	},
	[2] = {
		.adapter_nr = 2,
		.bus_count = 1,
		.bus_mux = { 0, 0 },
		.bus_clk_rate = { 100000, 0 },
		.is_dvc = false,
	},
	[3] = {
		.adapter_nr = 3,
		.bus_count = 1,
		.bus_mux = { 0, 0 },
		.bus_clk_rate = { 100000, 0 },
		.is_dvc = true,
	},
};
static struct platform_device tegra_i2c_devices[] = {
	[0] = {
		.name = "tegra_i2c",
		.id = 0,
		.dev = {
			.platform_data = &tegra_i2c_platform[0],
		},
	},
	[1] = {
		.name = "tegra_i2c",
		.id = 1,
		.dev = {
			.platform_data = &tegra_i2c_platform[1],
		},
	},
	[2] = {
		.name = "tegra_i2c",
		.id = 2,
		.dev = {
			.platform_data = &tegra_i2c_platform[2],
		},
	},
	[3] = {
		.name = "tegra_i2c",
		.id = 3,
		.dev = {
			.platform_data = &tegra_i2c_platform[3],
		},
	},
};
static noinline void __init olympus_i2c_init(void)
{
	tegra_i2c_platform[0].bus_clk_rate[0] = 400*1000;
	tegra_i2c_platform[1].bus_clk_rate[0] = 400*1000;
	tegra_i2c_platform[2].bus_clk_rate[0] = 400*1000;

	if (platform_device_register(&tegra_i2c_devices[0]))
		pr_err("%s: failed to register %s.%d\n",
			       __func__, tegra_i2c_devices[0].name, tegra_i2c_devices[0].id);
	if (platform_device_register(&tegra_i2c_devices[1]))
		pr_err("%s: failed to register %s.%d\n",
			       __func__, tegra_i2c_devices[0].name, tegra_i2c_devices[0].id);
	if (platform_device_register(&tegra_i2c_devices[2]))
		pr_err("%s: failed to register %s.%d\n",
			       __func__, tegra_i2c_devices[0].name, tegra_i2c_devices[0].id);
	if (platform_device_register(&tegra_i2c_devices[3]))
		pr_err("%s: failed to register %s.%d\n",
			       __func__, tegra_i2c_devices[0].name, tegra_i2c_devices[0].id);
}

static void __init olympus_w1_init(void)
{

	/*tegra_w1_platform.pinmux = 1;*/
	if (platform_device_register(&tegra_w1_device)) {
		pr_err("%s: failed to register %s.%d\n",
		       __func__, tegra_w1_device.name, tegra_w1_device.id);
	}
}

static void tegra_system_power_off(void)
{
	struct regulator *regulator = regulator_get(NULL, "soc_main");

	if (!IS_ERR(regulator)) {
		int rc;
		regulator_enable(regulator);
		rc = regulator_disable(regulator);
		pr_err("%s: regulator_disable returned %d\n", __func__, rc);
	} else {
		pr_err("%s: regulator_get returned %ld\n", __func__,
		       PTR_ERR(regulator));
	}
	local_irq_disable();
	while (1) {
		dsb();
		__asm__ ("wfi");
	}
}
#if 0
static struct tegra_suspend_platform_data tegra_suspend_platform = {
	.cpu_timer = 2000,
};


static void __init olympus_suspend_init(void)
{
#ifdef CONFIG_ARCH_TEGRA_2x_SOC
	const int wakepad_irq[] = {
		gpio_to_irq(TEGRA_GPIO_PO5), gpio_to_irq(TEGRA_GPIO_PV3),
		gpio_to_irq(TEGRA_GPIO_PL1), gpio_to_irq(TEGRA_GPIO_PB6),
		gpio_to_irq(TEGRA_GPIO_PN7), gpio_to_irq(TEGRA_GPIO_PA0),
		gpio_to_irq(TEGRA_GPIO_PU5), gpio_to_irq(TEGRA_GPIO_PU6),
		gpio_to_irq(TEGRA_GPIO_PC7), gpio_to_irq(TEGRA_GPIO_PS2),
		gpio_to_irq(TEGRA_GPIO_PAA1), gpio_to_irq(TEGRA_GPIO_PW3),
		gpio_to_irq(TEGRA_GPIO_PW2), gpio_to_irq(TEGRA_GPIO_PY6),
		gpio_to_irq(TEGRA_GPIO_PV6), gpio_to_irq(TEGRA_GPIO_PJ7),
		INT_RTC, INT_KBC, INT_EXTERNAL_PMU,
		/* FIXME: USB wake pad interrupt mapping may be wrong */
		INT_USB, INT_USB3, INT_USB, INT_USB3,
		gpio_to_irq(TEGRA_GPIO_PI5), gpio_to_irq(TEGRA_GPIO_PV2),
		gpio_to_irq(TEGRA_GPIO_PS4), gpio_to_irq(TEGRA_GPIO_PS5),
		gpio_to_irq(TEGRA_GPIO_PS0), gpio_to_irq(TEGRA_GPIO_PQ6),
		gpio_to_irq(TEGRA_GPIO_PQ7), gpio_to_irq(TEGRA_GPIO_PN2),
	};
#endif /* CONFIG_ARCH_TEGRA_2x_SOC */

	/*tegra_suspend_platform.dram_suspend = true;
	tegra_suspend_platform.core_off = true;*/
	tegra_suspend_platform.cpu_timer = 800;
	tegra_suspend_platform.cpu_off_timer = 600;
	tegra_suspend_platform.core_timer = 1842;
	tegra_suspend_platform.core_off_timer = 31;
/*	tegra_suspend_platform.separate_req = 1;*/
	tegra_suspend_platform.corereq_high = 1;
	tegra_suspend_platform.sysclkreq_high = 1;
/*	tegra_suspend_platform.wake_enb = 0;
	tegra_suspend_platform.wake_low = 0;
	tegra_suspend_platform.wake_high = 0;
	tegra_suspend_platform.wake_any = 0;*/
	
	enable_irq_wake(wakepad_irq[2]);
/*	tegra_suspend_platform.wake_enb = 4;
	tegra_suspend_platform.wake_low = 4;*/

	enable_irq_wake(wakepad_irq[5]);
/*	tegra_suspend_platform.wake_enb = 36;
	tegra_suspend_platform.wake_high = 32;*/

	enable_irq_wake(wakepad_irq[6]);
/*	tegra_suspend_platform.wake_enb = 100;
	tegra_suspend_platform.wake_any = 64;*/


	enable_irq_wake(wakepad_irq[7]);
/*	tegra_suspend_platform.wake_enb = 228;
	tegra_suspend_platform.wake_any = 192;*/


	enable_irq_wake(wakepad_irq[17]);
/*	tegra_suspend_platform.wake_enb = 131300;
	tegra_suspend_platform.wake_high = 131104;*/


	enable_irq_wake(wakepad_irq[18]);
/*	tegra_suspend_platform.wake_enb = 393444;
	tegra_suspend_platform.wake_high = 393248;*/

	enable_irq_wake(wakepad_irq[24]);
/*	tegra_suspend_platform.wake_enb = 17170660;
	tegra_suspend_platform.wake_any = 16777408;*/

	printk(KERN_INFO "pICS_%s: tegra_init_suspend(tegra_suspend_platform)",__func__);

	tegra_init_suspend(&tegra_suspend_platform);
	/*tegra_init_idle(&tegra_suspend_platform);*/
}
#endif

static int tegra_reboot_notify(struct notifier_block *nb,
				unsigned long event, void *data)
{
	printk(KERN_INFO "pICS_%s: event = [%lu]",__func__, event);
	switch (event) {
	case SYS_RESTART:
	case SYS_HALT:
	case SYS_POWER_OFF:
		/* USB power rail must be enabled during boot */
		/*NvOdmEnableUsbPhyPowerRail(1);*/
		return NOTIFY_OK;
	}
	return NOTIFY_DONE;
}

static struct notifier_block tegra_reboot_nb = {
	.notifier_call = tegra_reboot_notify,
	.next = NULL,
	.priority = 0
};

static void olympus_reboot_init(void)
{
	int rc = register_reboot_notifier(&tegra_reboot_nb);
	if (rc)
		pr_err("%s: failed to regsiter platform reboot notifier\n",
			__func__);
}

void __init olympus_devices_init()
{
	printk(KERN_INFO "pICS_%s: olympus_debug_uart_init();\n",__func__);
	olympus_debug_uart_init();
	printk(KERN_INFO "pICS_%s: olympus_ehci_init();\n",__func__);
	olympus_usb_init();
	printk(KERN_INFO "pICS_%s: olympus_hsuart_init();\n",__func__);
	olympus_hsuart_init();
	printk(KERN_INFO "pICS_%s: olympus_sdhci_init();\n",__func__);
	olympus_sdhci_init();
	printk(KERN_INFO "pICS_%s: olympus_rfkill_init();\n",__func__);
	olympus_rfkill_init();
	printk(KERN_INFO "pICS_%s: olympus_kbc_init();\n",__func__);
/*	olympus_kbc_init();*/
	printk(KERN_INFO "pICS_%s: olympus_i2c_init();\n",__func__);
	olympus_i2c_init();
	printk(KERN_INFO "pICS_%s: olympus_spi_init();\n",__func__);
	olympus_spi_init();
	printk(KERN_INFO "pICS_%s: olympus_w1_init();\n",__func__);
	olympus_w1_init();
	if (0==1) {	
	printk(KERN_INFO "pICS_%s: olympus_pdapower_init();\n",__func__);
	olympus_pdapower_init();
	}
/*	platform_add_devices(olympus_devices, ARRAY_SIZE(olympus_devices));*/

	pm_power_off = tegra_system_power_off;
	/*tegra_setup_suspend();*/
	olympus_reboot_init();
}

