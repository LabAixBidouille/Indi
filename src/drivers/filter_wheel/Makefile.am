INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/libs -I$(top_srcdir)/src/libs/fli

bin_PROGRAMS = fli_wheel trutech_wheel

# For indi_src_api and indi_lib_api
include $(top_srcdir)/src/Makefile.include

fli_wheel_SOURCES = fli_wheel.c $(indi_src_api)
fli_wheel_LDADD = $(top_srcdir)/src/libs/fli/libfli.la -lm $(indi_lib_api)

trutech_wheel_SOURCES =  trutech_wheel.c $(indi_src_api)
trutech_wheel_LDADD = -lm $(indi_lib_api)
