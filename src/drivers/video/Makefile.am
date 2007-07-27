INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/libs -I$(top_srcdir)/src/libs/webcam

# For indi_src_api and indi_lib_api
include $(top_srcdir)/src/Makefile.include

if HAVE_CFITSIO
bin_linux = indi_v4l_generic indi_v4l_philips indi_meade_lpi indi_sbig_stv
endif

bin_PROGRAMS = $(bin_linux)

indi_sbig_stv_SOURCES = stv.cpp stvdriver.c $(indi_src_api)
indi_sbig_stv_LDADD = $(indi_lib_api) @LIBCFITSIO@ -lm -lz

indi_v4l_generic_SOURCES = v4ldriver.cpp indi_v4l.cpp $(indi_src_api)
indi_v4l_generic_LDADD = $(indi_lib_api) $(top_srcdir)/src/libs/webcam/libwebcam.la @LIBCFITSIO@ -lm -lz

indi_v4l_philips_SOURCES = v4ldriver.cpp v4lphilips.cpp indi_philips.cpp $(indi_src_api)
indi_v4l_philips_LDADD = $(indi_lib_api) $(top_srcdir)/src/libs/webcam/libwebcam.la @LIBCFITSIO@ -lm -lz

indi_meade_lpi_SOURCES = v4ldriver.cpp indi_lpi.cpp $(indi_src_api)
indi_meade_lpi_LDADD = $(indi_lib_api) $(top_srcdir)/src/libs/webcam/libwebcam.la @LIBCFITSIO@ -lz
