/* GStreamer
 * Copyright (C) <1999> Erik Walthinsen <omega@cse.ogi.edu>
 * Copyright (C) <2003> David Schleef <ds@schleef.org>
 * Copyright (C) 2003 Arwed v. Merkatz <v.merkatz@gmx.net>
 * Copyright (C) 2006 Mark Nauwelaerts <manauw@skynet.be>
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
 * This file was (probably) generated from
 * gstvideotemplate.c,v 1.12 2004/01/07 21:07:12 ds Exp 
 * and
 * make_filter,v 1.6 2004/01/07 21:33:01 ds Exp 
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstobjblur.h"
#include <string.h>
#include <math.h>

#include <gst/video/video.h>

#define fix(x,n)      (int)((x)*(1 << (n)) + 0.5)

/* BGR/RGB -> Gray */

#define cscGr_32f  0.299f
#define cscGg_32f  0.587f
#define cscGb_32f  0.114f

#define csc_shift  14
#define cscGr  fix(cscGr_32f,csc_shift) 
#define cscGg  fix(cscGg_32f,csc_shift)
#define cscGb  /*fix(cscGb_32f,csc_shift)*/ ((1 << csc_shift) - cscGr - cscGg)

GST_DEBUG_CATEGORY_STATIC (objblur_debug);
#define GST_CAT_DEFAULT objblur_debug

enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_X1,
  PROP_Y1,
  PROP_X2,
  PROP_Y2
      /* FILL ME */
};

#define DEFAULT_PROP_X1  0
#define DEFAULT_PROP_X2  0
#define DEFAULT_PROP_Y1  10
#define DEFAULT_PROP_Y2  10

static const GstElementDetails objblur_details =
GST_ELEMENT_DETAILS ("Video Object Blur",
    "Filter/Effect/Video",
    "Tracks objects and adds a blur to them on video",
    "Sandaruwan Gunathilake <sandaruwan@gunathilake.com>");

static GstStaticPadTemplate gst_objblur_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_RGB)
    );

static GstStaticPadTemplate gst_objblur_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_RGB)
    );

static void gst_objblur_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_objblur_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static gboolean gst_objblur_set_caps (GstBaseTransform * base, GstCaps * incaps,
    GstCaps * outcaps);
static GstFlowReturn gst_objblur_transform_ip (GstBaseTransform * transform,
    GstBuffer * buf);

GST_BOILERPLATE (GstObjblur, gst_objblur, GstVideoFilter, GST_TYPE_VIDEO_FILTER);


static void
gst_objblur_base_init (gpointer g_class)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (g_class);

  gst_element_class_set_details (element_class, &objblur_details);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_objblur_sink_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_objblur_src_template));
}

