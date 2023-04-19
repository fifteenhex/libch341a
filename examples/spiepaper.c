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
#define DC		2
#define BUSY	7

int main (int argc, char **argv)
{
	int ret;

	ret = spi_controller_init(&ch341a_spi, ch341a_log_cb, NULL);
	if (ret)
		return ret;

	struct ch341a_handle *ch341a = spi_controller_get_client_data(&ch341a_spi);
	struct gpio_controller gpio;
	ch341a_gpio_from_handle(ch341a, &gpio);

	/* set initial state for DC */
	gpio_controller_set_value(&gpio, DC, 0);

	struct gdew042c37_data display_data;
	gdew042c37_new(&display_data, &gpio, &ch341a_spi, RESET, BUSY, DC);

	ebogroll_reset(&gdew042c37, &display_data);
	sleep(1);
	ebogroll_power_up(&gdew042c37, &display_data);
	ebogroll_send_plane_data(&gdew042c37, &display_data, 0, tux_bw);
	ebogroll_send_plane_data(&gdew042c37, &display_data, 1, tux_y);
	ebogroll_refresh(&gdew042c37, &display_data);
	ebogroll_power_down(&gdew042c37, &display_data);

	spi_controller_shutdown(&ch341a_spi);

	return 0;
}

