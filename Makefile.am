ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -D_GNU_SOURCE -I$(top_srcdir)/lib/include
AM_CFLAGS = $(WARN_CFLAGS)

bin_PROGRAMS =
sbin_PROGRAMS =
noinst_LIBRARIES =
nobase_sysconf_DATA =
sysconf_DATA = etc/initd.env

helperdir = @SCRIPTDIR@
helper_PROGRAMS =
helper_SCRIPTS =

EXTRA_DIST = README.md LICENSE docs

dist_man1_MANS =
dist_man8_MANS =

include lib/Makemodule.am
include cmd/Makemodule.am
include initd/Makemodule.am
include crond/Makemodule.am

install-exec-hook:
	(cd $(DESTDIR)$(sbindir); $(LN_S) shutdown reboot)

install-data-local:
	$(MKDIR_P) $(DESTDIR)$(man8dir)
	(cd $(DESTDIR)$(man8dir); $(LN_S) shutdown.8 reboot.8)
	$(MKDIR_P) $(DESTDIR)$(SVCDIR)
	$(MKDIR_P) $(DESTDIR)$(TEMPLATEDIR)
if GCROND
	$(MKDIR_P) $(DESTDIR)$(GCRONDIR)
endif
