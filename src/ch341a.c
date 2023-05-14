//SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 */

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <dgputil.h>

#include "ch341a.h"
#include "ch341a_log.h"

static const struct ch341a_dev_entry devs_ch341a_spi[] = {
	{
		.vendor_id = 0x1a86,
		.device_id = 0x5512,
		.vendor_name = "WinChipHead (WCH)",
		.device_name = "CH341A - EPP/MEM mode",
		.ep_size = 32,
		.ep_out = 0x02,
		.ep_in = 0x82,
		.spi_controller = &ch341a_spi,
	},
	{
		.vendor_id = 0x1a86,
		.device_id = 0x55db,
		.vendor_name = "WinChipHead (WCH)",
		.device_name = "CH347 - Mode 1",
		.ep_size = 512,
		.ep_out = 0x06,
		.ep_in = 0x86,
		.is_ch347 = true,
		.spi_controller = &ch347_spi,
	},
};

int ch341a_drain(struct ch341a_handle *ch341a)
{
	uint8_t buff[32] = { 0 };
	int ret = ch341a_usb_transf(ch341a, __func__, BULK_READ_ENDPOINT, buff, sizeof(buff));

	return ret;
}

static int ch341a_null_log_cb(int level, const char* tag, const char *restrict format, ...)
{
	return 0;
}

int ch341a_usb_transf(struct ch341a_handle *ch341a, const char *func,
					  uint8_t type, uint8_t *buf, int len)
{
	int ret, actuallen = 0;
	unsigned char ep = (type == BULK_WRITE_ENDPOINT) ?
			ch341a->dev_info->ep_out : ch341a->dev_info->ep_in;

	ret = libusb_bulk_transfer(ch341a->handle, ep, buf, len, &actuallen, DEFAULT_TIMEOUT);
	if (ret < 0) {
		ch341a_err(ch341a, "%s: Failed to %s %d bytes '%s(%d)'\n", func,
			(type == BULK_WRITE_ENDPOINT) ? "write" : "read", len, libusb_strerror(ret), ret);
		return -1;
	}

	if (actuallen != len) {
		ch341a_err(ch341a, "expected %d bytes of bulk transfer, actually got %d\n", len, actuallen);
		return -EIO;
	}

	return actuallen;
}

/*   Set the I2C bus speed (speed(b1b0): 0 = 20kHz; 1 = 100kHz, 2 = 400kHz, 3 = 750kHz).
 *   Set the SPI bus data width (speed(b2): 0 = Single, 1 = Double).  */
int ch341a_config_stream(struct ch341a_handle *ch341a, unsigned int speed)
{
	int ret;

	uint8_t buf[] = {
		CH341A_CMD_I2C_STREAM,
		CH341A_CMD_I2C_STM_SET | (speed & 0x7),
		CH341A_CMD_I2C_STM_END
	};

	ret = ch341a_usb_transf(ch341a, __func__, BULK_WRITE_ENDPOINT, buf, sizeof(buf));
	if (ret < 0)
		ch341a_err(ch341a, "Could not configure stream interface.\n");

	return ret;
}

#define	 USB_TIMEOUT		1000	/* 1000 ms is plenty and we have no backup strategy anyway. */
#define	 WRITE_EP			0x02
#define	 READ_EP			0x82

int ch341a_enable_pins(struct ch341a_handle *ch341a, bool enable)
{
	int ret;

	const uint8_t buf[] = {
		CH341A_CMD_UIO_STREAM,
		CH341A_CMD_UIO_STM_OUT | 0x37, // CS high (all of them), SCK=0, DOUT*=1
		CH341A_CMD_UIO_STM_OUT | 0x37, // CS high (all of them), SCK=0, DOUT*=1
		CH341A_CMD_UIO_STM_OUT | 0x37, // CS high (all of them), SCK=0, DOUT*=1
		CH341A_CMD_UIO_STM_OUT | 0x37, // CS high (all of them), SCK=0, DOUT*=1
		CH341A_CMD_UIO_STM_OUT | 0x37, // CS high (all of them), SCK=0, DOUT*=1
		CH341A_CMD_UIO_STM_OUT | 0x36, // CS low (all of them), SCK=0, DOUT*=1
		CH341A_CMD_UIO_STM_DIR | (enable ? 0x3F : 0x00), // Interface output enable / disable
		CH341A_CMD_UIO_STM_END,
	};

	ret = ch341a_usb_transf(ch341a, __func__, BULK_WRITE_ENDPOINT, buf, sizeof(buf));
	if (ret < 0) {
		ch341a_err(ch341a, "Could not %sable output pins.\n", enable ? "en" : "dis");
	}

	return ret;
}

static void ch341a_dealloc(struct ch341a_handle *ch341a)
{

}

static int ch341a_alloc(struct ch341a_handle *ch341a)
{
	return 0;
}

struct ch341a_handle *ch341a_open(int (*log_cb)(int level, const char *tag, const char *restrict format,...))
{
	struct ch341a_handle *ch341a = NULL;
	int ret;

	ch341a = malloc(sizeof(*ch341a));
	if (!ch341a) {
		log_cb(0, "", "Failed to malloc() memory");
		return err_ptr(-ENOMEM);
	}

	memset(ch341a, 0, sizeof(*ch341a));
	if (log_cb)
		ch341a->log_cb = log_cb;
	else
		ch341a->log_cb = ch341a_null_log_cb;

