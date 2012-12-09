/*
 * arch/arm/mach-tegra/board-olympus-panel.c
 *
 * Copyright (C) 2010 Google, Inc.
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

#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/i2c.h>
#include <linux/resource.h>
#include <linux/platform_device.h>
#include <linux/earlysuspend.h>
#include <linux/delay.h>
#include <linux/keyreset.h>
#include <linux/input.h>
#include <linux/nvhost.h>

#include <asm/mach-types.h>
#include <mach/clk.h>
#include <mach/irqs.h>
#include <mach/iomap.h>
#include <mach/dc.h>
#include <mach/fb.h>

#include "board.h"
#include "board-olympus.h"
#include "gpio-names.h"

#define STINGRAY_LVDS_SHDN_B	TEGRA_GPIO_PB2
#define STINGRAY_HDMI_5V_EN	TEGRA_GPIO_PC4
#define STINGRAY_HDMI_HPD	TEGRA_GPIO_PN7

/* Display Controller */
static struct resource olympus_disp1_resources[] = {
	{
		.name	= "irq",
		.start	= INT_DISPLAY_GENERAL,
		.end	= INT_DISPLAY_GENERAL,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.name	= "regs",
		.start	= TEGRA_DISPLAY_BASE,
		.end	= TEGRA_DISPLAY_BASE + TEGRA_DISPLAY_SIZE-1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.name	= "fbmem",
		/* .start and .end to be filled in later */
		.flags	= IORESOURCE_MEM,
	},
};

static struct resource olympus_disp2_resources[] = {
	{
		.name	= "irq",
		.start	= INT_DISPLAY_B_GENERAL,
		.end	= INT_DISPLAY_B_GENERAL,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.name	= "regs",
		.start	= TEGRA_DISPLAY2_BASE,
		.end	= TEGRA_DISPLAY2_BASE + TEGRA_DISPLAY2_SIZE-1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.name	= "fbmem",
		/* .start and .end to be filled in later */
		.flags	= IORESOURCE_MEM,
	},
	{
		.name	= "hdmi_regs",
		.start	= TEGRA_HDMI_BASE,
		.end	= TEGRA_HDMI_BASE + TEGRA_HDMI_SIZE-1,
		.flags	= IORESOURCE_MEM,
	},
};

static struct tegra_dc_mode olympus_panel_modes[] = {
	{
		.pclk = 38333333,
		.h_ref_to_sync = 4,
		.v_ref_to_sync = 1,
		.h_sync_width = 16,
		.v_sync_width = 1,
		.h_back_porch = 32,
		.v_back_porch = 1,
		.h_active = 540,
		.v_active = 960,
		.h_front_porch = 32,
		.v_front_porch = 2,
		.stereo_mode = 0,
	},
};

static struct tegra_fb_data olympus_fb_data = {
	.win		= 0,
	.xres		= 960,
	.yres		= 540,
	.bits_per_pixel	= -1,
	.flags		= TEGRA_FB_FLIP_ON_PROBE,
};

static struct tegra_dc_out olympus_disp1_out = {
	.type = TEGRA_DC_OUT_RGB,

	.align = TEGRA_DC_ALIGN_MSB,
	.order = TEGRA_DC_ORDER_RED_BLUE,
	.depth = 24,

	.height = 91, /* mm */
	.width = 51, /* mm */

	.modes = olympus_panel_modes,
	.n_modes = ARRAY_SIZE(olympus_panel_modes),

/*	.enable = olympus_panel_enable,
	.disable = olympus_panel_disable,*/
};

static struct tegra_dc_platform_data olympus_disp1_pdata = {
	.flags		= TEGRA_DC_FLAG_ENABLED,
	.emc_clk_rate	= 300000000,
	.default_out	= &olympus_disp1_out,
	.fb		= &olympus_fb_data,
};

