AC_PREREQ([2.66])
AC_INIT([gst-ivfmuxsink], [1.0], [sreerenjb@gnome.org])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])
AC_PROG_CC
LT_PREREQ([2.2])
LT_INIT
PKG_CHECK_MODULES([GST], [gstreamer-1.0 >= 1.2])
PKG_CHECK_MODULES([GST_VIDEO], [gstreamer-video-1.0 >= 1.2])
PKG_CHECK_MODULES([GST_BASE], [gstreamer-base-1.0 >= 1.2])
AC_SUBST(GST_CFLAGS)
AC_SUBST(GST_LIBS)
AC_SUBST(GST_BASE_CFLAGS)
AC_SUBST(GST_BASE_LIBS)
AC_SUBST(GST_VIDEO_CFLAGS)
AC_SUBST(GST_VIDEO_LIBS)
AC_CONFIG_FILES(Makefile)
AC_OUTPUT
