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


#ifndef __GST_VIDEO_OBJBLUR_H__
#define __GST_VIDEO_OBJBLUR_H__

#include <gst/video/gstvideofilter.h>

#include "cv.h"


G_BEGIN_DECLS

#define GST_TYPE_OBJBLUR \
  (gst_objblur_get_type())
#define GST_OBJBLUR(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_OBJBLUR,GstObjblur))
#define GST_OBJBLUR_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_OBJBLUR,GstObjblurClass))
#define GST_IS_OBJBLUR(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_OBJBLUR))
#define GST_IS_OBJBLUR_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_OBJBLUR))

typedef struct _GstObjblur GstObjblur;
typedef struct _GstObjblurClass GstObjblurClass;

/**
 * GstGamma:
 *
 * Opaque data structure.
 */
struct _GstObjblur
{
  GstVideoFilter videofilter;

  /* opencv */
  int done;
  IplImage *icon;

  /* format */
  gint width;
  gint height;
  gint size;

  /* properties */
  gint x1,y1,x2,y2;
};

struct _GstObjblurClass
{
  GstVideoFilterClass parent_class;
};

GType gst_objblur_get_type(void);

G_END_DECLS

#endif /* __GST_VIDEO_GAMMA_H__ */
