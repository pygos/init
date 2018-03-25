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