	ret = libusb_init(NULL);
	if (ret < 0) {
		ch341a_err(ch341a, "Couldnt initialize libusb!\n");
		return err_ptr(-1);
	}
#if LIBUSB_API_VERSION >= 0x01000106
	libusb_set_option(NULL, LIBUSB_OPTION_LOG_LEVEL, 3);
#else
	libusb_set_debug(NULL, 3); // Enable information, warning and error messages (only).
#endif

	for (int i = 0; i < array_size(devs_ch341a_spi); i++) {
		uint16_t vid = devs_ch341a_spi[i].vendor_id;
		uint16_t pid = devs_ch341a_spi[i].device_id;

		ch341a->handle = libusb_open_device_with_vid_pid(NULL, vid, pid);

		if (ch341a->handle) {
			ch341a_info(ch341a, "Found CH341A-like device: %s - %s\n",
				devs_ch341a_spi[i].vendor_name, devs_ch341a_spi[i].device_name);
			ch341a->dev_info = &devs_ch341a_spi[i];
			break;
		}

		ch341a_err(ch341a, "Couldn't open device %04x:%04x.\n", vid, pid);
	}

	if (!ch341a) {
		ch341a_info(ch341a, "No CH341A-like device found\n");
		return err_ptr(-ENODEV);
	}

#ifdef __gnu_linux__
	/* libusb_detach_kernel_driver() and friends basically only work on Linux. We simply try to detach on Linux
	 * without a lot of passion here. If that works fine else we will fail on claiming the interface anyway. */
	ret = libusb_detach_kernel_driver(ch341a->handle, 0);
	if (ret == LIBUSB_ERROR_NOT_SUPPORTED) {
		ch341a_err(ch341a, "Detaching kernel drivers is not supported. Further accesses may fail.\n");
	} else if (ret != 0 && ret != LIBUSB_ERROR_NOT_FOUND) {
		ch341a_err(ch341a, "Failed to detach kernel driver: '%s'. Further accesses will probably fail.\n",
			  libusb_error_name(ret));
	}
#endif

	ret = libusb_claim_interface(ch341a->handle, 0);
	if (ret != 0) {
		ch341a_err(ch341a, "Failed to claim interface 0: '%s'\n", libusb_error_name(ret));
		goto close_handle;
	}

	struct libusb_device *dev;
	if (!(dev = libusb_get_device(ch341a->handle))) {
		ch341a_err(ch341a, "Failed to get device from device handle.\n");
		goto close_handle;
	}

	struct libusb_device_descriptor desc;
	ret = libusb_get_device_descriptor(dev, &desc);
	if (ret < 0) {
		ch341a_err(ch341a, "Failed to get device descriptor: '%s'\n", libusb_error_name(ret));
		goto release_interface;
	}

	ch341a_info(ch341a, "Device revision is %d.%01d.%01d\n",
		(desc.bcdDevice >> 8) & 0x00FF,
		(desc.bcdDevice >> 4) & 0x000F,
		(desc.bcdDevice >> 0) & 0x000F);

	/* Allocate and pre-fill transfer structures. */
	ret = ch341a_alloc(ch341a);

	if (!ret) {
		ch341a_dbg(ch341a, "CH341a opened successfully\n");
		return ch341a;
	}

	ch341a_dealloc(ch341a);
release_interface:
	libusb_release_interface(ch341a->handle, 0);
close_handle:
	libusb_close(ch341a->handle);
	free(ch341a);

	return err_ptr(ret);
}

void ch341a_close(struct ch341a_handle *ch341a)
{
	assert(ch341a);

	ch341a_enable_pins(ch341a, false);
	ch341a_dealloc(ch341a);
	libusb_release_interface(ch341a->handle, 0);
	libusb_close(ch341a->handle);
	libusb_exit(NULL);
}

static int ch341a_mfd_open(const struct libusrio_mfd *mfd, int(*log_cb)(int level, const char* tag, const char *restrict format,...),
		const char *connection_string, void **priv)
{
	struct ch341a_handle *ch341a = ch341a_open(log_cb);

	if (is_err_ptr(ch341a))
		return ptr_err(ch341a);

	*priv = ch341a;

	return 0;
}

static const struct i2c_controller *ch341a_mfd_get_i2c(const struct libusrio_mfd *mfd, void* priv)
{
	return &ch341a_i2c;
}

static const struct gpio_controller *ch341a_mfd_get_gpio(const struct libusrio_mfd *mfd, void* priv)
{
	return &ch341a_gpio;
}

static const struct spi_controller *ch341a_mfd_get_spi(const struct libusrio_mfd *mfd, void* priv)
{
	struct ch341a_handle *ch341a = priv;

	return ch341a->dev_info->spi_controller;
}

int ch341a_mfd_set_flags(const struct libusrio_mfd *mfd, void *priv, unsigned int flags)
{
	struct ch341a_handle *ch341a = priv;

	ch341a->mfd_flags = flags;

	return 0;
}

int ch341a_mfd_get_flags(const struct libusrio_mfd *mfd, void *priv, unsigned int *flags)
{
	struct ch341a_handle *ch341a = priv;

	*flags = ch341a->mfd_flags;

	return 0;
}

const struct libusrio_mfd ch341a_mfd = {
	.open = ch341a_mfd_open,
	.get_i2c = ch341a_mfd_get_i2c,
	.get_gpio = ch341a_mfd_get_gpio,
	.get_spi = ch341a_mfd_get_spi,
	.set_flags = ch341a_mfd_set_flags,
	.get_flags = ch341a_mfd_get_flags,
};
