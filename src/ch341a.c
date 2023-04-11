//SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dgputil.h>

#include "ch341a.h"
#include "ch341a_log.h"

static int ch341a_null_log_cb(int level, const char* tag, const char *restrict format, ...)
{
	return 0;
}

int ch341a_usb_transf(struct ch341a_handle *ch341a, const char *func,
					  uint8_t type, uint8_t *buf, int len)
{
	int ret, actuallen = 0;

	ret = libusb_bulk_transfer(ch341a->handle, type, buf, len, &actuallen, DEFAULT_TIMEOUT);
	if (ret < 0) {
		printf("%s: Failed to %s %d bytes '%s(%d)'\n", func,
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
		printf("Could not configure stream interface.\n");

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
		printf("Could not %sable output pins.\n", enable ? "en" : "dis");
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
	uint16_t vid = devs_ch341a_spi[0].vendor_id;
	uint16_t pid = devs_ch341a_spi[0].device_id;
	ch341a->handle = libusb_open_device_with_vid_pid(NULL, vid, pid);
	if (ch341a->handle == NULL) {
		ch341a_err(ch341a, "Couldn't open device %04x:%04x.\n", vid, pid);
		return err_ptr(-1);
	}
	ch341a_info(ch341a, "Found programmer device: %s - %s\n",
			devs_ch341a_spi[0].vendor_name, devs_ch341a_spi[0].device_name);

#ifdef __gnu_linux__
	/* libusb_detach_kernel_driver() and friends basically only work on Linux. We simply try to detach on Linux
	 * without a lot of passion here. If that works fine else we will fail on claiming the interface anyway. */
	ret = libusb_detach_kernel_driver(ch341a->handle, 0);
	if (ret == LIBUSB_ERROR_NOT_SUPPORTED) {
		printf("Detaching kernel drivers is not supported. Further accesses may fail.\n");
	} else if (ret != 0 && ret != LIBUSB_ERROR_NOT_FOUND) {
		printf("Failed to detach kernel driver: '%s'. Further accesses will probably fail.\n",
			  libusb_error_name(ret));
	}
#endif

	ret = libusb_claim_interface(ch341a->handle, 0);
	if (ret != 0) {
		printf("Failed to claim interface 0: '%s'\n", libusb_error_name(ret));
		goto close_handle;
	}

	struct libusb_device *dev;
	if (!(dev = libusb_get_device(ch341a->handle))) {
		printf("Failed to get device from device handle.\n");
		goto close_handle;
	}

	struct libusb_device_descriptor desc;
	ret = libusb_get_device_descriptor(dev, &desc);
	if (ret < 0) {
		printf("Failed to get device descriptor: '%s'\n", libusb_error_name(ret));
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
