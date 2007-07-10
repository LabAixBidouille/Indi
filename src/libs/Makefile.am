liblilxml_a_SOURCES = lilxml.c
libindicom_a_SOURCES = indicom.c $(top_srcdir)/src/base64.c

noinst_LIBRARIES =  liblilxml.a libindicom.a

if LINUX
if HAVE_CFITSIO
SUBDIRS = . fli webcam apogee
else
SUBDIRS = . fli
endif
endif
