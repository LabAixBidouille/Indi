INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/libs -I$(top_srcdir)/src/libs/apogee -I$(top_srcdir)/src/libs/fli

# Only compile SBIG Library if we have libusb. If libsbigudrv is not available, compile a dummy version so that users can
# install the proprietery version later on.
if HAVE_LIBUSB
# FIXME Fix apogee ccd dependency problems on Elwood's environment
##usb_drivers = apogee_ccd
if !HAVE_LIBSBIGUDRV
lib_LTLIBRARIES = libsbigudrv.la

libsbigudrv_la_SOURCES = sbig_dummy.cpp
libsbigudrv_la_LIBADD = -lm

usb_drivers = indi_sbig_ccd
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



# NOTE: indialta does not conform to INDI Library standard practices. That is, it doesn't use cfitsio and it doesn't conform to
# INDI Standard Properties. Have a look at fli_ccd.c and sbigcam.cpp for a compliant driver. The driver needs to implement all 
# standard properties expected from a CCD driver staring. Specifically, it needs to implement CONNECTION, CCD_EXPOSURE, 
# CCD_TEMPERATURE, CCD_FRAME, CCD_BINNING. For a full list, refer to http://indi.sourceforge.net/index.php/Standard_Properties
# This should be an excellent exercise for new and veteran INDI Developers alike. If you fix this driver, please send 
# the updated version to mutlaqja@ikarustech.com, and send a note to indi-devel@lists.sourceforge.net about it.

#apogee_ccd_SOURCES = indialta.cpp  $(indi_src_api)
#apogee_ccd_LDADD = $(top_srcdir)/src/libs/fli/libfli.la $(indi_lib_api) -lz
