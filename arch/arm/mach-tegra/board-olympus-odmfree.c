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
#include <linux/regulator/machine.h>
#include <linux/lbee9qmb-rfkill.h>
#include <linux/gpio.h>
#include <linux/console.h>
#include <linux/reboot.h>
#include <linux/tegra_uart.h>
#include <linux/spi-tegra.h>
#include <linux/i2c-tegra.h>
#include <linux/mtd/partitions.h>

#include <mach/iomap.h>
#include <mach/io.h>
#include <mach/pinmux.h>
#include <mach/sdhci.h>
#include <mach/nand.h>
#include <mach/kbc.h>
#include <mach/spi.h>
#include <mach/w1.h>

#include <asm/mach-types.h>

#include "gpio-names.h"
#include "pm.h"
#include "board.h"
#include "hwrev.h"
#include "board-olympus.h"

# define BT_RESET 0
# define BT_SHUTDOWN 1

#include <linux/mmc/host.h>

static u64 tegra_dma_mask = DMA_BIT_MASK(32);

/*
static struct debug_port_data uart_debug_port = {
			.port = NvOdmDebugConsole_UartA,
};*/


extern const struct tegra_pingroup_config *tegra_pinmux_get(const char *dev_id,
	int config, int *len);


static struct plat_serial8250_port debug_uart_platform[] = {
	{
		.flags = UPF_BOOT_AUTOCONF,
		.iotype = UPIO_MEM,
		.regshift = 2,
	}, {
		.flags = 0,
	}
};

static struct platform_device debug_uart = {
	.name = "serial8250",
	.id = PLAT8250_DEV_PLATFORM,
	.dev = {
		.platform_data = debug_uart_platform,
	},
};

