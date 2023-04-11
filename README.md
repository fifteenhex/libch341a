# libch341a

This is an attempt to make the least shit code that operates
the ch341a as possible. There seem to be tons of different
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


http://www.wch-ic.com/products/CH341.html
