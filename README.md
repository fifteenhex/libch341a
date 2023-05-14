# libch341a

This is an attempt to make the least shit code that operates
the ch341a/ch347 as possible. There seem to be tons of different
attempts at this buried within different tools for doing
things with i2c or spi devices but none of them seem to very
good.

To avoid yet another hacked up copy of someone else's
ch341a code in yet another project lets make a reusable
library.

This will use my libusrio shim thingy to make the API
look like the Linux i2c/spi userspace APIs because
that's cool.

https://github.com/fifteenhex/libusrio

## More info about the ch341a

http://www.wch-ic.com/products/CH341.html

| #  | name       | name on common programmer | # | name | name on common programmer |
|----|------------|---------------------------|---|------|---------------------------|
| 15 | D0 - GPIO0 | CS                        |   |      |                           |
|    |            |                           |   |      |                           |
|    |            |                           |   |      |                           |

## More info about the ch347

The ch347 seems to be basically the same as the
ch341a except: Has JTAG, better SPI implementation, some
differences with how NAKs are handled, bigger USB endpoints.

http://www.wch-ic.com/products/CH347.html