static void __init tegra_setup_debug_uart(void)
{
	/*NvOdmDebugConsole uart = NvOdmQueryDebugConsole();
	const struct tegra_pingroup_config *pinmux = NULL;
	const NvU32 *odm_table;
	struct clk *c = NULL;
	NvU32 odm_nr;
	int nr_pins;

	if (uart < NvOdmDebugConsole_UartA ||
	    uart > NvOdmDebugConsole_UartE)
		return;

	NvOdmQueryPinMux(NvOdmIoModule_Uart, &odm_table, &odm_nr);
	if (odm_nr <= (uart - NvOdmDebugConsole_UartA)) {
		pr_err("%s: ODM query configured improperly\n", __func__);
		WARN_ON(1);
		return;
	}

	odm_nr = odm_table[uart - NvOdmDebugConsole_UartA];

	if (uart == NvOdmDebugConsole_UartA) {
		printk(KERN_INFO "pICS_%s: pinmux = tegra_pinmux_get('tegra_uart.0', odm_nr, &nr_pins) = \n",__func__);
		pinmux = tegra_pinmux_get("tegra_uart.0", odm_nr, &nr_pins);
		c = clk_get_sys("uart.0", NULL);
		printk(KERN_INFO "pICS_%s: clk_get_sys('uart.0', NULL)\n",__func__);
		debug_uart_platform[0].membase = IO_ADDRESS(TEGRA_UARTA_BASE);
		debug_uart_platform[0].mapbase = TEGRA_UARTA_BASE;
		debug_uart_platform[0].irq = INT_UARTA;
	} else if (uart == NvOdmDebugConsole_UartB) {
		printk(KERN_INFO "pICS_%s: pinmux = tegra_pinmux_get('tegra_uart.1', odm_nr, &nr_pins)\n",__func__);
		pinmux = tegra_pinmux_get("tegra_uart.1", odm_nr, &nr_pins);
		c = clk_get_sys("uart.1", NULL);
		printk(KERN_INFO "pICS_%s: clk_get_sys('uart.1', NULL)\n",__func__);
		debug_uart_platform[0].membase = IO_ADDRESS(TEGRA_UARTB_BASE);
		debug_uart_platform[0].mapbase = TEGRA_UARTB_BASE;
		debug_uart_platform[0].irq = INT_UARTB;
	} else if (uart == NvOdmDebugConsole_UartC) {
		printk(KERN_INFO "pICS_%s: pinmux = tegra_pinmux_get('tegra_uart.2', odm_nr, &nr_pins)\n",__func__);
		pinmux = tegra_pinmux_get("tegra_uart.2", odm_nr, &nr_pins);
		c = clk_get_sys("uart.2", NULL);
		printk(KERN_INFO "pICS_%s: clk_get_sys('uart.2', NULL)\n",__func__);
		debug_uart_platform[0].membase = IO_ADDRESS(TEGRA_UARTC_BASE);
		debug_uart_platform[0].mapbase = TEGRA_UARTC_BASE;
		debug_uart_platform[0].irq = INT_UARTC;
	} else if (uart == NvOdmDebugConsole_UartD) {
		printk(KERN_INFO "pICS_%s: pinmux = tegra_pinmux_get('tegra_uart.3', odm_nr, &nr_pins)\n",__func__);
		pinmux = tegra_pinmux_get("tegra_uart.3", odm_nr, &nr_pins);
		c = clk_get_sys("uart.3", NULL);
		printk(KERN_INFO "pICS_%s: clk_get_sys('uart.3', NULL)\n",__func__);
		debug_uart_platform[0].membase = IO_ADDRESS(TEGRA_UARTD_BASE);
		debug_uart_platform[0].mapbase = TEGRA_UARTD_BASE;
		debug_uart_platform[0].irq = INT_UARTD;
	} else if (uart == NvOdmDebugConsole_UartE) {
		printk(KERN_INFO "pICS_%s: pinmux = tegra_pinmux_get('tegra_uart.4', odm_nr, &nr_pins)\n",__func__);
		pinmux = tegra_pinmux_get("tegra_uart.4", odm_nr, &nr_pins);
		c = clk_get_sys("uart.4", NULL);
		printk(KERN_INFO "pICS_%s: clk_get_sys('uart.4', NULL)\n",__func__);
		debug_uart_platform[0].membase = IO_ADDRESS(TEGRA_UARTE_BASE);
		debug_uart_platform[0].mapbase = TEGRA_UARTE_BASE;
		debug_uart_platform[0].irq = INT_UARTE;
	}

	if (!c || !pinmux || !nr_pins) {
		if (c)
			clk_put(c);
		return;
	}

	tegra_pinmux_config_tristate_table(pinmux, nr_pins, TEGRA_TRI_NORMAL);
	clk_set_rate(c, 115200*16);
	clk_enable(c);
	debug_uart_platform[0].uartclk = clk_get_rate(c);

	platform_device_register(&debug_uart);

	uart_debug_port.port = uart;
	uart_debug_port.pinmux = pinmux;
	uart_debug_port.nr_pins = nr_pins;
	uart_debug_port.clk_data = c;*/
}


static void tegra_debug_port_suspend(void)
{

/*TODO	if (uart_debug_port.port == NvOdmDebugConsole_None)
		return;
	clk_disable(uart_debug_port.clk_data);
	tegra_pinmux_config_tristate_table(uart_debug_port.pinmux,
				uart_debug_port.nr_pins, TEGRA_TRI_TRISTATE);*/
}

static void tegra_debug_port_resume(void)
{
/*TODO	if (uart_debug_port.port == NvOdmDebugConsole_None)
		return;
	clk_enable(uart_debug_port.clk_data);
	tegra_pinmux_config_tristate_table(uart_debug_port.pinmux,
				uart_debug_port.nr_pins, TEGRA_TRI_NORMAL);*/
}


#ifdef CONFIG_MMC_SDHCI_TEGRA
extern struct tegra_nand_platform tegra_nand_plat;

