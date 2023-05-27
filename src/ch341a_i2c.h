//SPDX-License-Identifier: GPL-2.0-or-later
/*
 * libUSB driver for the ch341a in i2c mode
 * Copyright 2011 asbokid <ballymunboy@gmail.com>
 */

#ifndef __CH341A_I2C_H__
#define __CH341A_I2C_H__

#include <stdint.h>
#include <libch341a.h>

#include "ch341a.h"
#include "i2c_controller.h"

#define CH341A_CMD_I2C_STM_US			0x40
#define CH341A_CMD_I2C_STM_MS			0x50
#define CH341A_CMD_I2C_STM_DLY			0x0F
#define CH341A_CMD_I2C_STM_START		0x74
#define CH341A_CMD_I2C_STM_STOP			0x75

#define CH341A_I2C_NAK				(1 << 7)
#define CH347_I2C_ACK				1

#endif /* __CH341A_I2C_H__ */
