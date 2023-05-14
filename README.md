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

## Issues

- There seems to be no way to get the current output state of pins?
  So setting one pin can change the state of the others if they are not known.
  

## Sample programs

Included are a set of hacky sample programs. Most of the are re-implementations
of tools you'd normally use with an in-kernel driver:

- ch341a_i2cdetect - Like i2cdetect from i2c-utils
- ch341a_i2cget - Like i2cget from i2c-utils
- ch341a_i2cset - Like i2cset from i2c-utils
- ch341a_gpioinfo - Like gpioinfo from gpiod
- ch341a_gpioget - Like gpioget from gpiod
- ch341a_gpioset - Like gpioset from gpiod

## More info about the ch341a

http://www.wch-ic.com/products/CH341.html

| #  | name | name on common programmer | #  | name       | name on common programmer |
|----|------|---------------------------|----|------------|---------------------------|
| 1  |      |                           | 15 | D0 - GPIO0 | CS                        |
| 2  |      |                           | 16 | D1 - GPIO1 |                           |
| 3  |      |                           | 17 | D2 - GPIO2 |                           |
| 4  |      |                           | 18 | D3 - GPIO3 | CLK                       |
| 5  |      |                           | 19 | D4 - GPIO4 |                           |
| 6  |      |                           | 20 | D5 - GPIO5 |                           |
| 7  |      |                           | 21 | D6 - GPIO6 |                           |
| 8  |      |                           | 22 | D7 - GPIO7 |                           |
| 9  |      |                           | 23 |            |                           |
| 10 |      |                           | 24 |            |                           |
| 11 |      |                           | 25 |            |                           |
| 12 |      |                           | 26 |            |                           |
| 13 |      |                           | 27 |            |                           |
| 14 |      |                           | 28 |            |                           |

## More info about the ch347

The ch347 seems to be basically the same idea as the
ch341a except: Has JTAG, better SPI implementation, some
differences with how NAKs are handled, bigger USB endpoints, no usable GPIO?

http://www.wch-ic.com/products/CH347.html

### Todos:

- Implement SPI
- Work out if it's possible to get any gpio support
  For mode 3 at least GPIO seems to be a hacked on by messing with the i2c bus?
