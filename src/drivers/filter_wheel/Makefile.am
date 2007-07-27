INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/libs -I$(top_srcdir)/src/libs/fli

bin_PROGRAMS = indi_fli_wheel indi_trutech_wheel

# For indi_src_api and indi_lib_api
include $(top_srcdir)/src/Makefile.include

indi_fli_wheel_SOURCES = fli_wheel.c $(indi_src_api)
indi_fli_wheel_LDADD = $(top_srcdir)/src/libs/fli/libfli.la -lm $(indi_lib_api)

indi_trutech_wheel_SOURCES =  trutech_wheel.c $(indi_src_api)
indi_trutech_wheel_LDADD = -lm $(indi_lib_api)
