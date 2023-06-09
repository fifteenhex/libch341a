//SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 */
#ifndef __CH341_H__
#define __CH341_H__

#include <errno.h>
#include <stdint.h>
#include <stdbool.h>

#include "libch341a.h"

#define DEFAULT_TIMEOUT			1000

#define BULK_WRITE_ENDPOINT		0x02
#define BULK_READ_ENDPOINT		0x82
#define	 CH341A_EP_SIZE			32

#define	 CH341A_CMD_SET_OUTPUT		0xA1
#define	 CH341A_CMD_IO_ADDR		0xA2
#define	 CH341A_CMD_PRINT_OUT		0xA3
#define	 CH341A_CMD_SIO_STREAM		0xA9
#define	 CH341A_CMD_I2C_STREAM		0xAA


#define CH341A_CMD_I2C_STM_OUT		0x80
#define CH341A_CMD_I2C_STM_IN		0xC0
#define CH341A_CMD_I2C_STM_MAX		( min( 0x3F, CH341_PACKET_LENGTH ) )
#define CH341A_CMD_I2C_STM_SET		0x60 // bit 2: SPI with two data pairs D5,D4=out, D7,D6=in

#define	 CH341A_CMD_I2C_STM_END		0x00

#define	 CH341A_STM_I2C_20K		0x00
#define	 CH341A_STM_I2C_100K	0x01
#define	 CH341A_STM_I2C_400K	0x02
#define	 CH341A_STM_I2C_750K	0x03
#define	 CH341A_STM_SPI_DBL		0x04

/* The assumed map between UIO command bits, pins on CH341A chip and pins on SPI chip:
 * UIO	CH341A	SPI	CH341A SPI name
 * 0	D0/15	CS/1 	(CS0)
 * 1	D1/16	unused	(CS1)
 * 2	D2/17	unused	(CS2)
 * 3	D3/18	SCK/6	(DCK)
 * 4	D4/19	unused	(DOUT2)
 * 5	D5/20	SI/5	(DOUT)
 * - The UIO stream commands seem to only have 6 bits of output, and D6/D7 are the SPI inputs,
 *  mapped as follows:
 *	D6/21	unused	(DIN2)
 *	D7/22	SO/2	(DIN)
 */

int ch341a_enable_pins(struct ch341a_handle *ch341a, bool enable);
int ch341a_config_stream(struct ch341a_handle *ch341a, unsigned int speed);
void ch341a_close(struct ch341a_handle *ch341a);
int ch341a_usb_transf(struct ch341a_handle *ch341a, const char *func,
		uint8_t type, uint8_t *buf, int len);
int ch341a_drain(struct ch341a_handle *ch341a);

#endif /* __CH341_H__ */
