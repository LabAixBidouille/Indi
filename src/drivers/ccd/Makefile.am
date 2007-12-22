INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/libs -I$(top_srcdir)/src/libs/apogee -I$(top_srcdir)/src/libs/fli

# Only compile SBIG Library if we have libusb. If libsbigudrv is not available, compile a dummy version so that users can
# install the proprietery version later on.
if HAVE_LIBUSB
usb_drivers = indi_apogee_ccd indi_sbig_ccd
if !HAVE_LIBSBIGUDRV
lib_LTLIBRARIES = libsbigudrv.la
libsbigudrv_la_SOURCES = sbig_dummy.cpp
libsbigudrv_la_LIBADD = -lm
endif
endif

# Only compile CCD & Vdeo driver if we have CFITSIO
if HAVE_CFITSIO
bin_linux   =  indi_fli_ccd $(usb_drivers)
endif

bin_PROGRAMS = $(bin_linux)

# For indi_src_api and indi_lib_api
include $(top_srcdir)/src/Makefile.include

indi_fli_ccd_SOURCES =  fli_ccd.c $(indi_src_api)
indi_fli_ccd_LDADD = $(top_srcdir)/src/libs/fli/libfli.la $(indi_lib_api) @LIBCFITSIO@ -lm -lz $(LIBSOCKET)

indi_sbig_ccd_SOURCES = sbigcam.cpp $(indi_src_api)
indi_sbig_ccd_LDADD = $(indi_lib_api) @LIBCFITSIO@ @LIBSBIGUDRV@ -lz

indi_apogee_ccd_SOURCES = indialta.cpp  $(indi_src_api)
indi_apogee_ccd_LDADD = $(indi_lib_api) $(top_srcdir)/src/libs/apogee/libapogee.la @LIBCFITSIO@ -lz
