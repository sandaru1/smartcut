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

#include "gstobjeffect.h"
#include <string.h>
#include <math.h>

#include <gst/video/video.h>

GST_DEBUG_CATEGORY_STATIC (objeffect_debug);
#define GST_CAT_DEFAULT objeffect_debug

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
  PROP_Y2,
  PROP_HUE,
  PROP_EFFECT
      /* FILL ME */
};

#define DEFAULT_PROP_X1  0
#define DEFAULT_PROP_X2  10
#define DEFAULT_PROP_Y1  0
#define DEFAULT_PROP_Y2  10
#define DEFAULT_PROP_EFFECT 0
#define DEFAULT_PROP_HUE -50

static const GstElementDetails objeffect_details =
GST_ELEMENT_DETAILS ("Video Object Effect",
    "Filter/Effect/Video",
    "Tracks objects and adds a effect to them on video",
    "Sandaruwan Gunathilake <sandaruwan@gunathilake.com>");

static GstStaticPadTemplate gst_objeffect_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_RGB)
    );

static GstStaticPadTemplate gst_objeffect_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_RGB)
    );

static void gst_objeffect_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_objeffect_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static gboolean gst_objeffect_set_caps (GstBaseTransform * base, GstCaps * incaps,
    GstCaps * outcaps);
static GstFlowReturn gst_objeffect_transform (GstBaseTransform * transform,
    GstBuffer * inbuf,GstBuffer * outbuf);
static gboolean
gst_objeffect_get_unit_size (GstBaseTransform * btrans, GstCaps * caps,
    guint * size);


GST_BOILERPLATE (GstObjeffect, gst_objeffect, GstVideoFilter, GST_TYPE_VIDEO_FILTER);


static void
gst_objeffect_base_init (gpointer g_class)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (g_class);

  gst_element_class_set_details (element_class, &objeffect_details);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_objeffect_sink_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_objeffect_src_template));
}

