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


#ifndef __GST_VIDEO_OBJEFFECT_H__
#define __GST_VIDEO_OBJEFFECT_H__

#include <gst/video/gstvideofilter.h>

#include "cv.h"
#include "colorchange.h"


G_BEGIN_DECLS

#define GST_TYPE_OBJEFFECT \
  (gst_objeffect_get_type())
#define GST_OBJEFFECT(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_OBJEFFECT,GstObjeffect))
#define GST_OBJEFFECT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_OBJEFFECT,GstObjeffectClass))
#define GST_IS_OBJEFFECT(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_OBJEFFECT))
#define GST_IS_OBJEFFECT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_OBJEFFECT))

typedef struct _GstObjeffect GstObjeffect;
typedef struct _GstObjeffectClass GstObjeffectClass;


#define EFFECT_COLORCHANGE 0
#define MAX_COUNT 500

struct _GstObjeffect
{
  GstVideoFilter videofilter;

  /* opencv */
  int done;
  IplImage *prev_grey, *prev_pyramid;
  CvPoint2D32f* points[2];
  char* status;
  int flags;
  int count;

  /* format */
  gint width;
  gint height;
  gint size;

  /* properties */
  gint x1,y1,x2,y2,effect,hue;
};

struct _GstObjeffectClass
{
  GstVideoFilterClass parent_class;
};

GType gst_objeffect_get_type(void);

G_END_DECLS

#endif /* __GST_VIDEO_GAMMA_H__ */
