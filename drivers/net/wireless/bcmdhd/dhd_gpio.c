
#include <osl.h>
#include <dngl_stats.h>
#include <dhd.h>
#include <linux/gpio.h>

#ifdef CONFIG_MACH_ODROID_4210
#include <mach/gpio.h>
#include <mach/regs-gpio.h>
#include <plat/gpio-cfg.h>
#include <plat/sdhci.h>
#include <plat/devs.h>
#define	sdmmc_channel	s3c_device_hsmmc0
#endif
#ifdef CONFIG_MACH_NUC970
#include <mach/gpio.h>
#include <mach/map.h>
#include <mach/regs-clock.h>
#include <mach/regs-gcr.h>
#include <mach/regs-gpio.h>


#define WL_HOST_WAKE_DEF_GPIO NUC970_PB2 //GPB2
extern void nuc970_rescan_card(unsigned id, unsigned insert);
//extern void wifi_pm_power(int on);
#endif

struct wifi_platform_data dhd_wlan_control = {0};
int wl_host_wake_irqno = -1;
int wl_host_wake = -1;


#ifdef CUSTOMER_OOB
uint bcm_wlan_get_oob_irq(void)
{
	uint host_oob_irq = 0;
	if (gpio_request(WL_HOST_WAKE_DEF_GPIO, "wl_host_wake")) {
		gpio_free(WL_HOST_WAKE_DEF_GPIO);
		if (gpio_request(WL_HOST_WAKE_DEF_GPIO, "wl_host_wake"))
			{
				pr_warning("[%s] get wl_host_wake gpio failed\n", __FUNCTION__);

				wl_host_wake = -1;
				return -1;
			}
	}
	wl_host_wake = WL_HOST_WAKE_DEF_GPIO;
	gpio_direction_input(wl_host_wake);
	wl_host_wake_irqno = gpio_to_irq(wl_host_wake);
	pr_info("bcmdhd: got gpio%d, mapped to irqno%d\n", wl_host_wake, wl_host_wake_irqno);
	
	host_oob_irq = wl_host_wake_irqno;
#ifdef CONFIG_MACH_ODROID_4210
	printk("GPIO(WL_HOST_WAKE) = EXYNOS4_GPX0(7) = %d\n", EXYNOS4_GPX0(7));
	host_oob_irq = gpio_to_irq(EXYNOS4_GPX0(7));
	gpio_direction_input(EXYNOS4_GPX0(7));
#endif
	printk("host_oob_irq: %d \r\n", host_oob_irq);

	return host_oob_irq;
}

uint bcm_wlan_get_oob_irq_flags(void)
{
	uint host_oob_irq_flags = 0;
    host_oob_irq_flags = IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL | IORESOURCE_IRQ_SHAREABLE;
#ifdef CONFIG_MACH_ODROID_4210
#ifdef HW_OOB
	host_oob_irq_flags = IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL | IORESOURCE_IRQ_SHAREABLE;
#else
	host_oob_irq_flags = IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHEDGE | IORESOURCE_IRQ_SHAREABLE;
#endif
#endif
	printk("host_oob_irq_flags=%d\n", host_oob_irq_flags);

	return host_oob_irq_flags;
}
#endif

int bcm_wlan_set_power(bool on)
{
	int err = 0;

	if (on) {
		printk("======== PULL WL_REG_ON HIGH! ========\n");
#ifdef CONFIG_MACH_ODROID_4210
		err = gpio_set_value(EXYNOS4_GPK1(0), 1);
#endif
		/* Lets customer power to get stable */
        //wifi_pm_power(1);
		mdelay(100);
	} else {
		printk("======== PULL WL_REG_ON LOW! ========\n");
        //wifi_pm_power(0);
#ifdef CONFIG_MACH_ODROID_4210
		err = gpio_set_value(EXYNOS4_GPK1(0), 0);
#endif
	}

	return err;
}

int bcm_wlan_set_carddetect(bool present)
{
	int err = 0;

	if (present) {
		printk("======== Card detection to detect SDIO card! ========\n");
#ifdef CONFIG_MACH_ODROID_4210
		err = sdhci_s3c_force_presence_change(&sdmmc_channel, 1);
#endif
       // sunximmc_rescan_card(3, 1);
	nuc970_rescan_card(0,1);
	} else {
		printk("======== Card detection to remove SDIO card! ========\n");
#ifdef CONFIG_MACH_ODROID_4210
		err = sdhci_s3c_force_presence_change(&sdmmc_channel, 0);
#endif
        //sunximmc_rescan_card(3, 0);
	nuc970_rescan_card(0,0);
	}

	return err;
}

int bcm_wlan_get_mac_address(unsigned char *buf)
{
	int err = 0;
	
	printk("======== %s ========\n", __FUNCTION__);
#ifdef EXAMPLE_GET_MAC
	/* EXAMPLE code */
	{
		struct ether_addr ea_example = {{0x00, 0x11, 0x22, 0x33, 0x44, 0xFF}};
		bcopy((char *)&ea_example, buf, sizeof(struct ether_addr));
	}
#endif /* EXAMPLE_GET_MAC */

	return err;
}

#ifdef CONFIG_DHD_USE_STATIC_BUF
extern void *bcmdhd_mem_prealloc(int section, unsigned long size);
void* bcm_wlan_prealloc(int section, unsigned long size)
{
	void *alloc_ptr = NULL;
	alloc_ptr = bcmdhd_mem_prealloc(section, size);
	if (alloc_ptr) {
		printk("success alloc section %d, size %ld\n", section, size);
		if (size != 0L)
			bzero(alloc_ptr, size);
		return alloc_ptr;
	}
	printk("can't alloc section %d\n", section);
	return NULL;
}
#endif

int bcm_wlan_set_plat_data(void) {
	printk("======== %s ========\n", __FUNCTION__);
	dhd_wlan_control.set_power = bcm_wlan_set_power;
	dhd_wlan_control.set_carddetect = bcm_wlan_set_carddetect;
	dhd_wlan_control.get_mac_addr = bcm_wlan_get_mac_address;
#ifdef CONFIG_DHD_USE_STATIC_BUF
	dhd_wlan_control.mem_prealloc = bcm_wlan_prealloc;
#endif
	return 0;
}

