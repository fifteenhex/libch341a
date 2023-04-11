/*
 * spiepaper.c
 *
 */

/*
 *
 */

#include <stdio.h>
#include <libch341a.h>
#include <spi_controller.h>
#include <gpio_controller.h>

#include <unistd.h>

#include "common.h"
#include "tux.h"

#define RESET	1
#define DC		2
#define BUSY	7

static int send_command(struct ch341a_handle *ch341a, struct gpio_controller *gpio, uint8_t *command, size_t len)
{
	gpio_controller_set_value(gpio, DC, 0);
	spi_controller_cs_assert(&ch341a_spi);
	int ret = spi_controller_write(&ch341a_spi, command, len, 0);
	spi_controller_cs_release(&ch341a_spi);

	return ret;
}

static int send_data(struct ch341a_handle *ch341a, struct gpio_controller *gpio, uint8_t *data, size_t len)
{
	gpio_controller_set_value(gpio, DC, 1);
	spi_controller_cs_assert(&ch341a_spi);
	int ret = spi_controller_write(&ch341a_spi, data, len, 0);
	spi_controller_cs_release(&ch341a_spi);

	return ret;
}

static int wait_not_busy(struct gpio_controller *gpio)
{
	while(gpio_controller_get_value(gpio, BUSY) == 0){
	}

	return 0;
}

#define PLANESZ ((400 * 300) / 8)

#define CMD_PANEL_SETTING		0x00
#define CMD_POWER_SETTING		0x01
#define CMD_POWER_ON			0x04
#define CMD_BOOSTER_SOFT_START	0x06

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

	for (int i = 0; i < 3; i++) {
		/* apply reset and set default value for DC */
		gpio_controller_set_value(&gpio, RESET, 0);
		usleep(100);
		/* release reset */
		gpio_controller_set_value(&gpio, RESET, 1);
	}

	sleep(1);

	{
		uint8_t booster_soft_start[] = { CMD_BOOSTER_SOFT_START, 0x17, 0x17, 0x17 };
		send_command(ch341a, &gpio, booster_soft_start, sizeof(booster_soft_start));
	}

	{
		uint8_t power_setting[] = { CMD_POWER_SETTING, 0x03, 0x00, 0x2b, 0x2b, 0x09 };
		send_command(ch341a, &gpio, power_setting, sizeof(power_setting));
	};

	{
		uint8_t power_on[] = { CMD_POWER_ON };
		send_command(ch341a, &gpio, power_on, sizeof(power_on));
	};

	/* wait for ready */
	wait_not_busy(&gpio);

	{
		uint8_t panel_setting[] = { CMD_PANEL_SETTING, 0x0f};
		send_command(ch341a, &gpio, panel_setting, sizeof(panel_setting));
	};

	{
		uint8_t resolution_setting[] = { 0x61, 0x01, 0x90, 0x01, 0x2c };
		send_command(ch341a, &gpio, resolution_setting, sizeof(resolution_setting));
	};

	{
		uint8_t vcom_datainterval[] = { 0x50, 0x77 };
		send_command(ch341a, &gpio, vcom_datainterval, sizeof(vcom_datainterval));
	};

	{
		uint8_t start_bw_data[] = { 0x10 };
		send_command(ch341a, &gpio, start_bw_data, sizeof(start_bw_data));
	};

	send_data(ch341a, &gpio, tux_bw, PLANESZ);

	{
		uint8_t start_yellow_data[] = { 0x13 };
		send_command(ch341a, &gpio, start_yellow_data, sizeof(start_yellow_data));
	};

	send_data(ch341a, &gpio, tux_y, PLANESZ);

	{
		uint8_t display_refresh[] = { 0x12 };
		send_command(ch341a, &gpio, display_refresh, sizeof(display_refresh));
	};

	wait_not_busy(&gpio);

	{
		uint8_t power_off[] = { 0x02 };
		send_command(ch341a, &gpio, power_off, sizeof(power_off));
	};

	// vcom and interval setting?

	{
		uint8_t deep_sleep[] = { 0x07, 0xa5 };
		send_command(ch341a, &gpio, deep_sleep, sizeof(deep_sleep));
	};

	spi_controller_shutdown(&ch341a_spi);

	return 0;
}

