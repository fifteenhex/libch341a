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

#define DEFAULT_CONFIGURATION		0x01
#define DEFAULT_TIMEOUT			300    // 300mS for USB timeouts

/* Based on (closed-source) DLL V1.9 for USB by WinChipHead (c) 2005.
   Supports USB chips: CH341, CH341A
   This can be a problem for copyright, sure asbokid can't release this part on any GPL licence*/

#define mCH341_PACKET_LENGTH		32    /* wMaxPacketSize 0x0020  1x 32 bytes, unused on the source*/
#define mCH341_PKT_LEN_SHORT		8     /* wMaxPacketSize 0x0008  1x 8 bytes, unused on the source*/

#define mCH341_ENDP_INTER_UP		0x81  /* bEndpointAddress 0x81  EP 1 IN (Interrupt), unused on the source*/
#define mCH341_ENDP_INTER_DOWN		0x01  /* This endpoint isn't list on my lsusb -v output, unused on the source*/
#define mCH341_ENDP_DATA_UP		0x82  /* ==BULK_READ_ENDPOINT  Why repeat it?*/
#define mCH341_ENDP_DATA_DOWN		0x02  /* ==BULK_WRITE_ENDPOINT Why repeat it?*/

#define mCH341_PARA_INIT		0xB1  /* Unused on the source*/
#define mCH341_I2C_STATUS		0x52  /* Unused on the source*/
#define mCH341_I2C_COMMAND		0x53  /* Unused on the source*/

#define mCH341_PARA_CMD_R0		0xAC  /* Unused on the source*/
#define mCH341_PARA_CMD_R1		0xAD  /* Unused on the source*/
#define mCH341_PARA_CMD_W0		0xA6  /* Unused on the source*/
#define mCH341_PARA_CMD_W1		0xA7  /* Unused on the source*/
#define mCH341_PARA_CMD_STS		0xA0  /* Unused on the source*/

#define mCH341A_CMD_SET_OUTPUT		0xA1  /* Unused on the source*/
#define mCH341A_CMD_IO_ADDR		0xA2  /* Unused on the source*/
#define mCH341A_CMD_I2C_STREAM		0xAA

#define mCH341A_BUF_CLEAR		0xB2  /* Unused on the source*/
#define mCH341A_I2C_CMD_X		0x54  /* Unused on the source*/
#define mCH341A_DELAY_MS		0x5E  /* Unused on the source*/
#define mCH341A_GET_VER			0x5F  /* Unused on the source*/

#define mCH341_EPP_IO_MAX		( mCH341_PACKET_LENGTH - 1 )  /* Unused on the source*/
#define mCH341A_EPP_IO_MAX		0xFF  /* Unused on the source*/

#define mCH341A_CMD_IO_ADDR_W		0x00  /* Unused on the source*/
#define mCH341A_CMD_IO_ADDR_R		0x80  /* Unused on the source*/

#define mCH341A_CMD_I2C_STM_STA		0x74
#define mCH341A_CMD_I2C_STM_STO		0x75
#define mCH341A_CMD_I2C_STM_OUT		0x80
#define mCH341A_CMD_I2C_STM_IN		0xC0
#define mCH341A_CMD_I2C_STM_MAX		( min( 0x3F, mCH341_PACKET_LENGTH ) )  /* Unused on the source*/
#define mCH341A_CMD_I2C_STM_SET		0x60
#define mCH341A_CMD_I2C_STM_US		0x40  /* Unused on the source*/
#define mCH341A_CMD_I2C_STM_MS		0x50  /* Unused on the source*/
#define mCH341A_CMD_I2C_STM_DLY		0x0F  /* Unused on the source*/
#define mCH341A_CMD_I2C_STM_END		0x00

#define CH341A_I2C_NAK				(1 << 7)
#define CH347_I2C_ACK				1

#endif /* __CH341A_I2C_H__ */
