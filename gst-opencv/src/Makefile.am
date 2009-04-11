plugin_LTLIBRARIES = libgstfaceblur.la libgstlqrscale.la

noinst_HEADERS = gstfaceblur.h gstlqrscale.h

libgstfaceblur_la_SOURCES = gstfaceblur.c
libgstfaceblur_la_CFLAGS = $(GST_CFLAGS) $(GST_BASE_CFLAGS) $(OPENCV_CFLAGS)
libgstfaceblur_la_LIBADD = $(GST_PLUGINS_BASE_LIBS) -lgstvideo-@GST_MAJORMINOR@ $(OPENCV_LIBS) $(GST_BASE_LIBS) $(GST_LIBS)
libgstfaceblur_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstfaceblur_la_LIBTOOLFLAGS = --tag=disable-static

gstlqrscale_la_SOURCES = gstlqrscale.c
gstlqrscale_la_CFLAGS = $(GST_CFLAGS) $(GST_BASE_CFLAGS) $(LQR_CFLAGS)
gstlqrscale_la_LIBADD = $(GST_PLUGINS_BASE_LIBS) -lgstvideo-@GST_MAJORMINOR@ $(LQR_LIBS) $(GST_BASE_LIBS) $(GST_LIBS)
gstlqrscale_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
gstlqrscale_la_LIBTOOLFLAGS = --tag=disable-static

