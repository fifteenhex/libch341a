//SPDX-License-Identifier: GPL-2.0-or-later
/*
 * This file was, in the distant past, part of the flashrom project.
 *
 * Copyright (C) 2011 asbokid <ballymunboy@gmail.com>
 * Copyright (C) 2014 Pluto Yang <yangyj.ee@gmail.com>
 * Copyright (C) 2015-2016 Stefan Tauner
 * Copyright (C) 2015 Urja Rannikko <urjaman@gmail.com>
 * Copyright (C) 2018-2021 McMCC <mcmcc@mail.ru>
 */

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include <spi_controller.h>
#include <dgputil.h>

#include "ch341a.h"
#include "ch341a_spi.h"
#include "ch341a_gpio.h"

#include "ch341a_spi_log.h"

static struct spi_client ch341a_client;

/* ch341 requires LSB first, swap the bit order before send and after receive */
static uint8_t swap_byte(uint8_t x)
{
	x = ((x >> 1) & 0x55) | ((x << 1) & 0xaa);
	x = ((x >> 2) & 0x33) | ((x << 2) & 0xcc);
	x = ((x >> 4) & 0x0f) | ((x << 4) & 0xf0);
	return x;
}

#define PREAMBLE(_sc) \
	struct ch341a_handle *ch341a; \
	ch341a = spi_controller_get_client_data(_sc); \
	assert(ch341a)

static int ch341a_spi_send_command(const struct spi_controller *spi_controller,
		unsigned int writecnt, unsigned int readcnt, const unsigned char *writearr, unsigned char *readarr)
{
	PREAMBLE(spi_controller);

	int32_t ret = 0;

	/* How many packets ... */
	const size_t packets = (writecnt + readcnt + CH341_PACKET_LENGTH - 2) / (CH341_PACKET_LENGTH - 1);

	/* We pluck CS/timeout handling into the first packet thus we need to allocate one extra package. */
	uint8_t wbuf[packets+1][CH341_PACKET_LENGTH];
	uint8_t rbuf[writecnt + readcnt];
	/* Initialize the write buffer to zero to prevent writing random stack contents to device. */
	memset(wbuf[0], 0, CH341_PACKET_LENGTH);

	uint8_t *ptr = wbuf[0];
	/* CS usage is optimized by doing both transitions in one packet.
	 * Final transition to deselected state is in the pin disable. */
	unsigned int write_left = writecnt;
	unsigned int read_left = readcnt;
	unsigned int p;
	for (p = 0; p < packets; p++) {
		unsigned int write_now = min(CH341_PACKET_LENGTH - 1, write_left);
		unsigned int read_now = min ((CH341_PACKET_LENGTH - 1) - write_now, read_left);
		ptr = wbuf[p + 1];
		*ptr++ = CH341A_CMD_SPI_STREAM;
		unsigned int i;
		for (i = 0; i < write_now; ++i)
			*ptr++ = swap_byte(*writearr++);
		if (read_now) {
			memset(ptr, 0xFF, read_now);
			read_left -= read_now;
		}
		write_left -= write_now;
	}

	ret = ch341a_usb_transf(ch341a, __func__, BULK_WRITE_ENDPOINT, wbuf[0], CH341_PACKET_LENGTH + packets + writecnt + readcnt);
	ret = ch341a_usb_transf(ch341a, __func__, BULK_READ_ENDPOINT, rbuf, writecnt + readcnt);

	if (ret < 0)
		return -1;

	unsigned int i;
	for (i = 0; i < readcnt; i++) {
		*readarr++ = swap_byte(rbuf[writecnt + i]);
	}

	return 0;
}

#define CS0_LINE	0

static int ch341a_cs_assert(const struct spi_controller *spi_controller)
{
	PREAMBLE(spi_controller);

	_ch341a_gpio_set_value(ch341a, CS0_LINE, false);

	return 0;
}


static int ch341a_cs_release(const struct spi_controller *spi_controller)
{
	PREAMBLE(spi_controller);

	_ch341a_gpio_set_value(ch341a, CS0_LINE, true);

	return 0;
}

static int ch341a_spi_shutdown(const struct spi_controller *spi_controller)
{
	PREAMBLE(spi_controller);

	ch341a_close(ch341a);

	return 0;
}

static int ch341a_spi_init(
		const struct spi_controller *spi_controller,
		int(*log_cb)(int level, const char *tag, const char *restrict format,...),
		const char *connection)
{
	struct ch341a_handle *ch341a = ch341a_open(log_cb);

	if (!ch341a)
		return -ENODEV;

	if ((ch341a_config_stream(ch341a, CH341A_STM_I2C_750K) < 0) ||
			(ch341a_enable_pins(ch341a, true) < 0)) {
		ch341a_close(ch341a);
		return -EIO;
	}

	spi_controller_set_client_data(spi_controller, ch341a, false);

	/* make sure CS is in the default state */
	ch341a_cs_release(spi_controller);

	return 0;
}

static int ch341a_spi_max_transfer(const struct spi_controller *spi_controller)
{
	return 8;
}

const struct spi_controller ch341a_spi = {
	.name = "ch341a",
	.init = ch341a_spi_init,
	.shutdown = ch341a_spi_shutdown,
	.cs_assert = ch341a_cs_assert,
	.cs_release = ch341a_cs_release,
	.send_command = ch341a_spi_send_command,
	.max_transfer = ch341a_spi_max_transfer,
	.client = &ch341a_client,
};