static struct tegra_sdhci_platform_data tegra_sdhci_platform[] = {
	[0] = {
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

#define active_high(_pin) ((_pin)->activeState == NvOdmGpioPinActiveState_High ? 1 : 0)

static void __init tegra_setup_sdhci(void) {

	int i;

	printk(KERN_INFO "pICS_%s: Starting...",__func__);
	
	/* Olympus P3+, Etna P2+, Etna S3+, Daytona and Sunfire
	   can handle shutting down the external SD card. */
/*	if ( (HWREV_TYPE_IS_FINAL(system_rev) || (HWREV_TYPE_IS_PORTABLE(system_rev) && (HWREV_REV(system_rev) >= HWREV_REV_3)))) {
		tegra_sdhci_platform[2].mmc_data {
			.embedded_sdio = (char *)tegra_sdio_ext_reg_str;
		}
	}
*/
/*	tegra_sdhci_platform[3].offset = 0x680000; */ /*IMPORTANT this is tegrapart related this one is for d00*/

		/* check if an "MBR" partition was parsed from the tegra partition
		 * command line, and store it in sdhci.3's offset field */
	for (i=0; i<tegra_nand_plat.nr_parts; i++) {
		if (strcmp("mbr", tegra_nand_plat.parts[i].name))
			continue;
		tegra_sdhci_platform[3].offset = tegra_nand_plat.parts[i].offset;
		
		}


	platform_device_register(&tegra_sdhci_devices[3]);
	platform_device_register(&tegra_sdhci_devices[0]);
	platform_device_register(&tegra_sdhci_devices[2]);

	printk(KERN_INFO "pICS_%s: Ending...",__func__);
}
#else
static void __init tegra_setup_sdhci(void) { }
#endif

#ifdef CONFIG_SERIAL_TEGRA

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
static void __init tegra_setup_hsuart(void)
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

}
#else
static void __init tegra_setup_hsuart(void) { }
#endif

#ifdef CONFIG_USB_TEGRA_HCD
static struct tegra_hcd_platform_data tegra_hcd_platform[] = {
	[0] = {
		.instance = 0,
	},
	[1] = {
		.instance = 1,
	},
	[2] = {
		.instance = 2,
	},
};
static struct resource tegra_hcd_resources[][2] = {
	[0] = {
		[0] = {
			.flags = IORESOURCE_MEM,
			.start = TEGRA_USB_BASE,
			.end = TEGRA_USB_BASE + TEGRA_USB_SIZE - 1,
		},
		[1] = {
			.flags = IORESOURCE_IRQ,
			.start = INT_USB,
			.end = INT_USB,
		},
	},
	[1] = {
		[0] = {
			.flags = IORESOURCE_MEM,
			.start = TEGRA_USB1_BASE,
			.end = TEGRA_USB1_BASE + TEGRA_USB1_SIZE - 1,
		},
		[1] = {
			.flags = IORESOURCE_IRQ,
			.start = INT_USB2,
			.end = INT_USB2,
		},
	},
	[2] = {
		[0] = {
			.flags = IORESOURCE_MEM,
			.start = TEGRA_USB2_BASE,
			.end = TEGRA_USB2_BASE + TEGRA_USB2_SIZE - 1,
		},
		[1] = {
			.flags = IORESOURCE_IRQ,
			.start = INT_USB3,
			.end = INT_USB3,
		},
	},
};
/* EHCI transfers must be 32B aligned */
static u64 tegra_ehci_dma_mask = DMA_BIT_MASK(32) & ~0x1f;
static struct platform_device tegra_hcd[] = {
	[0] = {
		.name = "tegra-ehci",
		.id = 0,
		.dev = {
			.platform_data = &tegra_hcd_platform[0],
			.coherent_dma_mask = DMA_BIT_MASK(32) & ~0x1f,
			.dma_mask = &tegra_ehci_dma_mask,
		},
		.resource = tegra_hcd_resources[0],
		.num_resources = ARRAY_SIZE(tegra_hcd_resources[0]),
	},
	[1] = {
		.name = "tegra-ehci",
		.id = 1,
		.dev = {
			.platform_data = &tegra_hcd_platform[1],
			.coherent_dma_mask = DMA_BIT_MASK(32) & ~0x1f,
			.dma_mask = &tegra_ehci_dma_mask,
		},
		.resource = tegra_hcd_resources[1],
		.num_resources = ARRAY_SIZE(tegra_hcd_resources[1]),
	},
	[2] = {
		.name = "tegra-ehci",
		.id = 2,
		.dev = {
			.platform_data = &tegra_hcd_platform[2],
			.coherent_dma_mask = DMA_BIT_MASK(32) & ~0x1f,
			.dma_mask = &tegra_ehci_dma_mask,
		},
		.resource = tegra_hcd_resources[2],
		.num_resources = ARRAY_SIZE(tegra_hcd_resources[2]),
	},
};

#ifdef CONFIG_USB_TEGRA_OTG
#define otg_is_okay(_instance) ((_instance)==0)
static struct tegra_otg_platform_data tegra_otg_platform = {
	.instance = 0,
};
static struct resource tegra_otg_resources[] = {
	[0] = {
		.start = TEGRA_USB_BASE,
		.end = TEGRA_USB_BASE + TEGRA_USB_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = INT_USB,
		.end = INT_USB,
		.flags = IORESOURCE_IRQ,
	},
};
static struct platform_device tegra_otg = {
	.name = "tegra-otg",
	.id = 0,
	.resource = tegra_otg_resources,
	.num_resources = ARRAY_SIZE(tegra_otg_resources),
	.dev = {
		.platform_data = &tegra_otg_platform,
		.coherent_dma_mask = DMA_BIT_MASK(32),
		.dma_mask = &tegra_dma_mask,
	},
};
#else
#define otg_is_okay(_instance) (0)
#endif

static void __init tegra_setup_hcd(void)
{
		
	static const NvOdmUsbProperty Usb1Property =
   	{
    /* Specifies the USB controller is connected to a standard UTMI interface
     (only valid for ::NvOdmIoModule_Usb).
     NvOdmUsbInterfaceType_Utmi = 1,*/
		1,
		/* /// Specifies charger type 0, USB compliant charger, when D+ and D- are at low voltage.
    		NvOdmUsbChargerType_SE0 = 1,
		/// Specifies charger type 2, when D+ is low and D- is high.
    		NvOdmUsbChargerType_SK = 4,
    		/// Specifies charger type 3, when D+ and D- are at high voltage.
    		NvOdmUsbChargerType_SE1 = 8,

	        (NvOdmUsbChargerType_SE0 | NvOdmUsbChargerType_SE1 | NvOdmUsbChargerType_SK),*/
		(1 | 8 | 4),
	        20,
        	0, /*NV_FALSE,*/
		/// Specifies the instance as USB OTG.
    		4, /* NvOdmUsbModeType_OTG */
	        0, /*NvOdmUsbIdPinType_None,*/
	        0, /*NvOdmUsbConnectorsMuxType_None,*/
        	0 /*NV_FALSE*/
   	};

	printk(KERN_INFO "pICS_%s: Starting...",__func__);

	tegra_otg_platform.usb_property = &Usb1Property;
	platform_device_register(&tegra_otg);

	tegra_hcd_platform[0].otg_mode = 1;

	tegra_hcd_platform[2].otg_mode = 0;
	tegra_hcd_platform[2].fast_wakeup = 1;

	platform_device_register(&tegra_hcd[0]);
	platform_device_register(&tegra_hcd[2]);

	printk(KERN_INFO "pICS_%s: Ending...",__func__);

}
#else
static inline void tegra_setup_hcd(void) { }
#endif

#ifdef CONFIG_KEYBOARD_TEGRA
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

static noinline void __init tegra_setup_kbc(void)
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
#else
static void tegra_setup_kbc(void) { }
#endif

static void tegra_setup_gpio_key(void) { }

#ifdef CONFIG_LBEE9QMB_RFKILL
static struct lbee9qmb_platform_data lbee9qmb_platform;
static struct platform_device lbee9qmb_device = {
	.name = "lbee9qmb-rfkill",
	.dev = {
		.platform_data = &lbee9qmb_platform,
	},
};
static noinline void __init tegra_setup_rfkill(void)
{

	lbee9qmb_platform.delay=5;
	lbee9qmb_platform.gpio_pwr=-1;
	lbee9qmb_platform.gpio_reset = 160;

	if (platform_device_register(&lbee9qmb_device))
					pr_err("%s: registration failed\n", __func__);
				return;
}
#else
static void tegra_setup_rfkill(void) { }
#endif

#ifdef CONFIG_SPI_TEGRA
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
static noinline void __init tegra_setup_spi(void)
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
#else
static void tegra_setup_spi(void) { }
#endif

#ifdef CONFIG_I2C_TEGRA
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
static noinline void __init tegra_setup_i2c(void)
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
#else
static void tegra_setup_i2c(void) { }
#endif

#ifdef CONFIG_W1_MASTER_TEGRA
static struct tegra_w1_platform_data tegra_w1_platform;
static struct platform_device tegra_w1_device = {
	.name = "tegra_w1",
	.id = 0,
	.dev = {
		.platform_data = &tegra_w1_platform,
	},
};
static noinline void __init tegra_setup_w1(void)
{

	/*tegra_w1_platform.pinmux = 1;*/
	if (platform_device_register(&tegra_w1_device)) {
		pr_err("%s: failed to register %s.%d\n",
		       __func__, tegra_w1_device.name, tegra_w1_device.id);
	}
}
#else
static void tegra_setup_w1(void) { }
#endif

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

static struct tegra_suspend_platform_data tegra_suspend_platform = {
	.cpu_timer = 2000,
};

static void __init tegra_setup_suspend(void)
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
	tegra_suspend_platform.separate_req = 1;
	tegra_suspend_platform.corereq_high = 1;
	tegra_suspend_platform.sysclkreq_high = 1;
	tegra_suspend_platform.wake_enb = 0;
	tegra_suspend_platform.wake_low = 0;
	tegra_suspend_platform.wake_high = 0;
	tegra_suspend_platform.wake_any = 0;
	
