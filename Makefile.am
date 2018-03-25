ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -D_GNU_SOURCE -I$(top_srcdir)/lib/include
AM_CFLAGS = -std=c99 -pedantic -Wall -Wextra

sbin_PROGRAMS =
noinst_LIBRARIES =
EXTRA_DIST = README LICENSE

helperdir = @SCRIPTDIR@
helper_PROGRAMS =

include lib/Makemodule.am
include cmd/Makemodule.am
include initd/Makemodule.am
include services/Makemodule.am
include servicecmd/Makemodule.am

install-data-local:
	$(MKDIR_P) $(DESTDIR)$(SVCDIR)
	$(LN_S) $(TEMPLATEDIR)/loopback $(DESTDIR)$(SVCDIR)/loopback
	$(LN_S) $(TEMPLATEDIR)/hostname $(DESTDIR)$(SVCDIR)/hostname
	$(LN_S) $(TEMPLATEDIR)/sysctl $(DESTDIR)$(SVCDIR)/sysctl
	$(LN_S) $(TEMPLATEDIR)/sysinit $(DESTDIR)$(SVCDIR)/sysinit
	$(LN_S) $(TEMPLATEDIR)/shutdown $(DESTDIR)$(SVCDIR)/shutdown
	$(LN_S) $(TEMPLATEDIR)/reboot $(DESTDIR)$(SVCDIR)/reboot
	$(LN_S) $(TEMPLATEDIR)/sync $(DESTDIR)$(SVCDIR)/sync@shutdown
	$(LN_S) $(TEMPLATEDIR)/sigkill $(DESTDIR)$(SVCDIR)/sigkill@shutdown
	$(LN_S) $(TEMPLATEDIR)/sigterm $(DESTDIR)$(SVCDIR)/sigterm@shutdown
	$(LN_S) $(TEMPLATEDIR)/sync $(DESTDIR)$(SVCDIR)/sync@reboot
	$(LN_S) $(TEMPLATEDIR)/sigkill $(DESTDIR)$(SVCDIR)/sigkill@reboot
	$(LN_S) $(TEMPLATEDIR)/sigterm $(DESTDIR)$(SVCDIR)/sigterm@reboot
