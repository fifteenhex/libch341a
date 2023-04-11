#ifndef __LIBCH341A_H__
#define __LIBCH341A_H__

#include <libusb-1.0/libusb.h>
#include <i2c_controller.h>
#include <spi_controller.h>

struct ch341a_gpio_state {
	uint8_t dir;
	uint8_t value;
};

/* Don't touch anything inside here, it's not for you */
struct ch341a_handle {
	struct libusb_device_handle *handle;
	int(*log_cb)(int level, const char *tag, const char *restrict format, ...);
	struct ch341a_gpio_state gpio_state;
};

struct ch341a_handle *ch341a_open(int(*log_cb)(int level, const char *tag, const char *restrict format,...));

extern const struct i2c_controller ch341a_i2c;
extern const struct spi_controller ch341a_spi;
extern const struct gpio_controller ch341a_gpio;

// hack!
extern int ch341a_gpio_from_handle(struct ch341a_handle *ch341a, struct gpio_controller *gpio_controller);

#endif
