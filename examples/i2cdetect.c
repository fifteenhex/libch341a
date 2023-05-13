/*
 *
 */

#include <stdio.h>
#include <libch341a.h>
#include <i2c_controller.h>

#include "common.h"

int main (int argc, char **argv)
{
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

	printf("    0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
	for (int i = 0; i < 0x80; i += 0x10) {
		printf("%02x: ", i);
		for (int j = 0; j <= 0xf; j++) {
			uint8_t addr = i + j;
			int ret = i2c_controller_ping(i2c_controller, addr);

			if (!ret)
				printf("%02x ", addr);
			else
				printf("-- ");
		}
		printf("\n");
	}

	libusrio_mfd_close(&ch341a_mfd, ch341a_mfd_priv);

	return 0;
}
