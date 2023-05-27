/*
 *
 */

#include <stdio.h>
#include <libch341a.h>

#include "common.h"

int main (int argc, char **argv)
{
	struct ch341a_handle *ch341a;

	void *ch341a_mfd_priv;
	int ret = libusrio_mfd_open(&ch341a_mfd, ch341a_log_cb, NULL,
			LIBUSRIO_MFD_WANTI2C, &ch341a_mfd_priv);
	if (ret) {
		printf("Failed to open ch341a mfd: %d\n", ret);
		return ret;
	}

	const struct i2c_controller *i2c_controller;
	ret = libusrio_mfd_get_i2c(&ch341a_mfd, ch341a_mfd_priv, &i2c_controller);
	if (ret) {
		printf("Failed to open ch341a i2c: %d\n", ret);
		return ret;
	}

	uint8_t reg = 0x02;
	uint8_t val;
	ret = i2c_controller_writeone_then_readone(i2c_controller, 0x51, reg, &val, ch341a_mfd_priv);
	if (ret) {
		printf("Failed to read register %02x: %d\n", reg, ret);
		return ret;
	}

	printf("%02x\n", val);

	return 0;
}
