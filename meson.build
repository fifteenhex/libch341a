project('libch341a', 'c',
        license : 'GPLv2',
        default_options: ['c_std=c99'])

cc = meson.get_compiler('c')
cflags = cc.get_supported_arguments([
    '-Wall',
    '-Werror=incompatible-pointer-types',
    '-Werror=implicit-function-declaration',
    '-Werror=format-security'
])
add_project_arguments(cflags, language: 'c')
add_project_arguments('-D_GNU_SOURCE', language: 'c')

libdgpc_proj = subproject('libdgpc')
libdgpc_dep = libdgpc_proj.get_variable('libdgpc_dep')
log_macros_tmpl = libdgpc_proj.get_variable('log_macros_tmpl')

libusrio_dep = dependency('libusrio', required : false)
if not libusrio_dep.found()
    libusrio_proj = subproject('libusrio')
    libusrio_dep = libusrio_proj.get_variable('libusrio_dep')
endif

inc = include_directories('include')

subdir('include')
subdir('src')
subdir('examples')

libch341a_dep = declare_dependency(
    include_directories : inc,
    link_with : libch341a)

pkg_mod = import('pkgconfig')
pkg_mod.generate(
    libraries : libch341a,
    version : '0',
    name : 'libch341a',
    filebase : 'libch341a',
    description : 'Library to use the WCH ch341a to do stuff')
