#ifndef __LIBCH341A_H__
#define __LIBCH341A_H__

#include <libusb-1.0/libusb.h>
#include <i2c_controller.h>
#include <spi_controller.h>

struct ch341a_gpio_state {
	uint8_t dir;
	uint8_t value;
};

struct ch341a_dev_entry {
	uint16_t vendor_id;
	uint16_t device_id;
	const char *vendor_name;
	const char *device_name;

	unsigned int ep_size;
	unsigned char ep_out, ep_in;

	bool is_ch347;
};

/* Don't touch anything inside here, it's not for you */
struct ch341a_handle {
	struct libusb_device_handle *handle;
	int(*log_cb)(int level, const char *tag, const char *restrict format, ...);
	struct ch341a_gpio_state gpio_state;

	const struct ch341a_dev_entry *dev_info;
};

struct ch341a_handle *ch341a_open(int(*log_cb)(int level, const char *tag, const char *restrict format,...));

extern const struct i2c_controller ch341a_i2c;
extern const struct spi_controller ch341a_spi;
extern const struct gpio_controller ch341a_gpio;

#endif