	enable_irq_wake(wakepad_irq[2]);
	tegra_suspend_platform.wake_enb = 4;
	tegra_suspend_platform.wake_low = 4;

	enable_irq_wake(wakepad_irq[5]);
	tegra_suspend_platform.wake_enb = 36;
	tegra_suspend_platform.wake_high = 32;

	enable_irq_wake(wakepad_irq[6]);
	tegra_suspend_platform.wake_enb = 100;
	tegra_suspend_platform.wake_any = 64;


	enable_irq_wake(wakepad_irq[7]);
	tegra_suspend_platform.wake_enb = 228;
	tegra_suspend_platform.wake_any = 192;


	enable_irq_wake(wakepad_irq[17]);
	tegra_suspend_platform.wake_enb = 131300;
	tegra_suspend_platform.wake_high = 131104;


	enable_irq_wake(wakepad_irq[18]);
	tegra_suspend_platform.wake_enb = 393444;
	tegra_suspend_platform.wake_high = 393248;

	enable_irq_wake(wakepad_irq[24]);
	tegra_suspend_platform.wake_enb = 17170660;
	tegra_suspend_platform.wake_any = 16777408;

	printk(KERN_INFO "pICS_%s: tegra_init_suspend(tegra_suspend_platform)",__func__);

	tegra_init_suspend(&tegra_suspend_platform);
	/*tegra_init_idle(&tegra_suspend_platform);*/
}

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

