//SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 */

#include <argtable2.h>
#include <stdio.h>
#include <string.h>
#include <libch341a.h>
#include <spi_controller.h>
#include <gpio_controller.h>

#include <unistd.h>

#include "common.h"

extern int ch341a_enable_pins(struct ch341a_handle *ch341a, bool enable);

int main (int argc, char **argv)
{
	int ret = 0;

	struct arg_lit *help, *version;
	struct arg_rex *lines;
	struct arg_end *end;

	void *argtable[] = {
			/* help */
			help = arg_lit0("h", "help", "Display this message and exit"),
			version = arg_lit0("v", "version", "Display the version and exit"),
			lines = arg_rexn(NULL, NULL, "[0-9]*=[0-1]", "<offsetN>=<valueN>", 0, 64, 0, "Line number/value pairs"),
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

	struct line_value_pair {
		int line;
		int value;
	};
	unsigned int num_pairs = 0;

	struct line_value_pair *line_value_pairs;
	if (lines->count == 0) {
		printf("need some line/value pairs\n");
		return 1;
	}
	else {
		num_pairs = lines->count;
		line_value_pairs = malloc(sizeof(*line_value_pairs) * num_pairs);
		if (!line_value_pairs)
			return 1;

		for (int i = 0; i < lines->count; i++) {
			char *l = lines->sval[i];
			char *v = strchr(l, '=');

			*v = '\0';
			v++;

			line_value_pairs[i].line = (int) strtol(l, NULL, 0);
			line_value_pairs[i].value = (int) strtol(v, NULL, 0);

			if(line_value_pairs[i].line < 0 || line_value_pairs[i].line > 5) {
				printf("Cannot set line: %d\n", line_value_pairs[i].line);
				return 1;
			}

		}
	}

	void *ch341a_mfd_priv;
	struct gpio_controller_info *gpio_info;

	ret = libusrio_mfd_open(&ch341a_mfd, ch341a_log_cb, NULL,
			LIBUSRIO_MFD_WANTGPIO, &ch341a_mfd_priv);
	if (ret) {
		printf("Failed to open mfd: %d\n", ret);
		return ret;
	}

	// fixme: Should just make the pin that is needed an output
	ch341a_enable_pins(ch341a_mfd_priv, true);

	const struct gpio_controller *gpio;
	libusrio_mfd_get_gpio(&ch341a_mfd, ch341a_mfd_priv, &gpio);

	for (int i = 0; i < num_pairs; i++) {
		struct line_value_pair *lv = &line_value_pairs[i];
		ret = gpio_controller_set_value(gpio, ch341a_mfd_priv,
				lv->line, lv->value);
		if (ret)
			printf("Failed to set gpio value: %d\n", ret);
	}

	//sleep(10);

out:
	libusrio_mfd_close(&ch341a_mfd, ch341a_mfd_priv);

	return 0;
}
