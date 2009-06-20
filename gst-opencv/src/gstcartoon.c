/* GStreamer
 * Copyright (C) 2009 Sandaruwan Gunathilake <sandaruwan@gunathilake.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * This file was based on gstgamma.c
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstcartoon.h"
#include <string.h>
#include <math.h>

#include <gst/video/video.h>

GST_DEBUG_CATEGORY_STATIC (cartoon_debug);
#define GST_CAT_DEFAULT cartoon_debug

#define GST_VIDEO_I420_Y_ROWSTRIDE(width) (GST_ROUND_UP_4(width))
#define GST_VIDEO_I420_U_ROWSTRIDE(width) (GST_ROUND_UP_8(width)/2)
#define GST_VIDEO_I420_V_ROWSTRIDE(width) ((GST_ROUND_UP_8(GST_VIDEO_I420_Y_ROWSTRIDE(width)))/2)

#define GST_VIDEO_I420_Y_OFFSET(w,h) (0)
#define GST_VIDEO_I420_U_OFFSET(w,h) (GST_VIDEO_I420_Y_OFFSET(w,h)+(GST_VIDEO_I420_Y_ROWSTRIDE(w)*GST_ROUND_UP_2(h)))
#define GST_VIDEO_I420_V_OFFSET(w,h) (GST_VIDEO_I420_U_OFFSET(w,h)+(GST_VIDEO_I420_U_ROWSTRIDE(w)*GST_ROUND_UP_2(h)/2))
#define GST_VIDEO_I420_SIZE(w,h)     (GST_VIDEO_I420_V_OFFSET(w,h)+(GST_VIDEO_I420_V_ROWSTRIDE(w)*GST_ROUND_UP_2(h)/2))

enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_DIFF_SPACE,
  PROP_TRIP_LEVEL
};

#define DEFAULT_PROP_DIFF_SPACE  1
#define DEFAULT_PROP_TRIP_LEVEL  1200

static const GstElementDetails cartoon_details =
GST_ELEMENT_DETAILS ("Video Cartoonize",
    "Filter/Effect/Video",
    "Flatten colors and draw edges to give a cartoon look to videos",
    "Sandaruwan Gunathilake <sandaruwan@gunathilake.com>");

static GstStaticPadTemplate gst_cartoon_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_RGB)
    );

static GstStaticPadTemplate gst_cartoon_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_RGB)
    );

static void gst_cartoon_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_cartoon_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static gboolean gst_cartoon_set_caps (GstBaseTransform * base, GstCaps * incaps,
    GstCaps * outcaps);
static GstFlowReturn gst_cartoon_transform (GstBaseTransform * transform,
    GstBuffer * inbuf,GstBuffer * outbuf);
static gboolean
gst_cartoon_get_unit_size (GstBaseTransform * btrans, GstCaps * caps,
    guint * size);


GST_BOILERPLATE (GstCartoon, gst_cartoon, GstVideoFilter, GST_TYPE_VIDEO_FILTER);


static void
gst_cartoon_base_init (gpointer g_class)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (g_class);

  gst_element_class_set_details (element_class, &cartoon_details);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_cartoon_sink_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_cartoon_src_template));
}

static void
gst_cartoon_class_init (GstCartoonClass * g_class)
{
  GObjectClass *gobject_class;
  GstBaseTransformClass *trans_class;

  gobject_class = G_OBJECT_CLASS (g_class);
  trans_class = GST_BASE_TRANSFORM_CLASS (g_class);

  gobject_class->set_property = gst_cartoon_set_property;
  gobject_class->get_property = gst_cartoon_get_property;

  g_object_class_install_property (gobject_class, PROP_DIFF_SPACE,
      g_param_spec_int ("space", "space", "space",
          0, 1000, DEFAULT_PROP_DIFF_SPACE, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_TRIP_LEVEL,
      g_param_spec_int ("trip", "trip", "trip",
          0, 1000000, DEFAULT_PROP_TRIP_LEVEL, G_PARAM_READWRITE));

  trans_class->set_caps = GST_DEBUG_FUNCPTR (gst_cartoon_set_caps);
  trans_class->transform = GST_DEBUG_FUNCPTR (gst_cartoon_transform);
  trans_class->get_unit_size = GST_DEBUG_FUNCPTR (gst_cartoon_get_unit_size);
}

static void
gst_cartoon_init (GstCartoon * cartoon, GstCartoonClass * g_class)
{
  GstElementClass *klass = GST_ELEMENT_GET_CLASS (cartoon);
  GST_DEBUG_OBJECT (cartoon, "gst_cartoon_init");

  /* properties */
  cartoon->trip_level = DEFAULT_PROP_TRIP_LEVEL;
  cartoon->diff_space = DEFAULT_PROP_DIFF_SPACE;
}

