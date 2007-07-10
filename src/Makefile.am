INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/libs

include $(top_srcdir)/src/Makefile.include

bin_PROGRAMS = indiserver

indiserver_SOURCES = indiserver.c fq.c
indiserver_LDADD = libs/liblilxml.a $(LIBSOCKET)

SUBDIRS = libs . drivers