static void __init tegra_setup_reboot(void)
{
	int rc = register_reboot_notifier(&tegra_reboot_nb);
	if (rc)
		pr_err("%s: failed to regsiter platform reboot notifier\n",
			__func__);
}

static int __init tegra_setup_data(void)
{
	printk(KERN_INFO "pICS_%s: empty list of devices",__func__);
/*	platform_add_devices(nvodm_devices, ARRAY_SIZE(nvodm_devices));*/
	return 0;
}
postcore_initcall(tegra_setup_data);

void __init tegra_setup_nvodm(bool standard_i2c, bool standard_spi)
{
	tegra_setup_debug_uart();
	tegra_setup_hcd();
	tegra_setup_hsuart();
	tegra_setup_sdhci();
	tegra_setup_rfkill();
	tegra_setup_kbc();
	tegra_setup_gpio_key();
	if (standard_i2c)
		tegra_setup_i2c();
	if (standard_spi)
		tegra_setup_spi();
	tegra_setup_w1();
	pm_power_off = tegra_system_power_off;
	tegra_setup_suspend();
	tegra_setup_reboot();
}

void tegra_board_nvodm_suspend(void)
{
	printk(KERN_INFO "pICS_%s",__func__);
	if (console_suspend_enabled)
		tegra_debug_port_suspend();
}

void tegra_board_nvodm_resume(void)
{
	printk(KERN_INFO "pICS_%s",__func__);
	if (console_suspend_enabled)
		tegra_debug_port_resume();
}
