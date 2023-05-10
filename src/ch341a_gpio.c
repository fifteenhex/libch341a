//SPDX-License-Identifier: GPL-2.0-or-later
/*
 * ch341a_gpio.c
 *
 * Copyright (C) 2021 McMCC <mcmcc@mail.ru>
 * Copyright (C) 2023 Daniel Palmer <daniel@thingy.jp)
 */

#include <assert.h>
#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <gpio_controller.h>

#include "ch341a.h"

#include "ch341a_gpio_log.h"

#define CH341A_GPIO_LINES 8
#define DIR_MASK			0x3F /* D6,D7 - input, D0-D5 - output */

int ch341a_gpio_setdir(struct ch341a_handle *ch341a)
{
	const uint8_t buf[] = {
		CH341A_CMD_UIO_STREAM,
		CH341A_CMD_UIO_STM_DIR | DIR_MASK,
		CH341A_CMD_UIO_STM_END
	};

	return ch341a_usb_transf(ch341a, __func__, BULK_WRITE_ENDPOINT, buf, 3);
}

int ch341a_gpio_setbits(struct ch341a_handle *ch341a, uint8_t bits)
{
	const uint8_t buf[] = {
		CH341A_CMD_UIO_STREAM,
		CH341A_CMD_UIO_STM_OUT | bits,
		CH341A_CMD_UIO_STM_END
	};

	return ch341a_usb_transf(ch341a, __func__, BULK_WRITE_ENDPOINT, buf, 3);
}

int ch341a_gpio_getbits(struct ch341a_handle *ch341a, uint8_t *data)
{
	int ret;
	const uint8_t buf[] = {
		CH341A_CMD_UIO_STREAM,
		CH341A_CMD_UIO_STM_IN,
		CH341A_CMD_UIO_STM_END
	};

	uint8_t result;

	ret = ch341a_usb_transf(ch341a, __func__, BULK_WRITE_ENDPOINT, buf, sizeof(buf));
	if (ret < 0)
		return -1;

	ret = ch341a_usb_transf(ch341a, __func__, BULK_READ_ENDPOINT, &result, 1);
	if (ret < 0)
		return -1;

	*data = result;

	return ret;
}

int _ch341a_gpio_set_value(struct ch341a_handle *ch341a, int line, bool value)
{
	assert(ch341a);
	assert(line < CH341A_GPIO_LINES);

	uint8_t bit = (1 << line);

	if (value)
		ch341a->gpio_state.value |= bit;
	else
		ch341a->gpio_state.value &= ~bit;

	return ch341a_gpio_setbits(ch341a, ch341a->gpio_state.value);
}


/* libusrio api */
static int ch341a_gpio_set_dir(const struct gpio_controller *gpio_controller,
		void *priv, int line, bool out)
{
	return 0;
}

static int ch341a_gpio_set_value(const struct gpio_controller *gpio_controller,
		void *priv, int line, bool value)
{
	struct ch341a_handle *ch341a = priv;

	return _ch341a_gpio_set_value(ch341a, line, value);
}

static int ch341a_gpio_get_value(const struct gpio_controller *gpio_controller,
		void *priv, int line)
{
	struct ch341a_handle *ch341a = priv;
	uint8_t value;
	int ret;

	ret = ch341a_gpio_getbits(ch341a, &value);
	if (ret < 0)
		return ret;

	if ((1 << line) & value)
		return 1;

	return 0;
}

const struct gpio_controller ch341a_gpio = {
	.set_dir = ch341a_gpio_set_dir,
	.set_value = ch341a_gpio_set_value,
	.get_value = ch341a_gpio_get_value,
};
