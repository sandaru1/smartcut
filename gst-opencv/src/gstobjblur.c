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

  objblur->storage = cvCreateMemStorage(0);
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

CvScalar hsv2rgb( float hue )
{
    int rgb[3], p, sector;
    static const int sector_data[][3]=
        {{0,2,1}, {1,2,0}, {1,0,2}, {2,0,1}, {2,1,0}, {0,1,2}};
    hue *= 0.033333333333333333333333333333333f;
    sector = cvFloor(hue);
    p = cvRound(255*(hue - sector));
    p ^= sector & 1 ? 255 : 0;

    rgb[sector_data[sector][0]] = 255;
    rgb[sector_data[sector][1]] = 0;
    rgb[sector_data[sector][2]] = p;

    return cvScalar(rgb[2], rgb[1], rgb[0],0);
}

static GstFlowReturn
gst_objblur_transform (GstBaseTransform * base, GstBuffer * inbuf, GstBuffer * outbuf)
{
  GstObjblur *objblur;
  guint8 *data;
  guint8 *out;
  guint size;
  int i,k,pos,j,R,G,B,a;
  IplImage *image;
  //double t;
  int hdims = 16;
  float hranges_arr[] = {0,180};
  float* hranges = hranges_arr;
  int vmin = 10, vmax = 256, smin = 30;
	int _vmin = vmin, _vmax = vmax;
  int bin_w,c;

  //t = (double)cvGetTickCount();

  objblur = GST_OBJBLUR (base);

  if (base->passthrough)
    goto done;

  data = GST_BUFFER_DATA (inbuf);
  out = GST_BUFFER_DATA (outbuf);
  size = GST_BUFFER_SIZE (inbuf);

  if (size != objblur->size)
    goto wrong_size;
  image = cvCreateImage(cvSize(objblur->width,objblur->height), 8, 3 );
  for(i=0;i<objblur->height;i++)
    for(k=0;k<objblur->width;k++) {
		pos = (i * objblur->width + k) * 3;
		image->imageData[ pos ] = data[pos + 2];
		image->imageData[ pos + 1 ] = data[pos + 1];
		image->imageData[ pos + 2 ] = data[pos];
	}

  if (objblur->done==0) {
    objblur->done = 1;
  	objblur->selection.x = objblur->x1;
  	objblur->selection.y = objblur->y1;
  	objblur->selection.width = objblur->x2 - objblur->x1;
  	objblur->selection.height = objblur->y2 - objblur->y1;

    objblur->hsv = cvCreateImage( cvGetSize(image), 8, 3 );
    objblur->hue = cvCreateImage( cvGetSize(image), 8, 1 );
    objblur->mask = cvCreateImage( cvGetSize(image), 8, 1 );
    objblur->backproject = cvCreateImage( cvGetSize(image), 8, 1 );
    objblur->hist = cvCreateHist( 1, &hdims, CV_HIST_ARRAY, &hranges, 1 );
    objblur->histimg = cvCreateImage( cvSize(320,200), 8, 3 );
    cvZero( objblur->histimg );

	  cvCvtColor( image, objblur->hsv, CV_BGR2HSV );
    cvCvtColor( image, objblur->hsv, CV_BGR2HSV );

	  cvInRangeS( objblur->hsv, cvScalar(0,smin,MIN(_vmin,_vmax),0),
                        cvScalar(180,256,MAX(_vmin,_vmax),0), objblur->mask );
	  cvSplit( objblur->hsv, objblur->hue, 0, 0, 0 );

    float max_val = 0.f;
    cvSetImageROI( objblur->hue, objblur->selection );
    cvSetImageROI( objblur->mask, objblur->selection );
    cvCalcHist( &(objblur->hue), objblur->hist, 0, objblur->mask );
    cvGetMinMaxHistValue( objblur->hist, 0, &max_val, 0, 0 );
	  cvConvertScale( objblur->hist->bins, objblur->hist->bins, max_val ? 255. / max_val : 0., 0 );
	  cvResetImageROI( objblur->hue );
    cvResetImageROI( objblur->mask );

    cvZero( objblur->histimg );
    bin_w = objblur->histimg->width / hdims;
    for( i = 0; i < hdims; i++ )
    {
        int val = cvRound( cvGetReal1D(objblur->hist->bins,i) * objblur->histimg->height/255 );
        CvScalar color = hsv2rgb(i*180.f/hdims);
        cvRectangle( objblur->histimg, cvPoint(i*bin_w,objblur->histimg->height),
                        cvPoint((i+1)*bin_w,objblur->histimg->height - val),
                        color, -1, 8, 0 );
    }

  }

	cvCvtColor( image, objblur->hsv, CV_BGR2HSV );
  cvInRangeS( objblur->hsv, cvScalar(0,smin,MIN(_vmin,_vmax),0),
                  cvScalar(180,256,MAX(_vmin,_vmax),0), objblur->mask );
  cvSplit( objblur->hsv, objblur->hue, 0, 0, 0 );

  cvCalcBackProject( &(objblur->hue), objblur->backproject, objblur->hist );
 	cvAnd( objblur->backproject, objblur->mask, objblur->backproject, 0 );
  cvCamShift( objblur->backproject, objblur->selection,
    	            cvTermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ),
    	            &(objblur->track_comp), &(objblur->track_box) );

  objblur->selection = objblur->track_comp.rect;

	CvRect roi;
	roi.x = (int)objblur->track_box.center.x - objblur->track_box.size.width/2;
	roi.y = (int)objblur->track_box.center.y - objblur->track_box.size.height/2;
	roi.width = (int)objblur->track_box.size.width;
	roi.height = (int)objblur->track_box.size.height;

//  g_printf("%d %d %d %d\n",roi.x,roi.y,roi.width,roi.height);

  //t = (double)cvGetTickCount() - t;
  //g_print( "detection time = %gms\n", t/((double)cvGetTickFrequency()*1000.) );
  memcpy(out,data,size);

		for(k=0;k < roi.height;k++)
			for(j=0;j < roi.width;j+=10) {
				R = 0; G = 0; B = 0;
				for(a=0;a<10;a++) {
					pos = ((k+roi.y) * objblur->width + j + roi.x + a) * 3;
					R += out[pos]/10;
					G += out[pos+1]/10;
					B += out[pos+2]/10;
				}
				for(a=0;a<10;a++) {
					pos = ((k+roi.y) * objblur->width + j + roi.x + a) * 3;
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
