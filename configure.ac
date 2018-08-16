AC_PREREQ([2.60])

m4_define([RELEASE], 0.5)

AC_INIT([init], [RELEASE], [david.oberhollenzer@tele2.at], init)
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign subdir-objects dist-xz])
AM_SILENT_RULES([yes])
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_INSTALL
AC_PROG_RANLIB
AC_PROG_MKDIR_P
AC_PROG_LN_S

UL_WARN_ADD([-Wall])
UL_WARN_ADD([-Wextra])
UL_WARN_ADD([-Wunused])
UL_WARN_ADD([-Wmissing-prototypes])
UL_WARN_ADD([-Wmissing-declarations])
UL_WARN_ADD([-Wwrite-strings])
UL_WARN_ADD([-Wjump-misses-init])
UL_WARN_ADD([-Wuninitialized])
UL_WARN_ADD([-Winit-self])
UL_WARN_ADD([-Wlogical-op])
UL_WARN_ADD([-Wunused-but-set-parameter])
UL_WARN_ADD([-Wunused-but-set-variable])
UL_WARN_ADD([-Wunused-parameter])
UL_WARN_ADD([-Wunused-result])
UL_WARN_ADD([-Wunused-variable])
UL_WARN_ADD([-Wduplicated-cond])
UL_WARN_ADD([-Wduplicated-branches])
UL_WARN_ADD([-Wrestrict])
UL_WARN_ADD([-Wnull-dereference])
UL_WARN_ADD([-pedantic])

AC_SUBST([WARN_CFLAGS])


AC_ARG_WITH([usyslogd],
	[AS_HELP_STRING([--without-usyslogd], [Build without syslog daemon])],
	[case "${withval}" in
	yes) AM_CONDITIONAL([USYSLOGD], [true]) ;;
	no) AM_CONDITIONAL([USYSLOGD], [false]) ;;
	*) AC_MSG_ERROR([bad value ${withval} for --without-usyslogd]) ;;
	esac],
	[AM_CONDITIONAL([USYSLOGD], [true])])


AC_CONFIG_HEADERS([lib/include/config.h])
AC_DEFINE_DIR(SVCDIR, sysconfdir/init.d, [Startup service directory])
AC_DEFINE_DIR(TEMPLATEDIR, datadir/init, [Service template directory])
AC_DEFINE_DIR(SCRIPTDIR, libexecdir/init, [Helper script directory])
AC_DEFINE_DIR(SOCKDIR, localstatedir/run, [Directory for initd socket])

AC_DEFINE_DIR(BINPATH, bindir, [Fully evaluated bin directory])
AC_DEFINE_DIR(SBINPATH, sbindir, [Fully evaluated sbin directory])
AC_DEFINE_DIR(ETCPATH, sysconfdir, [Fulle evaluated etc directory])
AC_DEFINE_DIR(STATEFILESPATH, prefix/var/lib, [Path for persistent state files])

AC_DEFINE_DIR(PREFIXPATH, prefix, [Fully expaneded installation prefix])

AC_CONFIG_FILES([services/sigkill])
AC_CONFIG_FILES([services/sigterm])
AC_CONFIG_FILES([services/sysfs])
AC_CONFIG_FILES([services/devfs])
AC_CONFIG_FILES([services/procfs])
AC_CONFIG_FILES([services/ifrename])
AC_CONFIG_FILES([services/ifcfg])
AC_CONFIG_FILES([services/ifdown])
AC_CONFIG_FILES([services/modules])
AC_CONFIG_FILES([services/hostapd])
AC_CONFIG_FILES([services/swclock])
AC_CONFIG_FILES([services/swclocksave])
AC_CONFIG_FILES([services/nft])
AC_CONFIG_FILES([scripts/devfs.sh])
AC_CONFIG_FILES([scripts/ifrename.sh])
AC_CONFIG_FILES([scripts/ifcfg.sh])
AC_CONFIG_FILES([scripts/modules_load.sh])
AC_CONFIG_FILES([etc/initd.env])

AC_OUTPUT([Makefile])
