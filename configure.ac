AC_PREREQ([2.60])

AC_INIT([init], 0.1, [david.oberhollenzer@tele2.at], init)
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign subdir-objects dist-xz])
AM_SILENT_RULES([yes])
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_RANLIB
AC_PROG_MKDIR_P
AC_PROG_LN_S

AC_ARG_WITH([pygos],
	[AS_HELP_STRING([--without-pygos],
			[Disable Pygos specific default configuration])],
	[case "${withval}" in
	yes) AM_CONDITIONAL([PYGOSCFG], [true]) ;;
	no)  AM_CONDITIONAL([PYGOSCFG], [false]) ;;
	*) AC_MSG_ERROR([bad value ${withval} for --without-pygos]) ;;
	esac],
	[AM_CONDITIONAL([PYGOSCFG], [true])])


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


AC_CONFIG_HEADERS([lib/include/config.h])
AC_DEFINE_DIR(SVCDIR, sysconfdir/init.d, [Startup service directory])
AC_DEFINE_DIR(TEMPLATEDIR, datadir/init, [Service template directory])
AC_DEFINE_DIR(SCRIPTDIR, libexecdir/init, [Helper script directory])
AC_DEFINE_DIR(SOCKDIR, localstatedir/run, [Directory for initd socket])

AC_DEFINE_DIR(BINPATH, bindir, [Fully evaluated bin directory])
AC_DEFINE_DIR(SBINPATH, sbindir, [Fully evaluated sbin directory])
AC_DEFINE_DIR(ETCPATH, sysconfdir, [Fulle evaluated etc directory])

AC_CONFIG_FILES([services/agetty])
AC_CONFIG_FILES([services/hostname])
AC_CONFIG_FILES([services/loopback])
AC_CONFIG_FILES([services/sysctl])
AC_CONFIG_FILES([services/hwclock])
AC_CONFIG_FILES([services/reboot])
AC_CONFIG_FILES([services/shutdown])
AC_CONFIG_FILES([services/sigkill])
AC_CONFIG_FILES([services/sigterm])
AC_CONFIG_FILES([services/sync])
AC_CONFIG_FILES([services/sysfs])
AC_CONFIG_FILES([services/devfs])
AC_CONFIG_FILES([services/procfs])
AC_CONFIG_FILES([services/tmpfs])
AC_CONFIG_FILES([services/tmpfsrun])
AC_CONFIG_FILES([services/tmpfsvar])
AC_CONFIG_FILES([services/rootusr])
AC_CONFIG_FILES([scripts/devfs.sh])
AC_CONFIG_FILES([scripts/overlay.sh])
AC_CONFIG_FILES([scripts/trymount.sh])
AC_CONFIG_FILES([etc/initd.env])

AC_OUTPUT([Makefile])
