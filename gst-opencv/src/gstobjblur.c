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

#include "gstobjblur.h"
#include <string.h>
#include <math.h>

#include <gst/video/video.h>

GST_DEBUG_CATEGORY_STATIC (objblur_debug);
#define GST_CAT_DEFAULT objblur_debug

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
  PROP_X1,
  PROP_Y1,
  PROP_X2,
  PROP_Y2
      /* FILL ME */
};

#define DEFAULT_PROP_X1  0
#define DEFAULT_PROP_X2  10
#define DEFAULT_PROP_Y1  0
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
static GstFlowReturn gst_objblur_transform (GstBaseTransform * transform,
    GstBuffer * inbuf,GstBuffer * outbuf);
static gboolean
gst_objblur_get_unit_size (GstBaseTransform * btrans, GstCaps * caps,
    guint * size);


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
          0, 1000, DEFAULT_PROP_X1, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_X2,
      g_param_spec_int ("x2", "x2", "X2",
          0, 1000, DEFAULT_PROP_X2, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_Y1,
      g_param_spec_int ("y1", "y1", "Y1",
          0, 1000, DEFAULT_PROP_Y1, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_Y2,
      g_param_spec_int ("y2", "y2", "Y2",
          0, 1000, DEFAULT_PROP_Y2, G_PARAM_READWRITE));

  trans_class->set_caps = GST_DEBUG_FUNCPTR (gst_objblur_set_caps);
  trans_class->transform = GST_DEBUG_FUNCPTR (gst_objblur_transform);
  trans_class->get_unit_size = GST_DEBUG_FUNCPTR (gst_objblur_get_unit_size);
}

static void
gst_objblur_init (GstObjblur * objblur, GstObjblurClass * g_class)
{
  GstElementClass *klass = GST_ELEMENT_GET_CLASS (objblur);
  GST_DEBUG_OBJECT (objblur, "gst_objblur_init");

  objblur->done = 0;

  /* properties */
  objblur->x1 = DEFAULT_PROP_X1;
  objblur->x2 = DEFAULT_PROP_X2;
  objblur->y1 = DEFAULT_PROP_Y1;
  objblur->y2 = DEFAULT_PROP_Y2;
}

static gboolean
gst_objblur_get_unit_size (GstBaseTransform * btrans, GstCaps * caps,
    guint * size)
{
  GstObjblur *objblur;
  GstStructure *structure;
  gboolean ret = FALSE;
  gint width, height;

  objblur = GST_OBJBLUR (btrans);

  structure = gst_caps_get_structure (caps, 0);

  if (gst_structure_get_int (structure, "width", &width) &&
      gst_structure_get_int (structure, "height", &height)) {
    *size = width * height * 3;
    ret = TRUE;
    GST_DEBUG_OBJECT (objblur, "our frame size is %d bytes (%dx%d)", *size,
        width, height);
  }

  return ret;
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
    case PROP_X1:
      objblur->x1 = g_value_get_int (value);
      break;
    case PROP_Y1:
      objblur->y1 = g_value_get_int (value);
      break;
    case PROP_X2:
      objblur->x2 = g_value_get_int (value);
      break;
    case PROP_Y2:
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
    case PROP_X1:
      g_value_set_int (value, objblur->x1);
      break;
    case PROP_X2:
      g_value_set_int (value, objblur->x2);
      break;
    case PROP_Y1:
      g_value_set_int (value, objblur->y1);
      break;
    case PROP_Y2:
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

IplImage* shrinkImage(IplImage *image) {
  IplImage* temp = cvCloneImage(image);
  CvSize sourceSize = cvGetSize( temp );
  int depth = temp->depth;
  int numChannels = temp->nChannels;
  int ii;

  for(ii = 0; ii < 2; ii++ )
  {
    sourceSize.width /= 2;
    sourceSize.height /= 2;
    
    IplImage* smallSource = NULL;
    smallSource = cvCreateImage( sourceSize, depth, numChannels );
    cvPyrDown( temp, smallSource, CV_GAUSSIAN_5x5 );
    
    // prepare for next loop, if any
    cvReleaseImage( &temp );
    temp = cvCloneImage( smallSource );
    cvReleaseImage( &smallSource );
  }
  return temp;
}

static GstFlowReturn
gst_objblur_transform (GstBaseTransform * base, GstBuffer * inbuf, GstBuffer * outbuf)
{
  GstObjblur *objblur;
  guint8 *data;
  guint8 *out;
  guint size;
  int i,k,pos,j,R,G,B,a;
  IplImage *temp,*temp2,*image;
  CvRect selection;
  double m,M;
  CvPoint point1; CvPoint point2;

  //double t;
  //t = (double)cvGetTickCount();

  objblur = GST_OBJBLUR (base);

  if (base->passthrough)
    goto done;

  data = GST_BUFFER_DATA (inbuf);
  out = GST_BUFFER_DATA (outbuf);
  size = GST_BUFFER_SIZE (inbuf);

  if (size != objblur->size)
    goto wrong_size;

  	selection.x = objblur->x1;
  	selection.y = objblur->y1;
  	selection.width = objblur->x2 - objblur->x1;
  	selection.height = objblur->y2 - objblur->y1;

  temp = cvCreateImage(cvSize(objblur->width,objblur->height), 8, 3 );
  for(i=0;i<objblur->height;i++)
    for(k=0;k<objblur->width;k++) {
		pos = (i * objblur->width + k) * 3;
		temp->imageData[ pos ] = data[pos + 2];
		temp->imageData[ pos + 1 ] = data[pos + 1];
		temp->imageData[ pos + 2 ] = data[pos];
	}

  image = shrinkImage(temp);

  if (objblur->done==0) {
    objblur->done = 1;

    temp2 = cvCreateImage( cvSize(selection.width,selection.height), 8, 3 );
    cvSetImageROI( temp, selection);
    cvCopy( temp, temp2, 0 );
    cvResetImageROI(temp);
    
    objblur->icon = shrinkImage(temp2);

    cvReleaseImage( &temp2 );
  }

  CvSize resultSize;
  resultSize.width = image->width - objblur->icon->width + 1;
  resultSize.height = image->height - objblur->icon->height + 1;

  IplImage* result = cvCreateImage( resultSize, IPL_DEPTH_32F, 1 );
  
  cvMatchTemplate( image, objblur->icon, result, CV_TM_CCORR_NORMED );
  
  // release memory we don't need anymore
  cvReleaseImage( &temp );
  cvReleaseImage( &image );

  cvMinMaxLoc(result, &m, &M, &point1, &point2, NULL); 

  point2.x *= 4;
  point2.y *= 4;

  //printf("done %d %d\n",point2.x,point2.y);

  //t = (double)cvGetTickCount() - t;
  //g_print( "detection time = %gms\n", t/((double)cvGetTickFrequency()*1000.) );
  memcpy(out,data,size);

		for(k=0;k <selection.height;k++)
			for(j=0;j < selection.width;j+=20) {
				R = 0; G = 0; B = 0;
				for(a=0;a<20;a++) {
					pos = ((k+point2.y) * objblur->width + j + point2.x + a) * 3;
					R += out[pos]/20;
					G += out[pos+1]/20;
					B += out[pos+2]/20;
				}
				for(a=0;a<20;a++) {
					pos = ((k+point2.y) * objblur->width + j + point2.x + a) * 3;
					out[pos]=R;
					out[pos+1]=G;
					out[pos+2]=B;
				}
			}

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
    "Tracks objects and adds blur to them",
    plugin_init, VERSION, "LGPL", "GSteamer OpenCV Plugins", "http://www.sandaru1.com");
