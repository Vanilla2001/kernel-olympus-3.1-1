#include <linux/init.h>
#include <linux/irq.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/major.h>
#include <linux/tcmd_driver.h>
#include <linux/mtd/partitions.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/setup.h>
#include <asm/bootinfo.h>

#include <mach/iomap.h>
#include <mach/mdm_ctrl.h>
/*#include <mach/sec_linux.h>*/
#include <mach/sdhci-simple.h>
#include <mach/nand.h>

#include "board.h"
#include "gpio-names.h"
#include "board-olympus.h"

#if 0
#if defined(CONFIG_APANIC_MMC) || defined(CONFIG_APANIC_RAM)
#include <mach/apanic.h>
#endif

#ifdef CONFIG_APANIC_MMC

static struct tegra_sdhci_simple_platform_data tegra_sdhci_simple_platform_data;
static struct platform_device tegra_sdhci_simple_device;

static struct apanic_mmc_platform_data apanic_mmc_platform_data;

static struct platform_device apanic_handle_mmc_platform_device = {
	.name          = "apanic_handle_mmc",
	.id            = 0,
	.dev =
	{
		.platform_data = &apanic_mmc_platform_data,
	}
};

extern struct tegra_nand_platform tegra_nand_plat;
extern int tegra_sdhci_boot_device;
extern struct platform_device tegra_sdhci_devices[];

int apanic_mmc_init(void)
{
	int i;
	int result = -ENOMEM;

	/*
	 * This is a little convoluted, but the simple driver needs to map the
	 * I/O port and access other resources in order to use the hardware.
	 * It can't do it through the normal means or else the kernel will try
	 * to claim the same resources as the real sdhci-tegra driver at boot.
	 */
	if (tegra_sdhci_boot_device >= 0) {
		tegra_sdhci_simple_platform_data.sdhci_pdata =
			tegra_sdhci_devices[tegra_sdhci_boot_device].dev.platform_data;
		tegra_sdhci_simple_platform_data.resource =
			tegra_sdhci_devices[tegra_sdhci_boot_device].resource;
		tegra_sdhci_simple_platform_data.num_resources =
			tegra_sdhci_devices[tegra_sdhci_boot_device].num_resources;
		tegra_sdhci_simple_platform_data.clk_dev_name =
			kasprintf(GFP_KERNEL, "tegra-sdhci.%d", tegra_sdhci_boot_device);

		tegra_sdhci_simple_device.id = tegra_sdhci_boot_device;
		tegra_sdhci_simple_device.name = "tegra-sdhci-simple";
		tegra_sdhci_simple_device.dev.platform_data =
				&tegra_sdhci_simple_platform_data;

		result = platform_device_register(&tegra_sdhci_simple_device);
	}

	/*
	 * FIXME: There is no way to "discover" the kpanic partition because
	 * it has no file system and the legacy MBR/EBR tables do not support
	 * labels.  GPT promises to address this in K35 or later.
	 */
	if (result == 0) {
		apanic_mmc_platform_data.id = 0;  /* mmc0 - not used by sdhci-tegra-simple */
		for (i = 0; i < tegra_nand_plat.nr_parts; i++) {
			if (strcmp(CONFIG_APANIC_PLABEL, tegra_nand_plat.parts[i].name))
				continue;
			apanic_mmc_platform_data.sector_size = 512;  /* fixme */
			apanic_mmc_platform_data.start_sector =
					tegra_nand_plat.parts[i].offset / 512;
			apanic_mmc_platform_data.sectors =
					tegra_nand_plat.parts[i].size / 512;
			break;
		}

		result = platform_device_register(&apanic_handle_mmc_platform_device);
	}

	return result;
}
#endif
#endif

/* NVidia bootloader tags and parsing routines */
#define ATAG_NVIDIA		0x41000801

#define ATAG_NVIDIA_RM			0x1
#define ATAG_NVIDIA_DISPLAY		0x2
#define ATAG_NVIDIA_FRAMEBUFFER		0x3
#define ATAG_NVIDIA_CHIPSHMOO		0x4
#define ATAG_NVIDIA_CHIPSHMOOPHYS	0x5
#define ATAG_NVIDIA_CARVEOUT		0x6
#define ATAG_NVIDIA_WARMBOOT		0x7

#define ATAG_NVIDIA_PRESERVED_MEM_0	0x10000
#define ATAG_NVIDIA_PRESERVED_MEM_N	3
#define ATAG_NVIDIA_FORCE_32		0x7fffffff

/**
 * Resource Manager boot args.
 *
 * Nothing here yet.
 */
struct NVBOOTARGS_Rm
{
	u32 	reserved;
};

