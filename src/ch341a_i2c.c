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

static int ch341a_i2c_init(const struct i2c_controller *i2c_controller,
			   int(*log_cb)(int level, const char *tag, const char *restrict format,...),
			   void *priv)
{
	struct ch341a_handle *ch341a = priv;
	int ret;

	//ret = ch341a_config_stream(ch341a, CH341A_STM_I2C_20K);
	//ret = ch341a_config_stream(ch341a, CH341A_STM_I2C_100K);
	ret = ch341a_config_stream(ch341a, CH341A_STM_I2C_400K);
	//ret = ch341a_config_stream(ch341a, CH341A_STM_I2C_750K);
	if (ret < 0)
		return ret;

	ch341a_drain(ch341a);

	return 0;
}

static int ch341a_i2c_open(const struct i2c_controller *i2c_controller,
						   int(*log_cb)(int level, const char *tag, const char *restrict format,...),
						   const char *connection, void **priv)
{
	struct ch341a_handle *ch341a;
	int ret;

	ch341a = ch341a_open(log_cb);
	if (is_err_ptr(ch341a))
		return ptr_err(ch341a);

	ret = ch341a_i2c_init(i2c_controller, log_cb, ch341a);
	if (ret)
		return ret;

	*priv = ch341a;

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
		struct i2c_rdwr_ioctl_data *i2c_data, void *priv)
{
	struct ch341a_handle *ch341a = priv;
	CMDBUFF(outpkt);
	uint8_t buff[32] = { 0 };
	int msgs_done = 0;
	bool isch347 = ch341a->dev_info->is_ch347;
	int ret;

	assert(ch341a);

	for (int i = 0; i < i2c_data->nmsgs; i++) {
		struct i2c_msg *msg = &i2c_data->msgs[i];
		bool start = (msg->flags & I2C_M_NOSTART) ? false : true;
		bool isread = (msg->flags & I2C_M_RD) ? true : false;
		int outlen = !isread ? msg->len : 0;
		int inlen = isread ? msg->len : 0;
		bool last = (i + 1 == i2c_data->nmsgs);
		bool checkack = false;

		cmdbuff_reset(&outpkt);
		cmdbuff_push(&outpkt, CH341A_CMD_I2C_STREAM);

		ch341a_i2c_dbg(ch341a, "msg %d/%d start %d read %d, size %d\n",
				i + 1, i2c_data->nmsgs, start, isread, msg->len);

		/*
		 * Send start and address, address is sent with it's own OUT
		 * so we get an ack status byte
		 */
		if (start) {
			cmdbuff_push(&outpkt, CH341A_CMD_I2C_STM_START);
			cmdbuff_push(&outpkt, CH341A_CMD_I2C_STM_OUT);
			cmdbuff_push(&outpkt, (msg->addr << 1) | (isread ? 1 : 0));
			checkack = true;
			inlen++;
		}

		/* If this is a read add enough ins for the request amount */
		if (isread) {
			assert(msg->len <= CH341A_MAX_RD);

			for (int j = 0; j < msg->len; j++) {
				/* Last byte in the whole transfer should be nacked */
				bool nack = last && (j + 1 == msg->len);
				cmdbuff_push(&outpkt, CH341A_CMD_I2C_STM_IN | (!nack ? 1 : 0));
			}
		}
		/*
		 * If this is a write we need to work out if to check for ack or not.
		 * Note that the ch341a does not stop if it gets a nak. You must only
		 * write a byte per message if you can't handle bytes after a nak still
		 * being put on the bus
		 *
		 * For the ch347 every byte out gets it's ack checked
		 */
		else {
			if (isch347 || (msg->flags & I2C_M_IGNORE_NAK)) {
				cmdbuff_push(&outpkt, CH341A_CMD_I2C_STM_OUT | outlen);
				for (int j = 0; j < msg->len; j++)
					cmdbuff_push(&outpkt, msg->buf[j]);

				if (isch347)
					inlen++;
			}
			/* write byte with ack check */
			else {
				for (int j = 0; j < msg->len; j++) {
					cmdbuff_push(&outpkt, CH341A_CMD_I2C_STM_OUT);
					cmdbuff_push(&outpkt, msg->buf[j]);

					assert(!checkack);
					checkack = true;
					inlen++;
				}
			}
		}

		if (i + 1 == i2c_data->nmsgs)
			cmdbuff_push(&outpkt, CH341A_CMD_I2C_STM_STOP);

		cmdbuff_push(&outpkt, CH341A_CMD_I2C_STM_END);

		//fixme: write packet on ch347 should have the same out and in packet size?
		//assert (!isch347 || (plen == inlen));
		assert(cmdbuff_size(&outpkt) <= 32);

		ret = ch341a_usb_transf(ch341a, __func__, BULK_WRITE_ENDPOINT, cmdbuff_ptr(&outpkt), cmdbuff_size(&outpkt));
		usleep(100);

		/* Read back the result */
		if (inlen) {
			assert(inlen <= sizeof(buff));

			ret = ch341a_usb_transf(ch341a, __func__, BULK_READ_ENDPOINT, buff, inlen);
			if (ret <= 0)
				goto abort;

			ch341a_i2c_dbg(ch341a, "processing result, %d bytes in\n", inlen);

			/* Check if there is an ack for the address */
			uint8_t *data = buff;
			if (checkack) {
				uint8_t ackbyte = data[0];
				ch341a_i2c_dbg(ch341a, "ack byte: %02x\n", ackbyte);
				if ((isch347 && !(ackbyte && CH347_I2C_ACK)) ||
						(!ch341a->dev_info->is_ch347 && (ackbyte & CH341A_I2C_NAK))) {
					ch341a_i2c_dbg(ch341a, "NAK\n");
					ret = -EIO;
					goto abort;
				}

				data++;
			}

			if (isread) {
				ch341a_i2c_dbg(ch341a, "copying data to result\n");
				memcpy(msg->buf, data, msg->len);
			}
		}

		//return
		//		ret;
		msgs_done++;
	}

	return msgs_done;

abort:
	ch341a_abort(ch341a);
	return ret;
}

static int ch341a_max_transfer(const struct i2c_controller *i2c_controller)
{
	/*
	 * Endpoint buffer is 32 bytes but there is at most 6 bytes of over head
	 * so we only have 26 usable bytes.
	 */
	return 32 - 6;
}

static int ch341a_shutdown(const struct i2c_controller *i2c_controller, void *priv)
{
	struct ch341a_handle *ch341a = priv;

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

static struct libusrio_i2c_data *ch341a_get_libusrio_data(const struct i2c_controller *i2c_controller, void *priv)
{
	struct ch341a_handle *ch341a = priv;

	return &ch341a->libusrio_i2c_data;
}

const struct i2c_controller ch341a_i2c = {
	.name = "ch341a",
	.open = ch341a_i2c_open,
	.init = ch341a_i2c_init,
	.get_func = ch341a_get_func,
	.do_transaction = ch341a_do_transaction,
	.shutdown = ch341a_shutdown,

	.max_transfer = ch341a_max_transfer,
	.does_not_stop_on_nak = ch341a_does_not_stop_on_nak,
	.get_libusrio_data = ch341a_get_libusrio_data,
};
