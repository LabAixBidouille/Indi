INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/libs -I$(top_srcdir)/src/libs/webcam

# For indi_src_api and indi_lib_api
include $(top_srcdir)/src/Makefile.include

if HAVE_CFITSIO
bin_linux = v4ldriver v4lphilips meade_lpi stv
endif

bin_PROGRAMS = $(bin_linux)

stv_SOURCES = stv.cpp stvdriver.c $(indi_src_api)
stv_LDADD = $(indi_lib_api) @LIBCFITSIO@ -lm -lz

v4ldriver_SOURCES = v4ldriver.cpp indi_v4l.cpp $(indi_src_api)
v4ldriver_LDADD = $(indi_lib_api) webcam/libwebcam.la @LIBCFITSIO@ -lm -lz

v4lphilips_SOURCES = v4ldriver.cpp v4lphilips.cpp indi_philips.cpp $(indi_src_api)
v4lphilips_LDADD = $(indi_lib_api) $(top_srcdir)/src/libs/webcam/libwebcam.la @LIBCFITSIO@ -lm -lz

meade_lpi_SOURCES = v4ldriver.cpp indi_lpi.cpp $(indi_src_api)
meade_lpi_LDADD = $(indi_lib_api) $(top_srcdir)/src/libs/webcam/libwebcam.la @LIBCFITSIO@ -lz
