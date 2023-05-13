//SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 */

#include <stdio.h>
#include <libch341a.h>
#include <spi_controller.h>
#include <gpio_controller.h>

#include "common.h"

int main (int argc, char **argv)
{
	int ret;

	void *ch341a_mfd_priv;
	struct gpio_controller_info *gpio_info;

	ret = libusrio_mfd_open(&ch341a_mfd, ch341a_log_cb, NULL,
			LIBUSRIO_MFD_WANTGPIO, &ch341a_mfd_priv);
	if (ret) {
		printf("Failed to open mfd: %d\n", ret);
		return ret;
	}

	const struct gpio_controller *gpio;
	libusrio_mfd_get_gpio(&ch341a_mfd, ch341a_mfd_priv, &gpio);

	ret = libusrio_gpio_controller_get_info(gpio, ch341a_mfd_priv, &gpio_info);
	if (ret) {
		printf("Failed to get gpio info\n");
		return ret;
	}

	printf("xxx chip - %d lines:\n", gpio_info->num_lines);
	for (int i = 0; i < gpio_info->num_lines; i++) {
		printf("line\t%d:\n", gpio_info->lines[i].num);
	}

	libusrio_mfd_close(&ch341a_mfd, ch341a_mfd_priv);

	return 0;
}
