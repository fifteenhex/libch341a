//SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Based on: https://github.com/981213/spi-nand-prog/blob/master/spi-mem/ch347/ch347.h
 *
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

#define CH347_CMD_SPI_INIT	0xC0
#define CH347_CMD_SPI_CONTROL	0xC1
#define CH347_CMD_SPI_RD_WR	0xC2
#define CH347_CMD_SPI_BLCK_RD	0xC3
#define CH347_CMD_SPI_BLCK_WR	0xC4
#define CH347_CMD_INFO_RD	0xCA

struct ch347_spi_hw_config {
	uint16_t SPI_Direction;
	uint16_t SPI_Mode;
	uint16_t SPI_DataSize;
	uint16_t spi_cpol;
	uint16_t spi_cpha;
	/* hardware or software managed CS */
	uint16_t SPI_NSS;
	/* prescaler = x * 8. x: 0=60MHz, 1=30MHz, 2=15MHz, 3=7.5MHz, 4=3.75MHz, 5=1.875MHz, 6=937.5KHz，7=468.75KHz */
	uint16_t SPI_BaudRatePrescaler;
	/* MSB or LSB first */
	uint16_t SPI_FirstBit;
	/* polynomial used for the CRC calculation. */
	uint16_t SPI_CRCPolynomial;
	/* No idea what this is... Original comment from WCH: SPI接口常规读取写入数据命令(DEF_CMD_SPI_RD_WR))，单位为uS */
	uint16_t SPI_WriteReadInterval;
	/* Data to output on MOSI during SPI reading */
	uint8_t SPI_OutDefaultData;
	/*
	* Miscellaneous settings:
	* Bit 7: CS0 polarity
	* Bit 6: CS1 polarity
	* Bit 5: Enable I2C clock stretching
	* Bit 4: NACK on last I2C reading
	* Bit 3-0: reserved
	*/
	uint8_t OtherCfg;
	uint8_t Reserved[4];
};

#define CH347_SPI_MAX (CH341A_EP_SIZE - 1)

/* ch341 requires LSB first, swap the bit order before send and after receive */
static uint8_t swap_byte(uint8_t x)
{
	x = ((x >> 1) & 0x55) | ((x << 1) & 0xaa);
	x = ((x >> 2) & 0x33) | ((x << 2) & 0xcc);
	x = ((x >> 4) & 0x0f) | ((x << 4) & 0xf0);
	return x;
}

#define PREAMBLE(_sc, _priv) \
	struct ch341a_handle *ch341a; \
	ch341a = _priv; \
	assert(ch341a)

static int ch347_spi_send_command(const struct spi_controller *spi_controller,
		unsigned int writecnt,
		unsigned int readcnt,
		const unsigned char *writearr,
		unsigned char *readarr,
		void *priv)
{
	PREAMBLE(spi_controller, priv);
	uint8_t tmp[CH347_SPI_MAX];
	CMDBUFF(cmdbuff);
	int ret = 0, total = writecnt + readcnt;

	assert(total <= CH347_SPI_MAX);

	return 0;
}

#define CS0_LINE	0

static int ch347_cs_assert(const struct spi_controller *spi_controller, void *priv)
{
	PREAMBLE(spi_controller, priv);

	_ch341a_gpio_set_value(ch341a, CS0_LINE, false);

	return 0;
}


static int ch347_cs_release(const struct spi_controller *spi_controller, void *priv)
{
	PREAMBLE(spi_controller, priv);

	_ch341a_gpio_set_value(ch341a, CS0_LINE, true);

	return 0;
}

static int ch347_spi_close(const struct spi_controller *spi_controller, void *priv)
{
	PREAMBLE(spi_controller, priv);

	ch341a_close(ch341a);

	return 0;
}

static int ch347_spi_init(
		const struct spi_controller *spi_controller,
		int(*log_cb)(int level, const char *tag, const char *restrict format,...),
		void *priv)
{
	struct ch341a_handle *ch341a = priv;

	if ((ch341a_config_stream(ch341a, CH341A_STM_I2C_750K) < 0) ||
			(ch341a_enable_pins(ch341a, true) < 0)) {
		ch341a_close(ch341a);
		return -EIO;
	}

	ch341a_drain(ch341a);

	/* make sure CS is in the default state */
	ch347_cs_release(spi_controller, priv);

	return 0;
}

static int ch347_spi_open(
		const struct spi_controller *spi_controller,
		int(*log_cb)(int level, const char *tag, const char *restrict format,...),
		const char *connection, void **priv)
{
	//if (is_err_ptr(ch341a))
	//	return ptr_err(ch341a);

	return 0;
}

static int ch347_spi_max_transfer(const struct spi_controller *spi_controller, void *priv)
{
	return CH347_SPI_MAX;
}

const struct spi_controller ch347_spi = {
	.name = "ch347",
	.open = ch347_spi_open,
	.init = ch347_spi_init,
	.close = ch347_spi_close,
	.cs_assert = ch347_cs_assert,
	.cs_release = ch347_cs_release,
	.send_command = ch347_spi_send_command,
	.max_transfer = ch347_spi_max_transfer,
};
