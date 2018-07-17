ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -D_GNU_SOURCE -I$(top_srcdir)/lib/include
AM_CFLAGS = $(WARN_CFLAGS)

bin_PROGRAMS =
sbin_PROGRAMS =
noinst_LIBRARIES =
nobase_sysconf_DATA = netcfg/ifrename
sysconf_DATA = etc/initd.env
EXTRA_DIST = README.md LICENSE docs netcfg

helperdir = @SCRIPTDIR@
helper_PROGRAMS =
helper_SCRIPTS =

include lib/Makemodule.am
include cmd/Makemodule.am
include initd/Makemodule.am
include scripts/Makemodule.am
include services/Makemodule.am

if USYSLOGD
include syslogd/Makemodule.am
endif

install-data-local:
	$(MKDIR_P) $(DESTDIR)$(SVCDIR)
	$(LN_S) $(TEMPLATEDIR)/loopback $(DESTDIR)$(SVCDIR)/loopback
	$(LN_S) $(TEMPLATEDIR)/hostname $(DESTDIR)$(SVCDIR)/hostname
	$(LN_S) $(TEMPLATEDIR)/sysctl $(DESTDIR)$(SVCDIR)/sysctl
	$(LN_S) $(TEMPLATEDIR)/sysinit $(DESTDIR)$(SVCDIR)/sysinit
	$(LN_S) $(TEMPLATEDIR)/procfs $(DESTDIR)$(SVCDIR)/procfs
	$(LN_S) $(TEMPLATEDIR)/sysfs $(DESTDIR)$(SVCDIR)/sysfs
	$(LN_S) $(TEMPLATEDIR)/devfs $(DESTDIR)$(SVCDIR)/devfs
	$(LN_S) $(TEMPLATEDIR)/tmpfs $(DESTDIR)$(SVCDIR)/tmpfs
	$(LN_S) $(TEMPLATEDIR)/vfs $(DESTDIR)$(SVCDIR)/vfs
	$(LN_S) $(TEMPLATEDIR)/shutdown $(DESTDIR)$(SVCDIR)/shutdown
	$(LN_S) $(TEMPLATEDIR)/reboot $(DESTDIR)$(SVCDIR)/reboot
	$(LN_S) $(TEMPLATEDIR)/ifdown $(DESTDIR)$(SVCDIR)/ifdown@shutdown
	$(LN_S) $(TEMPLATEDIR)/sync $(DESTDIR)$(SVCDIR)/sync@shutdown
	$(LN_S) $(TEMPLATEDIR)/sigkill $(DESTDIR)$(SVCDIR)/sigkill@shutdown
	$(LN_S) $(TEMPLATEDIR)/sigterm $(DESTDIR)$(SVCDIR)/sigterm@shutdown
	$(LN_S) $(TEMPLATEDIR)/sync $(DESTDIR)$(SVCDIR)/sync@reboot
	$(LN_S) $(TEMPLATEDIR)/sigkill $(DESTDIR)$(SVCDIR)/sigkill@reboot
	$(LN_S) $(TEMPLATEDIR)/sigterm $(DESTDIR)$(SVCDIR)/sigterm@reboot
	$(LN_S) $(TEMPLATEDIR)/ifdown $(DESTDIR)$(SVCDIR)/ifdown@reboot
	$(LN_S) $(TEMPLATEDIR)/ifcfg $(DESTDIR)$(SVCDIR)/ifcfg
if USYSLOGD
	$(LN_S) $(TEMPLATEDIR)/usyslogd $(DESTDIR)$(SVCDIR)/usyslogd
endif
