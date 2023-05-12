/*
 * spiepaper.c
 *
 */

#include <stdio.h>
#include <libch341a.h>
#include <spi_controller.h>
#include <gpio_controller.h>

#include <unistd.h>

#include <libebogroll.h>
#include <gdew042c37.h>

#include "common.h"
#include "tux.h"

#define RESET	1
#define DC	2
#define BUSY	7

int main (int argc, char **argv)
{
	int ret;

	void *ch341a_mfd_priv;

	ret = libusrio_mfd_open(&ch341a_mfd, ch341a_log_cb, NULL,
			LIBUSRIO_MFD_WANTGPIO | LIBUSRIO_MFD_WANTSPI, &ch341a_mfd_priv);
	if (ret) {
		printf("Failed to open mfd: %d\n", ret);
		return ret;
	}

	const struct gpio_controller *gpio;
	ret = libusrio_mfd_get_gpio(&ch341a_mfd, ch341a_mfd_priv, &gpio);
	if (ret) {
		printf("Failed to open gpio: %d\n", ret);
		return ret;
	}

	const struct spi_controller *spi;
	ret = libusrio_mfd_get_spi(&ch341a_mfd, ch341a_mfd_priv, &spi);
	if (ret) {
		printf("Failed to open spi: %d\n", ret);
		return ret;
	}

	/* set initial state for DC */
	gpio_controller_set_value(gpio, ch341a_mfd_priv, DC, 0);

	struct gdew042c37_data display_data;
	gdew042c37_new(&display_data, gpio, ch341a_mfd_priv, spi, ch341a_mfd_priv, RESET, BUSY, DC);

	if (ebogroll_reset(&gdew042c37, &display_data)) {
		printf("failed to reset\n");
		return 1;
	}

	sleep(1);

	if(ebogroll_power_up(&gdew042c37, &display_data)) {
		printf("power up failed\n");
		return 1;
	}

	if (ebogroll_send_plane_data(&gdew042c37, &display_data, 0, tux_bw)) {
		printf("failed to send bw plane data\n");
		return 1;
	}

	if (ebogroll_send_plane_data(&gdew042c37, &display_data, 1, tux_y)) {
		printf("failed to send yellow plane data\n");
		return 1;
	}

	if (ebogroll_refresh(&gdew042c37, &display_data)) {
		printf("failed to trigger display refresh\n");
		return 1;
	}

	ebogroll_power_down(&gdew042c37, &display_data);

	libusrio_mfd_close(&ch341a_mfd, ch341a_mfd_priv);

	return 0;
}

