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
#include <unistd.h>

#include <cmdbuff.h>
#include <spi_controller.h>
#include <dgputil.h>

#include "ch341a.h"
#include "ch341a_spi.h"
#include "ch341a_gpio.h"

#include "ch341a_spi_log.h"

#define CH341A_SPI_MAX (CH341A_EP_SIZE - 1)

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
		unsigned int writecnt,
		unsigned int readcnt,
		const unsigned char *writearr,
		unsigned char *readarr)
{
	PREAMBLE(spi_controller);
	uint8_t tmp[CH341A_SPI_MAX];
	CMDBUFF(cmdbuff);
	int ret = 0, total = writecnt + readcnt;

	assert(total <= CH341A_SPI_MAX);

	/*
	 * It seems like an SPI transfer works by setting the stream
	 * once and then the data. The write size to the endpoint
	 * represents the length?
	 */
	cmdbuff_push(&cmdbuff, CH341A_CMD_SPI_STREAM);
	for (int i = 0; i < writecnt; i++) {
		cmdbuff_push(&cmdbuff, swap_byte(*writearr++));
	}

	//cmdbuff_push(&cmdbuff, CH341A_CMD_SPI_STREAM);
	for (int i = 0; i < readcnt; i++) {
		cmdbuff_push(&cmdbuff, 0xff);
	}

	ret = ch341a_usb_transf(ch341a, __func__, BULK_WRITE_ENDPOINT,
			cmdbuff_ptr(&cmdbuff),  cmdbuff_size(&cmdbuff));
	//usleep(10000);
	ret = ch341a_usb_transf(ch341a, __func__, BULK_READ_ENDPOINT,
			tmp, total);
	if (ret < 0)
		ch341a_spi_err(ch341a, "Failed to read %d bytes back: %d\n",
			total, ret);

	if (ret < 0)
		return -1;

	/* Swap bytes and fill the output */
	for (int i = 0; i < readcnt; i++)
		*readarr++ = swap_byte(tmp[writecnt + i]);

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

	ch341a_drain(ch341a);

	/* make sure CS is in the default state */
	ch341a_cs_release(spi_controller);

	return 0;
}

static int ch341a_spi_max_transfer(const struct spi_controller *spi_controller)
{
	return CH341A_SPI_MAX;
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
