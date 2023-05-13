//SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 */

#include <argtable2.h>
#include <stdio.h>
#include <libch341a.h>
#include <spi_controller.h>
#include <gpio_controller.h>

#include "common.h"

int main (int argc, char **argv)
{
	int ret = 0;

	struct arg_lit *help, *version;
	struct arg_end *end;

	void *argtable[] = {
			/* help */
			help = arg_lit0("h", "help", "Display this message and exit"),
			version = arg_lit0("v", "version", "Display the version and exit"),
			end = arg_end(1),
	};

	ret = arg_parse(argc, argv, argtable);
	if (ret) {
		arg_print_errors(stdout, end, "xxx");
		return -EINVAL;
	}

	if (help->count > 0) {
		arg_print_syntax(stdout, argtable, "\n");
		arg_print_glossary(stdout, argtable, "  %-30s %s\n");
		return 0;
	}

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

	libusrio_mfd_close(&ch341a_mfd, ch341a_mfd_priv);

	return 0;
}