static void
gst_objeffect_class_init (GstObjeffectClass * g_class)
{
  GObjectClass *gobject_class;
  GstBaseTransformClass *trans_class;

  gobject_class = G_OBJECT_CLASS (g_class);
  trans_class = GST_BASE_TRANSFORM_CLASS (g_class);

  gobject_class->set_property = gst_objeffect_set_property;
  gobject_class->get_property = gst_objeffect_get_property;

  g_object_class_install_property (gobject_class, PROP_X1,
      g_param_spec_int ("x1", "x1", "X1",
          0, 100000, DEFAULT_PROP_X1, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_X2,
      g_param_spec_int ("x2", "x2", "X2",
          0, 100000, DEFAULT_PROP_X2, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_Y1,
      g_param_spec_int ("y1", "y1", "Y1",
          0, 100000, DEFAULT_PROP_Y1, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_Y2,
      g_param_spec_int ("y2", "y2", "Y2",
          0, 100000, DEFAULT_PROP_Y2, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_HUE,
      g_param_spec_int ("hue", "hue", "HUE",
          -255, 255, DEFAULT_PROP_HUE, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_EFFECT,
      g_param_spec_int ("effect", "effect", "EFFECT",
          0, 1, DEFAULT_PROP_EFFECT, G_PARAM_READWRITE));


  trans_class->set_caps = GST_DEBUG_FUNCPTR (gst_objeffect_set_caps);
  trans_class->transform = GST_DEBUG_FUNCPTR (gst_objeffect_transform);
  trans_class->get_unit_size = GST_DEBUG_FUNCPTR (gst_objeffect_get_unit_size);
}

static void
gst_objeffect_init (GstObjeffect * objeffect, GstObjeffectClass * g_class)
{
  GstElementClass *klass = GST_ELEMENT_GET_CLASS (objeffect);
  GST_DEBUG_OBJECT (objeffect, "gst_objeffect_init");

  objeffect->done = 0;

  /* properties */
  objeffect->x1 = DEFAULT_PROP_X1;
  objeffect->x2 = DEFAULT_PROP_X2;
  objeffect->y1 = DEFAULT_PROP_Y1;
  objeffect->y2 = DEFAULT_PROP_Y2;
  objeffect->effect = DEFAULT_PROP_EFFECT;
  objeffect->hue = DEFAULT_PROP_HUE;
}

static gboolean
gst_objeffect_get_unit_size (GstBaseTransform * btrans, GstCaps * caps,
    guint * size)
{
  GstObjeffect *objeffect;
  GstStructure *structure;
  gboolean ret = FALSE;
  gint width, height;

  objeffect = GST_OBJEFFECT (btrans);

  structure = gst_caps_get_structure (caps, 0);

  if (gst_structure_get_int (structure, "width", &width) &&
      gst_structure_get_int (structure, "height", &height)) {
    *size = width * height * 3;
    ret = TRUE;
    GST_DEBUG_OBJECT (objeffect, "our frame size is %d bytes (%dx%d)", *size,
        width, height);
  }

  return ret;
}

static void
gst_objeffect_set_property (GObject * object, guint prop_id, const GValue * value,
    GParamSpec * pspec)
{
  GstObjeffect *objeffect;

  g_return_if_fail (GST_IS_OBJEFFECT (object));
  objeffect = GST_OBJEFFECT (object);

  GST_DEBUG ("gst_objeffect_set_property");
  switch (prop_id) {
    case PROP_X1:
      objeffect->x1 = g_value_get_int (value);
      break;
    case PROP_Y1:
      objeffect->y1 = g_value_get_int (value);
      break;
    case PROP_X2:
      objeffect->x2 = g_value_get_int (value);
      break;
    case PROP_Y2:
      objeffect->y2 = g_value_get_int (value);
      break;
    case PROP_EFFECT:
      objeffect->effect = g_value_get_int (value);
      break;
    case PROP_HUE:
      objeffect->hue = g_value_get_int (value);
      break;
    default:
      break;
  }
}

static void
gst_objeffect_get_property (GObject * object, guint prop_id, GValue * value,
    GParamSpec * pspec)
{
  GstObjeffect *objeffect;

  g_return_if_fail (GST_IS_OBJEFFECT (object));
  objeffect = GST_OBJEFFECT (object);

  switch (prop_id) {
    case PROP_X1:
      g_value_set_int (value, objeffect->x1);
      break;
    case PROP_X2:
      g_value_set_int (value, objeffect->x2);
      break;
    case PROP_Y1:
      g_value_set_int (value, objeffect->y1);
      break;
    case PROP_Y2:
      g_value_set_int (value, objeffect->y2);
      break;
    case PROP_EFFECT:
      g_value_set_int (value, objeffect->effect);
      break;
    case PROP_HUE:
      g_value_set_int (value, objeffect->hue);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}


static gboolean
gst_objeffect_set_caps (GstBaseTransform * base, GstCaps * incaps,
    GstCaps * outcaps)
{
  GstObjeffect *this;
  GstStructure *structure;
  gboolean res;

  this = GST_OBJEFFECT (base);

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
gst_objeffect_transform (GstBaseTransform * base, GstBuffer * inbuf, GstBuffer * outbuf)
{
  GstObjeffect *objeffect;
  guint8 *data;
  guint8 *out;
  guint size;
  CvRect selection;
  IplImage *g,*eig,*temp,*grey,*image,*pyramid,*swap_temp;
  int i, j, R, G, B, pos, k;
  double quality = 0.1;
  double min_distance = 10;
  int win_size = 10;
  int nc = 0;
  float X=0,Y=0;

  objeffect = GST_OBJEFFECT (base);

  if (base->passthrough)
    goto done;

  data = GST_BUFFER_DATA (inbuf);
  out = GST_BUFFER_DATA (outbuf);
  size = GST_BUFFER_SIZE (inbuf);

  if (size != objeffect->size)
    goto wrong_size;

	selection.x = objeffect->x1;
	selection.y = objeffect->y1;
	selection.width = objeffect->x2 - objeffect->x1;
	selection.height = objeffect->y2 - objeffect->y1;

  image = cvCreateImage(cvSize(objeffect->width,objeffect->height), 8, 3 );
  for(i=0;i<objeffect->height;i++)
    for(k=0;k<objeffect->width;k++) {
		pos = (i * objeffect->width + k) * 3;
		image->imageData[ pos ] = data[pos + 2];
		image->imageData[ pos + 1 ] = data[pos + 1];
		image->imageData[ pos + 2 ] = data[pos];
	}

  grey = cvCreateImage( cvGetSize(image), 8, 1 );
  pyramid = cvCreateImage( cvGetSize(image), 8, 1 );

  cvCvtColor( image, grey, CV_BGR2GRAY );

  if (objeffect->done==0) {
    objeffect->prev_grey = cvCreateImage( cvGetSize(image), 8, 1 );
    objeffect->prev_pyramid = cvCreateImage( cvGetSize(image), 8, 1 );
    objeffect->points[0] = (CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(objeffect->points[0][0]));
    objeffect->points[1] = (CvPoint2D32f*)cvAlloc(MAX_COUNT*sizeof(objeffect->points[0][0]));
    objeffect->status = (char*)cvAlloc(MAX_COUNT);
    objeffect->flags = 0;
    objeffect->done = 1;

    g = cvCreateImage( cvSize(selection.width,selection.height), 8, 1 );

    for( i = 0 ; i < selection.height ; i++ ) {
      for( j = 0 ; j < selection.width ; j++ ) {
        pos = ((i+selection.y)*selection.width + j+selection.x) * 3;
        R = image->imageData[pos];
        G = image->imageData[pos + 1];
        B = image->imageData[pos + 2];
        g->imageData[i*selection.width+j] = (R + G + B)/3;
      }
    }

    eig = cvCreateImage( cvGetSize(g), 32, 1 );
    temp = cvCreateImage( cvGetSize(g), 32, 1 );

    objeffect->count = MAX_COUNT;
    cvGoodFeaturesToTrack( g, eig, temp, objeffect->points[1], &(objeffect->count),
                              quality, min_distance, 0, 3, 0, 0.04 );
    cvReleaseImage( &eig );
    cvReleaseImage( &temp );

    for(i=0;i<objeffect->count;i++) {
      objeffect->points[1][i].x += selection.x;
      objeffect->points[1][i].y += selection.y;
    }
  } else if( objeffect->count > 0 ) {
    cvCalcOpticalFlowPyrLK( objeffect->prev_grey, grey, objeffect->prev_pyramid, pyramid,
        objeffect->points[0], objeffect->points[1], objeffect->count, cvSize(win_size,win_size), 3, objeffect->status, 0,
        cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03), objeffect->flags );
    objeffect->flags |= CV_LKFLOW_PYR_A_READY;
    for( i = k = 0; i < objeffect->count; i++ )
    {                
      if( !objeffect->status[i] )
        continue;
        
      objeffect->points[1][k++] = objeffect->points[1][i];
    }
    objeffect->count = k;
  }

  CV_SWAP( objeffect->prev_grey, grey, swap_temp );
  CV_SWAP( objeffect->prev_pyramid, pyramid, swap_temp );

  for(i=0;i<objeffect->count;i++) {
    X += objeffect->points[1][i].x;
    Y += objeffect->points[1][i].y;
  }
  X /= (double)objeffect->count;
  Y /= (double)objeffect->count;

  for(i=0;i<objeffect->count;i++) {
    float dx = abs(objeffect->points[1][i].x-X);
    float dy = abs(objeffect->points[1][i].y-Y);
    if (dx<(selection.width/2) && dy<(selection.height/2)) {
      objeffect->points[0][nc++] = objeffect->points[1][i];
    }
  }
  objeffect->count = nc;

  memcpy(out,data,size);
  /*
  for(i=-10;i<10;i++)
    for(k=-10;k<10;k++) {
      int pos = (((int)Y+k)*objeffect->width+((int)X+i))*3;
      out[pos] = 255;
      out[pos+1] = 0;
      out[pos+2] = 0;
    }*/
  switch(objeffect->effect) {
    case EFFECT_COLORCHANGE:
      change_color(image,(int)X,(int)Y,selection.width,selection.height,objeffect->hue,out);
      break;
  }

done:
  return GST_FLOW_OK;

  /* ERRORS */
wrong_size:
  {
    GST_ELEMENT_ERROR (objeffect, STREAM, FORMAT,
        (NULL), ("Invalid buffer size %d, expected %d", size, objeffect->size));
    return GST_FLOW_ERROR;
  }
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  GST_DEBUG_CATEGORY_INIT (objeffect_debug, "objeffect", 0, "objeffect");

  return gst_element_register (plugin, "objeffect", GST_RANK_NONE, GST_TYPE_OBJEFFECT);
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "objeffect",
    "Tracks objects and adds blur to them",
    plugin_init, VERSION, "LGPL", "GSteamer OpenCV Plugins", "http://www.sandaru1.com");
