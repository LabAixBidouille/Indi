INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/libs

if HAVE_NOVA
extra_bin = indi_temma
endif

bin_PROGRAMS = indi_lx200basic indi_lx200generic indi_celestron_gps indi_skycommander indi_intelliscope indi_orion_atlas $(extra_bin)

# For indi_src_api and indi_lib_api
include $(top_srcdir)/src/Makefile.include

indi_lx200generic_SOURCES = lx200driver.c lx200autostar.cpp lx200_16.cpp lx200gps.cpp lx200generic.cpp lx200classic.cpp $(indi_src_api)
indi_lx200generic_LDADD = $(indi_lib_api) -lm

indi_lx200basic_SOURCES = lx200driver.c lx200basic.cpp $(indi_src_api)
indi_lx200basic_LDADD = $(indi_lib_api)

indi_celestron_gps_SOURCES =  celestronprotocol.c celestrongps.cpp $(indi_src_api)
indi_celestron_gps_LDADD = $(indi_lib_api) -lm

indi_temma_SOURCES = $(indi_src_api) temmadriver.c
indi_temma_LDADD = $(indi_lib_api) -lm

indi_skycommander_SOURCES = lx200driver.c skycommander.c $(indi_src_api)
indi_skycommander_LDADD = $(indi_lib_api) -lm

indi_intelliscope_SOURCES = lx200driver.c intelliscope.c $(indi_src_api)
indi_intelliscope_LDADD = $(indi_lib_api) -lm

indi_orion_atlas_SOURCES = orionatlas.cpp $(indi_src_api)
indi_orion_atlas_LDADD = $(indi_lib_api) -lm

install-exec-hook:
	$(mkinstalldirs) $(DESTDIR)$(bindir)
	rm -f $(DESTDIR)$(bindir)/indi_lx200classic
	$(LN_S) lx200generic $(DESTDIR)$(bindir)/indi_lx200classic
	rm -f $(DESTDIR)$(bindir)/indi_lx200autostar
	$(LN_S) lx200generic $(DESTDIR)$(bindir)/indi_lx200autostar
	rm -f $(DESTDIR)$(bindir)/indi_lx200_16
	$(LN_S) lx200generic $(DESTDIR)$(bindir)/indi_lx200_16
	rm -f $(DESTDIR)$(bindir)/indi_lx200gps
	$(LN_S) lx200generic $(DESTDIR)$(bindir)/indi_lx200gps

uninstall-local:
	rm $(DESTDIR)$(bindir)/indi_lx200autostar $(DESTDIR)$(bindir)/indi_lx200_16 $(DESTDIR)$(bindir)/indi_lx200gps
