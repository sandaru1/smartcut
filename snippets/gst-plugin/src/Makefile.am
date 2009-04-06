plugin_LTLIBRARIES = libgstfaceblur.la

noinst_HEADERS = gstfaceblur.h

libgstfaceblur_la_SOURCES = gstfaceblur.c
libgstfaceblur_la_CFLAGS = $(GST_CFLAGS) $(GST_BASE_CFLAGS) "-I/usr/local/include/opencv"
libgstfaceblur_la_LIBADD = $(GST_PLUGINS_BASE_LIBS) -lgstvideo-@GST_MAJORMINOR@ -lcxcore -lcv -lhighgui -lcvaux -lml $(GST_BASE_LIBS) $(GST_LIBS)
libgstfaceblur_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstfaceblur_la_LIBTOOLFLAGS = --tag=disable-static


