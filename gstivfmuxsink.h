/* GStreamer
 * Copyright (C) 2019 Sreerenj Balachandran <sreerenj.balachandran@intel.com>
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
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _GST_IVF_MUX_SINK_H_
#define _GST_IVF_MUX_SINK_H_

#include <gst/gst.h>
#include <gst/video/gstvideosink.h>
#include <gst/video/video.h>
#include <gst/base/base.h>

G_BEGIN_DECLS

#define GST_TYPE_IVF_MUX_SINK   (gst_ivf_mux_sink_get_type())
#define GST_IVF_MUX_SINK(obj)   (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_IVF_MUX_SINK,GstIvfMuxSink))
#define GST_IVF_MUX_SINK_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_IVF_MUX_SINK,GstIvfMuxSinkClass))
#define GST_IS_IVF_MUX_SINK(obj)   (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_IVF_MUX_SINK))
#define GST_IS_IVF_MUX_SINK_CLASS(obj)   (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_IVF_MUX_SINK))

typedef struct _GstIvfMuxSink GstIvfMuxSink;
typedef struct _GstIvfMuxSinkClass GstIvfMuxSinkClass;

typedef enum {
  GST_IVF_FILE_START,
  GST_IVF_FRAME_START
} GstIvfState;

struct _GstIvfMuxSink
{
  GstVideoSink parent;
  GstVideoInfo vinfo;

  /* properties */
  gchar *fourcc;
  gchar *raw_file_name;
  gint fd;
  guint width;
  guint height;
  guint fps_n;
  guint fps_d;
  guint num_frames;
  GstClockTime pts;
  guint8 *data;
  gsize data_size;

  GstIvfState state;
};

struct _GstIvfMuxSinkClass
{
  GstVideoSinkClass parent_class;
};

GType gst_ivf_mux_sink_get_type (void);

G_END_DECLS

#endif
