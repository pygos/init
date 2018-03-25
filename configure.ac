AC_PREREQ([2.60])

AC_INIT([init], 0.1, [david.oberhollenzer@tele2.at], init)
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign subdir-objects dist-xz])
AM_SILENT_RULES([yes])
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_RANLIB

AC_CONFIG_HEADERS([lib/include/config.h])
AC_DEFINE_DIR(SVCDIR, sysconfdir/init.d, [Startup service directory])
AC_DEFINE_DIR(TEMPLATEDIR, datadir/init, [Service template directory])
AC_DEFINE_DIR(SCRIPTDIR, libexecdir/init, [Helper script directory])
AC_DEFINE_DIR(SOCKDIR, localstatedir/run, [Directory for initd socket])

AC_DEFINE_DIR(BINPATH, bindir, [Fully evaluated bin directory])
AC_DEFINE_DIR(SBINPATH, sbindir, [Fully evaluated sbin directory])

AC_CONFIG_FILES([services/agetty])
AC_CONFIG_FILES([services/hostname])
AC_CONFIG_FILES([services/loopback])
AC_CONFIG_FILES([services/sysctl])
AC_CONFIG_FILES([services/hwclock])

AC_OUTPUT([Makefile])
