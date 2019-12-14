lib_LTLIBRARIES = libgstivfmuxsink.la

noinst_HEADERS = gstivfmuxsink.h
libgstivfmuxsink_la_SOURCES = gstivfmuxsink.c plugin.c
libgstivfmuxsink_la_CFLAGS = $(GST_CFLAGS) $(GST_BASE_CFLAGS) $(GST_VIDEO_CFLAGS)
libgstivfmuxsink_la_LIBADD = \
        $(GST_LIBS) \
        $(GST_BASE_LIBS) \
        $(GST_VIDEO_LIBS) \
        $(NULL)

libdir = ${exec_prefix}/lib/gstreamer-1.0
