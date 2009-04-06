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

#include "gstfaceblur.h"
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

GST_DEBUG_CATEGORY_STATIC (faceblur_debug);
#define GST_CAT_DEFAULT faceblur_debug

enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_FACE_WIDTH,
  PROP_FACE_HEIGHT
      /* FILL ME */
};

#define DEFAULT_PROP_FACE_WIDTH  30
#define DEFAULT_PROP_FACE_HEIGHT  30

static const GstElementDetails faceblur_details =
GST_ELEMENT_DETAILS ("Video face detection",
    "Filter/Effect/Video",
    "Detects and adds a blur to faces on video",
    "Sandaruwan Gunathilake <sandaruwan@gunathilake.com>");

static GstStaticPadTemplate gst_faceblur_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_RGB)
    );

static GstStaticPadTemplate gst_faceblur_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_RGB)
    );

static void gst_faceblur_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_faceblur_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static gboolean gst_faceblur_set_caps (GstBaseTransform * base, GstCaps * incaps,
    GstCaps * outcaps);
static GstFlowReturn gst_faceblur_transform_ip (GstBaseTransform * transform,
    GstBuffer * buf);

GST_BOILERPLATE (GstFaceblur, gst_faceblur, GstVideoFilter, GST_TYPE_VIDEO_FILTER);


static void
gst_faceblur_base_init (gpointer g_class)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (g_class);

  gst_element_class_set_details (element_class, &faceblur_details);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_faceblur_sink_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_faceblur_src_template));
}

static void
gst_faceblur_class_init (GstFaceblurClass * g_class)
{
  GObjectClass *gobject_class;
  GstBaseTransformClass *trans_class;

  gobject_class = G_OBJECT_CLASS (g_class);
  trans_class = GST_BASE_TRANSFORM_CLASS (g_class);

  gobject_class->set_property = gst_faceblur_set_property;
  gobject_class->get_property = gst_faceblur_get_property;

  g_object_class_install_property (gobject_class, PROP_FACE_WIDTH,
      g_param_spec_int ("face_width", "face_width", "Approximate width of the face",
          1, 100, DEFAULT_PROP_FACE_WIDTH, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_FACE_HEIGHT,
      g_param_spec_int ("face_height", "face_height", "Approximate height of the face",
          1, 100, DEFAULT_PROP_FACE_HEIGHT, G_PARAM_READWRITE));

  trans_class->set_caps = GST_DEBUG_FUNCPTR (gst_faceblur_set_caps);
  trans_class->transform_ip = GST_DEBUG_FUNCPTR (gst_faceblur_transform_ip);
}

static void
gst_faceblur_init (GstFaceblur * faceblur, GstFaceblurClass * g_class)
{
  GST_DEBUG_OBJECT (faceblur, "gst_faceblur_init");

  faceblur->storage = cvCreateMemStorage(0);
  faceblur->cascade = (CvHaarClassifierCascade*)cvLoad( "/usr/local/share/opencv/haarcascades/haarcascade_frontalface_alt.xml", 0, 0, 0 );

  /* properties */
  faceblur->face_width = DEFAULT_PROP_FACE_WIDTH;
  faceblur->face_height = DEFAULT_PROP_FACE_HEIGHT;
}

static void
gst_faceblur_set_property (GObject * object, guint prop_id, const GValue * value,
    GParamSpec * pspec)
{
  GstFaceblur *faceblur;

  g_return_if_fail (GST_IS_FACEBLUR (object));
  faceblur = GST_FACEBLUR (object);

  GST_DEBUG ("gst_faceblur_set_property");
  switch (prop_id) {
    case PROP_FACE_WIDTH:
      faceblur->face_width = g_value_get_int (value);
      break;
    case PROP_FACE_HEIGHT:
      faceblur->face_height = g_value_get_int (value);
      break;
    default:
      break;
  }
}

static void
gst_faceblur_get_property (GObject * object, guint prop_id, GValue * value,
    GParamSpec * pspec)
{
  GstFaceblur *faceblur;

  g_return_if_fail (GST_IS_FACEBLUR (object));
  faceblur = GST_FACEBLUR (object);

  switch (prop_id) {
    case PROP_FACE_WIDTH:
      g_value_set_int (value, faceblur->face_width);
      break;
    case PROP_FACE_HEIGHT:
      g_value_set_int (value, faceblur->face_height);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}


static gboolean
gst_faceblur_set_caps (GstBaseTransform * base, GstCaps * incaps,
    GstCaps * outcaps)
{
  GstFaceblur *this;
  GstStructure *structure;
  gboolean res;

  this = GST_FACEBLUR (base);

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
gst_faceblur_transform_ip (GstBaseTransform * base, GstBuffer * outbuf)
{
  GstFaceblur *faceblur;
  guint8 *data;
  guint size;
  int i,k,pos,j,R,G,B,a;
  IplImage *image;
  //double t;

  //t = (double)cvGetTickCount();

  faceblur = GST_FACEBLUR (base);

  if (base->passthrough)
    goto done;

  data = GST_BUFFER_DATA (outbuf);
  size = GST_BUFFER_SIZE (outbuf);

  if (size != faceblur->size)
    goto wrong_size;

  image = cvCreateImage(cvSize(faceblur->width,faceblur->height), 8, 1 );
  for(i=0;i<faceblur->height;i++)
    for(k=0;k<faceblur->width;k++) {
		pos = (i * faceblur->width + k) * 3;
		image->imageData[ pos/3 ] = (data[pos] + data[pos+1] + data[pos+2]) / 3;
	}

    CvSeq* faces = cvHaarDetectObjects( image, faceblur->cascade, faceblur->storage,
                                         1.1, 2, 0
                                          |CV_HAAR_FIND_BIGGEST_OBJECT
                                            //|CV_HAAR_DO_ROUGH_SEARCH
                                            //|CV_HAAR_DO_CANNY_PRUNING
                                            //|CV_HAAR_SCALE_IMAGE
                                            ,
                                            cvSize(faceblur->face_width, faceblur->face_height) );
  //t = (double)cvGetTickCount() - t;
  //g_print( "detection time = %gms\n", t/((double)cvGetTickFrequency()*1000.) );

	for( i = 0; i < (faces ? faces->total : 0); i++ )
	{
		CvRect* r = (CvRect*)cvGetSeqElem( faces, i );
		for(k=0;k < r->height;k++)
			for(j=0;j < r->width;j+=10) {
				R = 0; G = 0; B = 0;
				for(a=0;a<10;a++) {
					pos = ((k+r->y) * faceblur->width + j + r->x + a) * 3;
					R += data[pos]/10;
					G += data[pos+1]/10;
					B += data[pos+2]/10;
				}
				for(a=0;a<10;a++) {
					pos = ((k+r->y) * faceblur->width + j + r->x + a) * 3;
					data[pos]=R;
					data[pos+1]=G;
					data[pos+2]=B;
				}
			}
	}

done:
  return GST_FLOW_OK;

  /* ERRORS */
wrong_size:
  {
    GST_ELEMENT_ERROR (faceblur, STREAM, FORMAT,
        (NULL), ("Invalid buffer size %d, expected %d", size, faceblur->size));
    return GST_FLOW_ERROR;
  }
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  GST_DEBUG_CATEGORY_INIT (faceblur_debug, "faceblur", 0, "faceblur");

  return gst_element_register (plugin, "faceblur", GST_RANK_NONE, GST_TYPE_FACEBLUR);
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "faceblur",
    "Detects faces and adds blur on images",
    plugin_init, VERSION, "LGPL", "GSteamer OpenCV Plugins", "http://www.sandaru1.com");