static void
gst_objblur_class_init (GstObjblurClass * g_class)
{
  GObjectClass *gobject_class;
  GstBaseTransformClass *trans_class;

  gobject_class = G_OBJECT_CLASS (g_class);
  trans_class = GST_BASE_TRANSFORM_CLASS (g_class);

  gobject_class->set_property = gst_objblur_set_property;
  gobject_class->get_property = gst_objblur_get_property;

  g_object_class_install_property (gobject_class, PROP_X1,
      g_param_spec_int ("x1", "x1", "X1",
          1, 100, DEFAULT_PROP_X1, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_X2,
      g_param_spec_int ("x2", "x2", "X2",
          1, 100, DEFAULT_PROP_X2, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_Y1,
      g_param_spec_int ("y1", "y1", "Y1",
          1, 100, DEFAULT_PROP_Y1, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_Y2,
      g_param_spec_int ("y2", "y2", "Y2",
          1, 100, DEFAULT_PROP_Y2, G_PARAM_READWRITE));

  trans_class->set_caps = GST_DEBUG_FUNCPTR (gst_objblur_set_caps);
  trans_class->transform_ip = GST_DEBUG_FUNCPTR (gst_objblur_transform_ip);
}

static void
gst_objblur_init (GstObjblur * objblur, GstObjblurClass * g_class)
{
  GST_DEBUG_OBJECT (objblur, "gst_objblur_init");

  objblur->storage = cvCreateMemStorage(0);

  /* properties */
  objblur->x1 = DEFAULT_PROP_X1;
  objblur->x2 = DEFAULT_PROP_X2;
  objblur->y1 = DEFAULT_PROP_Y1;
  objblur->y2 = DEFAULT_PROP_Y2;
}

static void
gst_objblur_set_property (GObject * object, guint prop_id, const GValue * value,
    GParamSpec * pspec)
{
  GstObjblur *objblur;

  g_return_if_fail (GST_IS_OBJBLUR (object));
  objblur = GST_OBJBLUR (object);

  GST_DEBUG ("gst_objblur_set_property");
  switch (prop_id) {
    case PROP_OBJ_X1:
      objblur->x1 = g_value_get_int (value);
      break;
    case PROP_OBJ_Y1:
      objblur->y1 = g_value_get_int (value);
      break;
    case PROP_OBJ_X2:
      objblur->x2 = g_value_get_int (value);
      break;
    case PROP_OBJ_Y2:
      objblur->y2 = g_value_get_int (value);
      break;
    default:
      break;
  }
}

static void
gst_objblur_get_property (GObject * object, guint prop_id, GValue * value,
    GParamSpec * pspec)
{
  GstObjblur *objblur;

  g_return_if_fail (GST_IS_OBJBLUR (object));
  objblur = GST_OBJBLUR (object);

  switch (prop_id) {
    case PROP_OBJ_X1:
      g_value_set_int (value, objblur->x1);
      break;
    case PROP_OBJ_X2:
      g_value_set_int (value, objblur->x2);
      break;
    case PROP_OBJ_Y1:
      g_value_set_int (value, objblur->y1);
      break;
    case PROP_OBJ_Y2:
      g_value_set_int (value, objblur->y2);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}


static gboolean
gst_objblur_set_caps (GstBaseTransform * base, GstCaps * incaps,
    GstCaps * outcaps)
{
  GstObjblur *this;
  GstStructure *structure;
  gboolean res;

  this = GST_OBJBLUR (base);

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
gst_objblur_transform_ip (GstBaseTransform * base, GstBuffer * outbuf)
{
  GstObjblur *objblur;
  guint8 *data;
  guint size;
  int i,k,pos,j,R,G,B,a;
  IplImage *image;
  //double t;

  //t = (double)cvGetTickCount();

  objblur = GST_OBJBLUR (base);

  if (base->passthrough)
    goto done;

  data = GST_BUFFER_DATA (outbuf);
  size = GST_BUFFER_SIZE (outbuf);

  if (size != objblur->size)
    goto wrong_size;

  image = cvCreateImage(cvSize(objblur->width,objblur->height), 8, 1 );
  for(i=0;i<objblur->height;i++)
    for(k=0;k<objblur->width;k++) {
		pos = (i * objblur->width + k) * 3;
		image->imageData[ pos/3 ] = (data[pos] + data[pos+1] + data[pos+2]) / 3;
	}

/*		CvRect* r = (CvRect*)cvGetSeqElem( objs, i );
		for(k=0;k < r->height;k++)
			for(j=0;j < r->width;j+=10) {
				R = 0; G = 0; B = 0;
				for(a=0;a<10;a++) {
					pos = ((k+r->y) * objblur->width + j + r->x + a) * 3;
					R += data[pos]/10;
					G += data[pos+1]/10;
					B += data[pos+2]/10;
				}
				for(a=0;a<10;a++) {
					pos = ((k+r->y) * objblur->width + j + r->x + a) * 3;
					data[pos]=R;
					data[pos+1]=G;
					data[pos+2]=B;
				}
			}*/

done:
  return GST_FLOW_OK;

  /* ERRORS */
wrong_size:
  {
    GST_ELEMENT_ERROR (objblur, STREAM, FORMAT,
        (NULL), ("Invalid buffer size %d, expected %d", size, objblur->size));
    return GST_FLOW_ERROR;
  }
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  GST_DEBUG_CATEGORY_INIT (objblur_debug, "objblur", 0, "objblur");

  return gst_element_register (plugin, "objblur", GST_RANK_NONE, GST_TYPE_OBJBLUR);
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "objblur",
    "Tracks objects and adds blur on images",
    plugin_init, VERSION, "LGPL", "GSteamer OpenCV Plugins", "http://www.sandaru1.com");
