INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/libs -I$(top_srcdir)/src/libs/apogee -I$(top_srcdir)/src/libs/fli

bin_PROGRAMS = tutorial_one tutorial_two tutorial_three tutorial_dome tutorial_rain tutorial_ccdpreview

# For indi_src_api and indi_lib_api
include $(top_srcdir)/src/Makefile.include

tutorial_one_SOURCES =  tutorial_one.c $(indi_src_api)
tutorial_one_LDADD = $(indi_lib_api) $(LIBSOCKET)

tutorial_two_SOURCES =  tutorial_two.c $(indi_src_api)
tutorial_two_LDADD = $(indi_lib_api) $(LIBSOCKET)

tutorial_dome_SOURCES =  tutorial_dome.c $(indi_src_api)
tutorial_dome_LDADD = $(indi_lib_api) $(LIBSOCKET)

tutorial_rain_SOURCES =  tutorial_rain.c $(indi_src_api)
tutorial_rain_LDADD = $(indi_lib_api) $(LIBSOCKET)

tutorial_three_SOURCES =  tutorial_three.c $(indi_src_api)
tutorial_three_LDADD = $(indi_lib_api) @LIBCFITSIO@ -lm -lz $(LIBSOCKET)

tutorial_ccdpreview_SOURCES =  tutorial_ccdpreview.c $(indi_src_api)
tutorial_ccdpreview_LDADD = $(indi_lib_api) @LIBCFITSIO@ -lm -lz $(LIBSOCKET)