/**
 * Carveout boot args, which define the physical memory location of the GPU
 * carved-out memory region(s).
 */
struct NVBOOTARGS_Carveout
{
	void* 	base;
	u32 	size;
};

/**
 * Warmbootloader boot args. This structure only contains
 * a mem handle key to preserve the warm bootloader
 * across the bootloader->os transition
 */
struct NVBOOTARGS_Warmboot
{
	/* The key used for accessing the preserved memory handle */
	u32 	MemHandleKey;
};

/**
 * PreservedMemHandle boot args, indexed by ATAG_NVIDIA_PRESERVED_MEM_0 + n.
 * This allows physical memory allocations (e.g., for framebuffers) to persist
 * between the bootloader and operating system.  Only carveout and IRAM
 * allocations may be preserved with this interface.
 */
struct NVBOOTARGS_PreservedMemHandle
{
	u32 	Address;
	u32   	Size;
};

/**
 * Display boot args.
 *
 * The bootloader may have a splash screen. This will flag which controller
 * and device was used for the splash screen so the device will not be
 * reinitialized (which causes visual artifacts).
 */
struct NVBOOTARGS_Display
{
	/* which controller is initialized */
	u32 	Controller;

	/* index into the ODM device list of the boot display device */
	u32 	DisplayDeviceIndex;

	/* set to != 0 if the display has been initialized */
	u8 		bEnabled;
};

/**
 * Framebuffer boot args
 *
 * A framebuffer may be shared between the bootloader and the
 * operating system display driver.  When this key is present,
 * a preserved memory handle for the framebuffer must also
 * be present, to ensure that no display corruption occurs
 * during the transition.
 */
struct NVBOOTARGS_Framebuffer
{
	/*  The key used for accessing the preserved memory handle */
	u32 	MemHandleKey;
	/*  Total memory size of the framebuffer */
	u32 	Size;
	/*  Color format of the framebuffer, cast to a U32  */
	u32 	ColorFormat;
	/*  Width of the framebuffer, in pixels  */
	u16 	Width;
	/*  Height of each surface in the framebuffer, in pixels  */
	u16 	Height;
	/*  Pitch of a framebuffer scanline, in bytes  */
	u16 	Pitch;
	/*  Surface layout of the framebuffer, cast to a U8 */
	u8  	SurfaceLayout;
	/*  Number of contiguous surfaces of the same height in the
	    framebuffer, if multi-buffering.  Each surface is
	    assumed to begin at Pitch * Height bytes from the
	    previous surface.  */
	u8  	NumSurfaces;
	/* Flags for future expandability.
	   Current allowable flags are:
	   zero - default
	   NV_BOOT_ARGS_FB_FLAG_TEARING_EFFECT - use a tearing effect signal in
	   combination with a trigger from the display software to generate
	   a frame of pixels for the display device. */
	u32 	Flags;
#define NVBOOTARG_FB_FLAG_TEARING_EFFECT (0x1)

};

/**
 * Chip characterization shmoo data
 */
struct NVBOOTARGS_ChipShmoo
{
	/* The key used for accessing the preserved memory handle of packed
	   characterization tables  */
	u32 	MemHandleKey;

	/* Offset and size of each unit in the packed buffer */
	u32 	CoreShmooVoltagesListOffset;
	u32 	CoreShmooVoltagesListSize;

	u32 	CoreScaledLimitsListOffset;
	u32 	CoreScaledLimitsListSize;

	u32 	OscDoublerListOffset;
	u32 	OscDoublerListSize;

	u32 	SKUedLimitsOffset;
	u32 	SKUedLimitsSize;

	u32 	CpuShmooVoltagesListOffset;
	u32 	CpuShmooVoltagesListSize;

	u32 	CpuScaledLimitsOffset;
	u32 	CpuScaledLimitsSize;

	/* Misc characterization settings */
	u16 	CoreCorner;
	u16 	CpuCorner;
	u32 	Dqsib;
	u32 	SvopLowVoltage;
	u32 	SvopLowSetting;
	u32 	SvopHighSetting;
};

/**
 * Chip characterization shmoo data indexed by NvBootArgKey_ChipShmooPhys
 */
struct NVBOOTARGS_ChipShmooPhys
{
	u32 	PhysShmooPtr;
	u32 	Size;
};


/**
 * OS-agnostic bootarg structure.
 */
