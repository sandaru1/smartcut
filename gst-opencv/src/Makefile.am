plugin_LTLIBRARIES = libgstfaceblur.la libgstobjblur.la

noinst_HEADERS = gstfaceblur.h gstobjblur.h

libgstfaceblur_la_SOURCES = gstfaceblur.c
libgstfaceblur_la_CFLAGS = $(GST_CFLAGS) $(GST_BASE_CFLAGS) $(OPENCV_CFLAGS)
libgstfaceblur_la_LIBADD = $(GST_PLUGINS_BASE_LIBS) -lgstvideo-@GST_MAJORMINOR@ $(OPENCV_LIBS) $(GST_BASE_LIBS) $(GST_LIBS)
libgstfaceblur_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstfaceblur_la_LIBTOOLFLAGS = --tag=disable-static

libgstobjblur_la_SOURCES = gstobjblur.c
libgstobjblur_la_CFLAGS = $(GST_CFLAGS) $(GST_BASE_CFLAGS) $(OPENCV_CFLAGS)
libgstobjblur_la_LIBADD = $(GST_PLUGINS_BASE_LIBS) -lgstvideo-@GST_MAJORMINOR@ $(OPENCV_LIBS) $(GST_BASE_LIBS) $(GST_LIBS)
libgstobjblur_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstobjblur_la_LIBTOOLFLAGS = --tag=disable-static