static gboolean
gst_cartoon_get_unit_size (GstBaseTransform * btrans, GstCaps * caps,
    guint * size)
{
  GstCartoon *cartoon;
  GstStructure *structure;
  gboolean ret = FALSE;
  gint width, height;

  cartoon = GST_CARTOON (btrans);

  structure = gst_caps_get_structure (caps, 0);

  if (gst_structure_get_int (structure, "width", &width) &&
      gst_structure_get_int (structure, "height", &height)) {
    *size = width * height * 3;
    ret = TRUE;
    GST_DEBUG_OBJECT (cartoon, "our frame size is %d bytes (%dx%d)", *size,
        width, height);
  }

  return ret;
}

static void
gst_cartoon_set_property (GObject * object, guint prop_id, const GValue * value,
    GParamSpec * pspec)
{
  GstCartoon *cartoon;

  g_return_if_fail (GST_IS_CARTOON (object));
  cartoon = GST_CARTOON (object);

  GST_DEBUG ("gst_cartoon_set_property");
  switch (prop_id) {
    case PROP_DIFF_SPACE:
      cartoon->diff_space = g_value_get_int (value);
      break;
    case PROP_TRIP_LEVEL:
      cartoon->trip_level = g_value_get_int (value);
      break;
    default:
      break;
  }
}

static void
gst_cartoon_get_property (GObject * object, guint prop_id, GValue * value,
    GParamSpec * pspec)
{
  GstCartoon *cartoon;

  g_return_if_fail (GST_IS_CARTOON (object));
  cartoon = GST_CARTOON (object);

  switch (prop_id) {
    case PROP_DIFF_SPACE:
      g_value_set_int (value, cartoon->diff_space);
      break;
    case PROP_TRIP_LEVEL:
      g_value_set_int (value, cartoon->trip_level);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}


static gboolean
gst_cartoon_set_caps (GstBaseTransform * base, GstCaps * incaps,
    GstCaps * outcaps)
{
  GstCartoon *this;
  GstStructure *structure;
  gboolean res;

  this = GST_CARTOON (base);

  GST_DEBUG_OBJECT (this,
      "set_caps: in %" GST_PTR_FORMAT " out %" GST_PTR_FORMAT, incaps, outcaps);

  structure = gst_caps_get_structure (incaps, 0);

  res = gst_structure_get_int (structure, "width", &this->width);
  res &= gst_structure_get_int (structure, "height", &this->height);
  if (!res)
    goto done;

  this->size = this->width * this->height * 3;

done:
  return res;
}

static GstFlowReturn
gst_cartoon_transform (GstBaseTransform * base, GstBuffer * inbuf, GstBuffer * outbuf)
{
  GstCartoon *cartoon;
  guint8 *data;
  guint8 *out;
  guint size;

  //double t;
  //t = (double)cvGetTickCount();

  cartoon = GST_CARTOON (base);

  if (base->passthrough)
    goto done;

  data = GST_BUFFER_DATA (inbuf);
  out = GST_BUFFER_DATA (outbuf);
  size = GST_BUFFER_SIZE (inbuf);

  if (size != cartoon->size)
    goto wrong_size;

  memcpy(out,data,size);
  make_cartoon(data,out,cartoon->width,cartoon->height,cartoon->diff_space,cartoon->trip_level);

done:
  return GST_FLOW_OK;

  /* ERRORS */
wrong_size:
  {
    GST_ELEMENT_ERROR (cartoon, STREAM, FORMAT,
        (NULL), ("Invalid buffer size %d, expected %d", size, cartoon->size));
    return GST_FLOW_ERROR;
  }
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  GST_DEBUG_CATEGORY_INIT (cartoon_debug, "cartoon", 0, "cartoon");

  return gst_element_register (plugin, "cartoon", GST_RANK_NONE, GST_TYPE_CARTOON);
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "cartoon",
    "Cartoon filter",
    plugin_init, VERSION, "LGPL", "GSteamer OpenCV Plugins", "http://www.sandaru1.com");
