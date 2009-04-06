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


#ifndef __GST_VIDEO_FACEBLUR_H__
#define __GST_VIDEO_FACEBLUR_H__

#include <gst/video/gstvideofilter.h>

#include "cv.h"


G_BEGIN_DECLS

#define GST_TYPE_FACEBLUR \
  (gst_faceblur_get_type())
#define GST_FACEBLUR(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_FACEBLUR,GstFaceblur))
#define GST_FACEBLUR_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_FACEBLUR,GstFaceblurClass))
#define GST_IS_FACEBLUR(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_FACEBLUR))
#define GST_IS_FACEBLUR_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_FACEBLUR))

typedef struct _GstFaceblur GstFaceblur;
typedef struct _GstFaceblurClass GstFaceblurClass;

/**
 * GstGamma:
 *
 * Opaque data structure.
 */
struct _GstFaceblur
{
  GstVideoFilter videofilter;

  /* opencv */
  CvMemStorage* storage;
  CvHaarClassifierCascade* cascade;

  /* format */
  gint width;
  gint height;
  gint size;

  /* properties */
  gint face_width;
  gint face_height;
};

struct _GstFaceblurClass
{
  GstVideoFilterClass parent_class;
};

GType gst_faceblur_get_type(void);

G_END_DECLS

#endif /* __GST_VIDEO_GAMMA_H__ */