static struct nvhost_device olympus_disp1_device = {
	.name		= "tegradc",
	.id		= 0,
	.resource	= olympus_disp1_resources,
	.num_resources	= ARRAY_SIZE(olympus_disp1_resources),
	.dev = {
		.platform_data = &olympus_disp1_pdata,
	},
};

static int olympus_hdmi_init(void)
{
	/*tegra_gpio_enable(STINGRAY_HDMI_5V_EN);
	gpio_request(STINGRAY_HDMI_5V_EN, "hdmi_5v_en");
	gpio_direction_output(STINGRAY_HDMI_5V_EN, 1);

	tegra_gpio_enable(STINGRAY_HDMI_HPD);
	gpio_request(STINGRAY_HDMI_HPD, "hdmi_hpd");
	gpio_direction_input(STINGRAY_HDMI_HPD);
*/

	return 0;
}

static struct tegra_dc_out olympus_disp2_out = {
	.type = TEGRA_DC_OUT_HDMI,
	.flags = TEGRA_DC_OUT_HOTPLUG_HIGH,

	.dcc_bus = 1,
	.hotplug_gpio = STINGRAY_HDMI_HPD,

	.align = TEGRA_DC_ALIGN_MSB,
	.order = TEGRA_DC_ORDER_RED_BLUE,
};

static struct tegra_fb_data olympus_disp2_fb_data = {
	.win		= 0,
	.xres		= 1280,
	.yres		= 720,
	.bits_per_pixel	= 32,
};

static struct tegra_dc_platform_data olympus_disp2_pdata = {
	.flags		= 0,
	.emc_clk_rate	= ULONG_MAX,
	.default_out	= &olympus_disp2_out,
	.fb		= &olympus_disp2_fb_data,
};

static struct nvhost_device olympus_disp2_device = {
	.name		= "tegradc",
	.id		= 1,
	.resource	= olympus_disp2_resources,
	.num_resources	= ARRAY_SIZE(olympus_disp2_resources),
	.dev = {
		.platform_data = &olympus_disp2_pdata,
	},
};

#ifdef CONFIG_HAS_EARLYSUSPEND
/* put early_suspend/late_resume handlers here for the display in order
 * to keep the code out of the display driver, keeping it closer to upstream
 */
struct early_suspend olympus_panel_early_suspender;

static void olympus_panel_early_suspend(struct early_suspend *h)
{
	if (num_registered_fb > 0)
		fb_blank(registered_fb[0], FB_BLANK_POWERDOWN);
}

static void olympus_panel_late_resume(struct early_suspend *h)
{
	if (num_registered_fb > 0)
		fb_blank(registered_fb[0], FB_BLANK_UNBLANK);
}
#endif

int __init olympus_panel_init(void)
{
	struct resource *res;

	olympus_hdmi_init();

#ifdef CONFIG_HAS_EARLYSUSPEND
	olympus_panel_early_suspender.suspend = olympus_panel_early_suspend;
	olympus_panel_early_suspender.resume = olympus_panel_late_resume;
	olympus_panel_early_suspender.level = EARLY_SUSPEND_LEVEL_DISABLE_FB;
	register_early_suspend(&olympus_panel_early_suspender);
#endif

	res = nvhost_get_resource_byname(&olympus_disp1_device,
		IORESOURCE_MEM, "fbmem");
	res->start = tegra_fb_start;
	res->end = tegra_fb_start + tegra_fb_size - 1;
/*
	res = nvhost_get_resource_byname(&olympus_disp2_device,
		IORESOURCE_MEM, "fbmem");
	res->start = tegra_fb2_start;
	res->end = tegra_fb2_start + tegra_fb2_size - 1;
*/
	tegra_move_framebuffer(tegra_fb_start, tegra_bootloader_fb_start,
		min(tegra_fb_size, tegra_bootloader_fb_size));

/*	nvhost_device_register(&olympus_disp1_device);
	return  nvhost_device_register(&olympus_disp2_device);*/
	return  nvhost_device_register(&olympus_disp1_device);

}

