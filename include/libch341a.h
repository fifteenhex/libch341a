#ifndef __LIBCH341A_H__
#define __LIBCH341A_H__

#include <libusb-1.0/libusb.h>
#include <mfd.h>

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

	const struct gpio_controller *gpio_controller;
	const struct spi_controller *spi_controller;
};

/* Don't touch anything inside here, it's not for you */
struct ch341a_handle {
	struct libusb_device_handle *handle;
	int(*log_cb)(int level, const char *tag, const char *restrict format, ...);
	struct ch341a_gpio_state gpio_state;
	const struct ch341a_dev_entry *dev_info;
	unsigned int mfd_flags;

	struct libusrio_i2c_data libusrio_i2c_data;
};

struct ch341a_handle *ch341a_open(int(*log_cb)(int level, const char *tag, const char *restrict format,...));

extern const struct spi_controller ch341a_spi;
extern const struct spi_controller ch347_spi;
extern const struct i2c_controller ch341a_i2c;
extern const struct gpio_controller ch341a_gpio;
extern const struct libusrio_mfd ch341a_mfd;

#endif