struct NVBOOTARGS
{
	struct NVBOOTARGS_Rm 					RmArgs;
	struct NVBOOTARGS_Display 				DisplayArgs;
	struct NVBOOTARGS_Framebuffer 			FramebufferArgs;
	struct NVBOOTARGS_ChipShmoo 			ChipShmooArgs;
	struct NVBOOTARGS_ChipShmooPhys			ChipShmooPhysArgs;
	struct NVBOOTARGS_Warmboot 				WarmbootArgs;
	struct NVBOOTARGS_PreservedMemHandle 	MemHandleArgs[ATAG_NVIDIA_PRESERVED_MEM_N];
};

static __initdata struct NVBOOTARGS NvBootArgs = { {0}, {0}, {0}, {0}, {0}, {0}, {{0}} }; 

static int __init get_cfg_from_tags(void)
{
	/* If the bootloader framebuffer is found, use it */
	if (tegra_bootloader_fb_start == 0 && tegra_bootloader_fb_size == 0 &&
			NvBootArgs.FramebufferArgs.MemHandleKey >= ATAG_NVIDIA_PRESERVED_MEM_0 &&
			NvBootArgs.FramebufferArgs.MemHandleKey <  (ATAG_NVIDIA_PRESERVED_MEM_0+ATAG_NVIDIA_PRESERVED_MEM_N) &&
			NvBootArgs.FramebufferArgs.Size != 0 &&
			NvBootArgs.MemHandleArgs[NvBootArgs.FramebufferArgs.MemHandleKey - ATAG_NVIDIA_PRESERVED_MEM_0].Size != 0) 
	{
		/* Got the bootloader framebuffer address and size. Store it */
		tegra_bootloader_fb_start = NvBootArgs.MemHandleArgs[NvBootArgs.FramebufferArgs.MemHandleKey - ATAG_NVIDIA_PRESERVED_MEM_0].Address;
		tegra_bootloader_fb_size  = NvBootArgs.MemHandleArgs[NvBootArgs.FramebufferArgs.MemHandleKey - ATAG_NVIDIA_PRESERVED_MEM_0].Size;

		pr_info("Nvidia ATAG: framebuffer: %lu @ 0x%08lx\n",tegra_bootloader_fb_size,tegra_bootloader_fb_start);
	}

	/* If the LP0 vector is found, use it */
	if (tegra_lp0_vec_start == 0 && tegra_lp0_vec_size == 0 &&
			NvBootArgs.WarmbootArgs.MemHandleKey >= ATAG_NVIDIA_PRESERVED_MEM_0 &&
			NvBootArgs.WarmbootArgs.MemHandleKey <  (ATAG_NVIDIA_PRESERVED_MEM_0+ATAG_NVIDIA_PRESERVED_MEM_N) &&
			NvBootArgs.MemHandleArgs[NvBootArgs.WarmbootArgs.MemHandleKey - ATAG_NVIDIA_PRESERVED_MEM_0].Size != 0) 
	{
		/* Got the Warmboot block address and size. Store it */
		tegra_lp0_vec_start = NvBootArgs.MemHandleArgs[NvBootArgs.WarmbootArgs.MemHandleKey - ATAG_NVIDIA_PRESERVED_MEM_0].Address;
		tegra_lp0_vec_size  = NvBootArgs.MemHandleArgs[NvBootArgs.WarmbootArgs.MemHandleKey - ATAG_NVIDIA_PRESERVED_MEM_0].Size;

		pr_info("Nvidia ATAG: LP0: %lu @ 0x%08lx\n",tegra_lp0_vec_size,tegra_lp0_vec_start);

	}

	return 0;
}

