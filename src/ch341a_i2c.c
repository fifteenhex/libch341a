//SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 */

#include <assert.h>
#include <libusb-1.0/libusb.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <dgputil.h>

#include "ch341a_i2c.h"
#include "cmdbuff.h"

#include "ch341a_i2c_log.h"

#define CH341A_MAX_RD (32 - 6)

static struct i2c_client i2cdev_client;

static int ch341a_i2c_init(const struct i2c_controller *i2c_controller,
						   int(*log_cb)(int level, const char *tag, const char *restrict format,...),
						   const char *connection)
{
	struct ch341a_handle *ch341a;
	int ret;

	ch341a = ch341a_open(log_cb);
	if (is_err_ptr(ch341a))
		return ptr_err(ch341a);

	//ret = ch341a_config_stream(ch341a, CH341A_STM_I2C_20K);
	//ret = ch341a_config_stream(ch341a, CH341A_STM_I2C_100K);
	ret = ch341a_config_stream(ch341a, CH341A_STM_I2C_400K);
	//ret = ch341a_config_stream(ch341a, CH341A_STM_I2C_750K);
	if (ret < 0)
		return ret;

	i2c_controller_set_priv(i2c_controller, ch341a);

	ch341a_drain(ch341a);

	return 0;
}

static int ch341a_abort(struct ch341a_handle *ch341a)
{
	int ret;
	CMDBUFF(stop);

	ch341a_i2c_dbg(ch341a, "sending stop to abort transaction \n");
	cmdbuff_push(&stop, CH341A_CMD_I2C_STREAM);
	cmdbuff_push(&stop, CH341A_CMD_I2C_STM_STOP);
	cmdbuff_push(&stop, CH341A_CMD_I2C_STM_END);
	ret = ch341a_usb_transf(ch341a, __func__, BULK_WRITE_ENDPOINT, stop.buff, stop.pos);

	return ret;
}

static int ch341a_do_transaction(const struct i2c_controller *i2c_controller,
		struct i2c_rdwr_ioctl_data *i2c_data)
{
	struct ch341a_handle *ch341a = i2c_controller_get_priv(i2c_controller);
	uint8_t packet[32] = { 0 };
	uint8_t buff[32] = { 0 };
	int pindex = 0;
	int msgs_done = 0;

	assert(ch341a);

	packet[pindex++] = CH341A_CMD_I2C_STREAM;

	for (int i = 0; i < i2c_data->nmsgs; i++) {
		struct i2c_msg *msg = &i2c_data->msgs[i];
		int plen = pindex;
		int ret;
		bool start = (msg->flags & I2C_M_NOSTART) ? false : true;
		bool isread = (msg->flags & I2C_M_RD) ? true : false;
		int outlen = !isread ? msg->len : 0;
		int inlen = isread ? msg->len : 0;
		bool last = (i + 1 == i2c_data->nmsgs);
		bool checkack = false;

		ch341a_i2c_dbg(ch341a, "msg %d/%d start %d read %d, size %d\n",
				i + 1, i2c_data->nmsgs,start, isread, msg->len);

		/*
		 * Send start and address, address is sent with it's own OUT
		 * so we get an ack status byte
		 */
		if (start) {
			packet[plen++] = CH341A_CMD_I2C_STM_START;
			packet[plen++] = CH341A_CMD_I2C_STM_OUT;
			packet[plen++] = (msg->addr << 1) | (isread ? 1 : 0);

			checkack = true;
			inlen++;
		}

		/* If this is a read add enough ins for the request amount */
		if (isread) {
			assert(msg->len <= CH341A_MAX_RD);

			for (int j = 0; j < msg->len; j++) {
				/* Last byte in the whole transfer should be nacked */
				bool nack = last && (j + 1 == msg->len);
				packet[plen++] = CH341A_CMD_I2C_STM_IN | (!nack ? 1 : 0);
			}
		}
		/*
		 * If this is a write we need to work out if to check for ack or not.
		 * Note that the ch341a does not stop if it gets a nak. You must only
		 * write a byte per message if you can't handle bytes after a nak still
		 * being put on the bus
		 */
		else {
			/* write buffer, can't check ack */
			if (msg->flags & I2C_M_IGNORE_NAK) {
				packet[plen++] = CH341A_CMD_I2C_STM_OUT | outlen;
				for (int j = 0; j < msg->len; j++)
					packet[plen++] = msg->buf[j];
			}
			/* write byte with ack check */
			else {
				for (int j = 0; j < msg->len; j++) {
					packet[plen++] = CH341A_CMD_I2C_STM_OUT;
					packet[plen++] = msg->buf[j];

					assert(!checkack);
					checkack = true;
					inlen++;
				}
			}
		}

		if (i + 1 == i2c_data->nmsgs)
			packet[plen++] = CH341A_CMD_I2C_STM_STOP;

		packet[plen++] = CH341A_CMD_I2C_STM_END;

		assert(plen <= 32);

		ret = ch341a_usb_transf(ch341a, __func__, BULK_WRITE_ENDPOINT, packet, plen);
		usleep(100);

		/* Read back the result */
		if (inlen) {
			uint8_t *data;

			assert(inlen <= sizeof(buff));

			ret = ch341a_usb_transf(ch341a, __func__, BULK_READ_ENDPOINT, buff, inlen);
			if (ret <= 0)
				return ret;

			/* Check if there is an ack for the address */
			data = buff;
			if (checkack) {
				uint8_t ackbyte = data[0];

				ch341a_i2c_dbg(ch341a, "ack byte: %02x\n", ackbyte);
				if (ackbyte & CH341A_I2C_NAK) {
					ch341a_i2c_dbg(ch341a, "NAK\n");
					ch341a_abort(ch341a);
					return -EIO;
				}

				data++;
			}

			if (isread)
				memcpy(msg->buf, data, msg->len);
		}

		//return
		//		ret;
		msgs_done++;
	}

	return msgs_done;
}

static int ch341a_max_transfer(const struct i2c_controller *i2c_controller)
{
	/*
	 * Endpoint buffer is 32 bytes but there is at most 6 bytes of over head
	 * so we only have 26 usable bytes.
	 */
	return 32 - 6;
}

static int ch341a_shutdown(const struct i2c_controller *i2c_controller)
{
	struct ch341a_handle *ch341a = i2c_controller_get_priv(i2c_controller);

	assert(ch341a);

	ch341a_close(ch341a);

	return 0;
}

static int ch341a_get_func(const struct i2c_controller *i2c_controller)
{
	return I2C_FUNC_NOSTART;
}

static bool ch341a_does_not_stop_on_nak(const struct i2c_controller *i2c_controller)
{
	return true;
}

static struct libusrio_i2c_data ch341a_libusrio_data;

const struct i2c_controller ch341a_i2c = {
	.name = "ch341a",
	.init = ch341a_i2c_init,
	.get_func = ch341a_get_func,
	.do_transaction = ch341a_do_transaction,
	.shutdown = ch341a_shutdown,

	.max_transfer = ch341a_max_transfer,
	.does_not_stop_on_nak = ch341a_does_not_stop_on_nak,

	.client = &i2cdev_client,
	._data = &ch341a_libusrio_data,
};
