INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/libs

bin_PROGRAMS = lx200basic lx200generic celestrongps temma skycommander intelliscope orionatlas

# For indi_src_api and indi_lib_api
include $(top_srcdir)/src/Makefile.include

lx200generic_SOURCES = lx200driver.c lx200autostar.cpp lx200_16.cpp lx200gps.cpp lx200generic.cpp lx200classic.cpp $(indi_src_api)
lx200generic_LDADD = $(indi_lib_api) -lm

lx200basic_SOURCES = lx200driver.c lx200basic.cpp $(indi_src_api)
lx200basic_LDADD = $(indi_lib_api)

celestrongps_SOURCES =  celestronprotocol.c celestrongps.cpp $(indi_src_api)
celestrongps_LDADD = $(indi_lib_api) -lm

temma_SOURCES = $(indi_src_api) temmadriver.c
temma_LDADD = $(indi_lib_api) -lm

skycommander_SOURCES = lx200driver.c skycommander.c $(indi_src_api)
skycommander_LDADD = $(indi_lib_api) -lm

intelliscope_SOURCES = lx200driver.c intelliscope.c $(indi_src_api)
intelliscope_LDADD = $(indi_lib_api) -lm

orionatlas_SOURCES = orionatlas.cpp $(indi_src_api)
orionatlas_LDADD = $(indi_lib_api) -lm

install-exec-hook:
	$(mkinstalldirs) $(DESTDIR)$(bindir)
	rm -f $(DESTDIR)$(bindir)/lx200classic
	$(LN_S) lx200generic $(DESTDIR)$(bindir)/lx200classic
	rm -f $(DESTDIR)$(bindir)/lx200autostar
	$(LN_S) lx200generic $(DESTDIR)$(bindir)/lx200autostar
	rm -f $(DESTDIR)$(bindir)/lx200_16
	$(LN_S) lx200generic $(DESTDIR)$(bindir)/lx200_16
	rm -f $(DESTDIR)$(bindir)/lx200gps
	$(LN_S) lx200generic $(DESTDIR)$(bindir)/lx200gps

uninstall-local:
	rm $(DESTDIR)$(bindir)/lx200autostar $(DESTDIR)$(bindir)/lx200_16 $(DESTDIR)$(bindir)/lx200gps
