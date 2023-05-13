argtable2 = dependency('argtable2')

link_libs = [ libch341a ]

gpioinfo = executable(
    'ch341a_gpioinfo', 'ch341a_gpioinfo.c',
    include_directories : inc,
    link_with : link_libs,
    dependencies: libusrio_dep)

gpioget = executable(
    'ch341a_gpioget', 'ch341a_gpioget.c',
    include_directories : inc,
    link_with : link_libs,
    dependencies: [libusrio_dep, argtable2])

gpioset = executable(
    'ch341a_gpioset', 'ch341a_gpioset.c',
    include_directories : inc,
    link_with : link_libs,
    dependencies: [libusrio_dep, argtable2])

probech341a = executable(
    'probech341a', 'probech341a.c',
    include_directories : inc,
    link_with : link_libs,
    dependencies: libusrio_dep)

i2cdetect = executable(
    'i2cdetect', 'i2cdetect.c',
    include_directories : inc,
    link_with : link_libs,
    dependencies: libusrio_dep)

i2cget = executable(
    'i2cget', 'i2cget.c',
    include_directories : inc,
    link_with : link_libs,
    dependencies: libusrio_dep)

i2cset = executable(
    'i2cset', 'i2cset.c',
    include_directories : inc,
    link_with : link_libs,
    dependencies: libusrio_dep)

spiepaper = executable(
    'spiepaper', 'spiepaper.c',
    include_directories : inc,
    link_with : link_libs,
    dependencies: [libusrio_dep, libebogroll_dep])