static int __init parse_tag_nvidia(const struct tag *tag)
{
	const char *addr = (const char *)&tag->hdr + sizeof(struct tag_header);
	const struct tag_nvidia_tegra *nvtag = (const struct tag_nvidia_tegra*)addr;

	if (nvtag->bootarg_key >= ATAG_NVIDIA_PRESERVED_MEM_0 &&
			nvtag->bootarg_key <  (ATAG_NVIDIA_PRESERVED_MEM_0+ATAG_NVIDIA_PRESERVED_MEM_N) )
	{
		int Index = nvtag->bootarg_key - ATAG_NVIDIA_PRESERVED_MEM_0;

		struct NVBOOTARGS_PreservedMemHandle *dst = 
			&NvBootArgs.MemHandleArgs[Index];
		const struct NVBOOTARGS_PreservedMemHandle *src = 
			(const struct NVBOOTARGS_PreservedMemHandle *) nvtag->bootarg;

		if (nvtag->bootarg_len != sizeof(*dst)) {
			pr_err("Unexpected preserved memory handle tag length (expected: %d, got: %d!\n",
					sizeof(*dst), nvtag->bootarg_len);
		} else {

			pr_debug("Preserved memhandle: 0x%08x, address: 0x%08x, size: %d\n",
					nvtag->bootarg_key, src->Address, src->Size);

			memcpy(dst,src,sizeof(*dst));
		}
		return get_cfg_from_tags();
	}

	switch (nvtag->bootarg_key) {
		case ATAG_NVIDIA_FRAMEBUFFER:
			{
				struct NVBOOTARGS_Framebuffer *dst = 
					&NvBootArgs.FramebufferArgs;
				const struct NVBOOTARGS_Framebuffer *src = 
					(const struct NVBOOTARGS_Framebuffer *)nvtag->bootarg;

				if (nvtag->bootarg_len != sizeof(*dst)) {
					pr_err("Unexpected framebuffer tag length (expected: %d, got: %d!\n",
							sizeof(*dst), nvtag->bootarg_len);
				} else {
					pr_debug("Framebuffer tag with 0x%08x handle, size: %d\n",
							src->MemHandleKey,src->Size);
					memcpy(dst,src,sizeof(*dst));
				}
				return get_cfg_from_tags();
			}
		case ATAG_NVIDIA_WARMBOOT:
			{
				struct NVBOOTARGS_Warmboot *dst = 
					&NvBootArgs.WarmbootArgs;
				const struct NVBOOTARGS_Warmboot *src =
					(const struct NVBOOTARGS_Warmboot *)nvtag->bootarg;

				if (nvtag->bootarg_len != sizeof(*dst)) {
					pr_err("Unexpected Warnboot tag length (expected: %d, got: %d!\n",
							sizeof(*dst), nvtag->bootarg_len);
				} else {
					pr_debug("Found a warmboot tag with handle 0x%08x!\n", src->MemHandleKey);
					memcpy(dst,src,sizeof(*dst));
				}
				return get_cfg_from_tags();
			}

		default:
			pr_info("Ignoring irrelevant nvidia tag 0x%04x!\n", nvtag->bootarg_key);
			break;
	} 
	return 0;
}
__tagtable(ATAG_NVIDIA, parse_tag_nvidia);

#if 0
/*
 * Security
 */

static struct sec_platform_data mot_sec_platform_data;
static struct platform_device mot_sec_platform_device = {
    .name          = "sec",
    .id            = 0,
    .dev =
    {
        .platform_data = &mot_sec_platform_data,
    }
};

void mot_sec_init(void)
{
    	platform_device_register(&mot_sec_platform_device);
}

#endif
/*
 * TCMD
 */
static struct tcmd_driver_platform_data mot_tcmd_platform_data;
static struct platform_device mot_tcmd_platform_device = {
    .name          = "tcmd_driver",
    .id            = 0,
    .dev = {
		.platform_data = &mot_tcmd_platform_data,
    }
};

void mot_tcmd_init(void)
{
	mot_tcmd_platform_data.gpio_list[TCMD_GPIO_ISL29030_INT]
      = TEGRA_GPIO_PE1;
	mot_tcmd_platform_data.gpio_list[TCMD_GPIO_KXTF9_INT]
      = TEGRA_GPIO_PV3;
	mot_tcmd_platform_data.gpio_list[TCMD_GPIO_MMC_DETECT]
      = TEGRA_GPIO_PI5;
	mot_tcmd_platform_data.gpio_list[TCMD_GPIO_INT_MAX_NUM]
      = -1;
	mot_tcmd_platform_data.size = TCMD_GPIO_INT_MAX_NUM;

	platform_device_register(&mot_tcmd_platform_device);
}

/*
 * Some global queries for the framebuffer, display, and backlight drivers.
 */
static unsigned int s_MotorolaDispInfo = 0;
static unsigned int s_MotorolaFBInfo = 1;

unsigned short bootloader_ver_major = 0;
unsigned short bootloader_ver_minor = 0;
unsigned short uboot_ver_major = 0;
unsigned short uboot_ver_minor = 0;

unsigned char lpddr2_mr[12];

int MotorolaBootFBArgGet(unsigned int *arg)
{
    *arg = s_MotorolaFBInfo;
    return 0;
}

int MotorolaBootDispArgGet(unsigned int *arg)
{
    if(s_MotorolaDispInfo)
    {
        *arg = s_MotorolaDispInfo;
        return 0;
    }

    return -1;
}


/*
 * Parse the Motorola-specific ATAG
 */
