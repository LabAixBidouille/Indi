INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/libs -I$(top_srcdir)/src/libs/fli

bin_PROGRAMS = indi_robo_focuser indi_fli_focuser

# For indi_src_api and indi_lib_api
include $(top_srcdir)/src/Makefile.include

indi_robo_focuser_SOURCES = robofocusdriver.c robofocus.c $(indi_src_api)
indi_robo_focuser_LDADD = -lm $(indi_lib_api)

indi_fli_focuser_SOURCES = fli_pdf.c $(indi_src_api)
indi_fli_focuser_LDADD = $(top_srcdir)/src/libs/fli/libfli.la -lm $(indi_lib_api)
