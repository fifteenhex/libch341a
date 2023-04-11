/*
 *
 */

#include <stdio.h>
#include <libch341a.h>
#include <i2c_controller.h>

#include "common.h"

int main (int argc, char **argv)
{
	int ret = i2c_controller_init(&ch341a_i2c, ch341a_log_cb, NULL);

	printf("    0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
	for (int i = 0; i < 0x80; i += 0x10) {
		printf("%02x: ", i);
		for (int j = 0; j <= 0xf; j++) {
			uint8_t addr = i + j;
			int ret = i2c_controller_ping(&ch341a_i2c, addr);

			if (!ret)
				printf("%02x ", addr);
			else
				printf("-- ");
		}
		printf("\n");
	}

	i2c_controller_shutdown(&ch341a_i2c);

	return 0;
}