static int __init parse_tag_motorola(const struct tag *tag)
{
    const struct tag_motorola *moto_tag = &tag->u.motorola;
    int i = 0;

    s_MotorolaDispInfo = moto_tag->panel_size;
    printk(KERN_INFO "pICS_%s",__func__);
	
    s_MotorolaFBInfo = moto_tag->allow_fb_open;

/*    #ifdef CONFIG_MOT_ALLOW_FB_OPEN*/
    s_MotorolaFBInfo = 1; // allow fb open for charger and bootmenu (minui)
/*    #endif*/
 
/*    mot_sec_platform_data.fl_factory = moto_tag->in_factory;*/

    bootloader_ver_major = moto_tag->bl_ver_major;
    bootloader_ver_minor = moto_tag->bl_ver_minor;
    uboot_ver_major = moto_tag->uboot_ver_major;
    uboot_ver_minor = moto_tag->uboot_ver_minor;
    bi_set_cid_recover_boot(moto_tag->cid_suspend_boot);

    pr_info("%s: bootloader v%d.%d\n", __func__, bootloader_ver_major, bootloader_ver_minor);
    pr_info("%s: uboot v%d.%d\n", __func__, uboot_ver_major, uboot_ver_minor);
    pr_info("%s: panel_size: 0x%x\n", __func__, s_MotorolaDispInfo);
    pr_info("%s: allow_fb_open: %x\n", __func__, s_MotorolaFBInfo);
/*    pr_info("%s: factory: %d\n", __func__, mot_sec_platform_data.fl_factory);*/
    pr_info("%s: cid_suspend_boot: %u\n", __func__, (unsigned)moto_tag->cid_suspend_boot);

    /*
     *	Dump memory information
     */
     /* FIXME:  Add eMMC support */
	for (i = 0; i < 12; i++) {
		lpddr2_mr[i] = moto_tag->at_lpddr2_mr[i];
		pr_info("%s: LPDDR2 MR%d:     0x%04X (0x%04X)\n", __func__, i,
			lpddr2_mr[i],
			moto_tag->at_lpddr2_mr[i]);
	}

    return 0;
}
__tagtable(ATAG_MOTOROLA, parse_tag_motorola);

/*
 * Parse the Motorola boot loader ATAG 41000811
 */
static int __init parse_tag_bldebug(const struct tag *t)
{
	pr_info("%s: powerup reason regs: INTS1=0x%4.4x INT2=0x%4.4x INTS2=0x%4.4x INT3=0x%4.4x "
		"PC2=0x%4.4x MEMA=0x%4.4x ACCY=%d UBOOT=%d\n", __func__, t->u.bldebug.ints1,
		t->u.bldebug.int2, t->u.bldebug.ints2, t->u.bldebug.int3, t->u.bldebug.pc2,
		t->u.bldebug.mema, t->u.bldebug.accy, t->u.bldebug.uboot);
	return 0;
}
__tagtable(ATAG_BLDEBUG, parse_tag_bldebug);

/*
 *   UTS tool needs to emulate numbers keys and send/end keys. Add them to tegra kbc driver keymap 
 */
#if 0
static int keymap_update_connect(struct input_handler *handler, struct input_dev *dev,
					  const struct input_device_id *id)
 {
	if (strcmp(dev->name , "tegra-kbc"))  return 0;

	set_bit(0x38, dev->keybit); // ALT
	set_bit(0x3E, dev->keybit); // CALL
	set_bit(0x3D, dev->keybit); // ENDCALL
	set_bit(0xE7, dev->keybit); // SEND
	set_bit(0x6A, dev->keybit); // DPAD_UP
	set_bit(0x69, dev->keybit); // DPAD_DOWN
	set_bit(0x67, dev->keybit); // DPAD_LEFT
	set_bit(0x6C, dev->keybit); // DPAD_RIGHT
	set_bit(0xE8, dev->keybit); // DPAD_CENTER
	set_bit(0xA2, dev->keybit); // Power


	pr_info("keymap_update add key codes to device %s\n", dev->name);
	return 0;
 }

static void keymap_update_disconnect(struct input_handle *handle)  {}
static void keymap_update_event(struct input_handle *handle, unsigned int type, unsigned int code, int value) {}

static const struct input_device_id keymap_update_ids[] = {
	{
		.flags = INPUT_DEVICE_ID_MATCH_EVBIT,
		.evbit = { BIT_MASK(EV_KEY) },
	},
	{ },
};

MODULE_DEVICE_TABLE(input, keymap_update_ids);

static struct input_handler keymap_update_handler = {
	.event      = keymap_update_event,
	.connect    = keymap_update_connect,
	.disconnect = keymap_update_disconnect,
	.name       = "keymap_update",
	.id_table   = keymap_update_ids,
};


void mot_keymap_update_init(void)
{
	input_register_handler(&keymap_update_handler);
}
#endif
