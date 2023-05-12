libusb_dep = dependency('libusb-1.0')

if get_option('libch341a_debug')
	add_project_arguments('-DCONFIG_DEBUG_CH341A', language : 'c')
	if get_option('libch341a_debug_gpio')
		add_project_arguments('-DCONFIG_DEBUG_CH341A_GPIO', language : 'c')
	endif
	if get_option('libch341a_debug_i2c')
		add_project_arguments('-DCONFIG_DEBUG_CH341A_I2C', language : 'c')
	endif
	if get_option('libch341a_debug_spi')
		add_project_arguments('-DCONFIG_DEBUG_CH341A_SPI', language : 'c')
	endif
endif

conf_data = configuration_data()
conf_data.set('TAG', 'ch341a')
conf_data.set('DEBUG_OPT', 'CONFIG_DEBUG_CH341A')
conf_data.set('PREFIX', 'ch341a')
conf_data.set('FUNC', '_log_var->log_cb')

configure_file(input : log_macros_tmpl,
               output : 'ch341a_log.h',
               configuration : conf_data)

conf_data = configuration_data()
conf_data.set('TAG', 'ch341a_gpio')
conf_data.set('DEBUG_OPT', 'CONFIG_DEBUG_CH341A_GPIO')
conf_data.set('PREFIX', 'ch341a_spi')
conf_data.set('FUNC', '_log_var->log_cb')

configure_file(input : log_macros_tmpl,
               output : 'ch341a_gpio_log.h',
               configuration : conf_data)

libch341a_sources = [ 'ch341a.c',
                      'ch341a_gpio.c',
                      'ch341a_spi.c',
                      'ch341a_i2c.c',
                      'ch347_spi.c'
                    ]

conf_data = configuration_data()
conf_data.set('TAG', 'ch341a_i2c')
conf_data.set('DEBUG_OPT', 'CONFIG_DEBUG_CH341A_I2C')
conf_data.set('PREFIX', 'ch341a_i2c')
conf_data.set('FUNC', '_log_var->log_cb')

configure_file(input : log_macros_tmpl,
               output : 'ch341a_i2c_log.h',
               configuration : conf_data)

conf_data = configuration_data()
conf_data.set('TAG', 'ch341a_spi')
conf_data.set('DEBUG_OPT', 'CONFIG_DEBUG_CH341A_SPI')
conf_data.set('PREFIX', 'ch341a_spi')
conf_data.set('FUNC', '_log_var->log_cb')

configure_file(input : log_macros_tmpl,
               output : 'ch341a_spi_log.h',
               configuration : conf_data)

libch341a = shared_library('ch341a',
                           libch341a_sources,
                           dependencies : [ libusb_dep, libusrio_dep, libdgpc_dep],
                           include_directories: inc,
                           install : true)
