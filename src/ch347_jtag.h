/*
 * note: just notes for now
 */

#ifndef SRC_CH347_JTAG_H_
#define SRC_CH347_JTAG_H_

// seems to be for reading the current config?
#define CH347_JTAG_OP_INFO_RD			0xca
#define CH347_JTAG_OP_INIT				0xd0
/* bit bang write? */
#define CH347_JTAG_OP_BIT_OP			0xd1
/* bit bang read? */
#define CH347_JTAG_OP_BIT_OP_READ		0xd2
/* shift out data? */
#define CH347_JTAG_OP_DATA_SHIFT		0xd3
/* shift in data? */
#define CH347_JTAG_OP_DATA_SHIFT_READ	0xd4


#endif /* SRC_CH347_JTAG_H_ */
