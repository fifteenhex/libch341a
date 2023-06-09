//SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2021 McMCC <mcmcc@mail.ru>
 * ch341a_gpio.h
 */
#ifndef __CH341A_GPIO_H__
#define __CH341A_GPIO_H__

#include <stdint.h>

#include "ch341a.h"

// todo hide these
#define	 CH341A_CMD_UIO_STREAM		0xAB
#define	 CH341A_CMD_UIO_STM_IN		0x00
#define	 CH341A_CMD_UIO_STM_DIR		0x40
#define	 CH341A_CMD_UIO_STM_OUT		0x80
#define	 CH341A_CMD_UIO_STM_US		0xC0
#define	 CH341A_CMD_UIO_STM_END		0x20

int ch341a_gpio_setdir(struct ch341a_handle *ch341a);
int ch341a_gpio_setbits(struct ch341a_handle *ch341a, uint8_t bits);
int ch341a_gpio_getbits(struct ch341a_handle *ch341a, uint8_t *data);

/* internal hack function for spi.. */
int _ch341a_gpio_set_value(struct ch341a_handle *ch341a, int line, bool value);

#endif /* __CH341A_GPIO_H__ */
