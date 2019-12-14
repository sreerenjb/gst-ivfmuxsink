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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <fcntl.h>
#include <glib/gstdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "gstivfmuxsink.h"

#define IVF_FILE_HEADER_SIZE 32
#define IVF_FRAME_HEADER_SIZE 12

static gboolean gst_ivf_mux_sink_start (GstBaseSink * sink);
static gboolean gst_ivf_mux_sink_stop (GstBaseSink * sink);
static gboolean gst_ivf_mux_sink_set_caps (GstBaseSink * base_sink,
    GstCaps * caps);
static gboolean gst_ivf_mux_sink_propose_allocation (GstBaseSink *
    base_sink, GstQuery * query);
static GstFlowReturn gst_ivf_mux_sink_render (GstBaseSink * sink,
    GstBuffer * buffer);
static void gst_ivf_mux_sink_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_ivf_mux_sink_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

enum
{
  PROP_0,
  PROP_FOURCC,
  PROP_RAW_LOCATION,
  PROP_NUM_FRAMES
};

static GstStaticPadTemplate gst_ivf_mux_sink_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY"));

/* class initialization */

#define CAT_PERFORMANCE _get_perf_category()
static inline GstDebugCategory *
_get_perf_category (void)
{
  static GstDebugCategory *cat = NULL;

  if (g_once_init_enter (&cat)) {
    GstDebugCategory *c;

    GST_DEBUG_CATEGORY_GET (c, "GST_PERFORMANCE");
    g_once_init_leave (&cat, c);
  }
  return cat;
}

GST_DEBUG_CATEGORY_STATIC (gst_ivf_mux_sink_debug);
#define GST_CAT_DEFAULT gst_ivf_mux_sink_debug

#define gst_ivf_mux_sink_parent_class parent_class
G_DEFINE_TYPE_WITH_CODE (GstIvfMuxSink, gst_ivf_mux_sink,
    GST_TYPE_VIDEO_SINK, GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT,
        "ivfmuxsink", 0, "ivf mux sink"));

static void
gst_ivf_mux_sink_class_init (GstIvfMuxSinkClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);
  GstBaseSinkClass *base_sink_class = GST_BASE_SINK_CLASS (klass);

  gobject_class->set_property = gst_ivf_mux_sink_set_property;
  gobject_class->get_property = gst_ivf_mux_sink_get_property;
  base_sink_class->start = GST_DEBUG_FUNCPTR (gst_ivf_mux_sink_start);
  base_sink_class->stop = GST_DEBUG_FUNCPTR (gst_ivf_mux_sink_stop);
  base_sink_class->set_caps = gst_ivf_mux_sink_set_caps;
  base_sink_class->render = gst_ivf_mux_sink_render;

  g_object_class_install_property (gobject_class, PROP_FOURCC,
      g_param_spec_string ("fourcc", "Forucc for codec (vp8, vp9 or av1)",
          "Fourcc",
          NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_RAW_LOCATION,
      g_param_spec_string ("dump-location", "File Location",
          "Location of the file to write ivf muxed stream",
          NULL, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_NUM_FRAMES,
      g_param_spec_uint ("num-frames",
          "Total Frames",
          "Number of Frames in the Stream",
          0, 360000, 30,
          G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS));

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_ivf_mux_sink_sink_template));

  gst_element_class_set_static_metadata (element_class, "IVF Mux sink",
      "Debug/Sink", "Stuff ivf header into elementory stream",
      "Sreerenj Balachandran <sreerenj.balachandran@intel.com>");
}

static void
gst_ivf_mux_sink_init (GstIvfMuxSink * ivfmuxsink)
{
  gst_base_sink_set_sync (GST_BASE_SINK (ivfmuxsink), FALSE);
  ivfmuxsink->raw_file_name = "sample.ivf";
  ivfmuxsink->fourcc = "VP80";
  ivfmuxsink->fd = -1;
  ivfmuxsink->state = GST_IVF_FILE_START;
  ivfmuxsink->num_frames = 30;
  ivfmuxsink->width = 320;
  ivfmuxsink->height = 240;
  ivfmuxsink->fps_n = 30;
  ivfmuxsink->fps_d = 1;
  ivfmuxsink->pts = 0;
}

static void
gst_ivf_mux_sink_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstIvfMuxSink *ivfmuxsink = GST_IVF_MUX_SINK (object);

  switch (prop_id) {
    case PROP_RAW_LOCATION:
      ivfmuxsink->raw_file_name = g_strdup (g_value_get_string (value));
      break;
    case PROP_FOURCC:
      ivfmuxsink->fourcc = g_strdup (g_value_get_string (value));
      break;
    case PROP_NUM_FRAMES:
      ivfmuxsink->num_frames = g_value_get_uint (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_ivf_mux_sink_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstIvfMuxSink *ivfmuxsink = GST_IVF_MUX_SINK (object);

  switch (prop_id) {
    case PROP_FOURCC:
      g_value_set_string (value, ivfmuxsink->fourcc);
      break;
    case PROP_RAW_LOCATION:
      g_value_set_string (value, ivfmuxsink->raw_file_name);
      break;
    case PROP_NUM_FRAMES:
      g_value_set_uint (value, ivfmuxsink->num_frames);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gboolean
open_raw_file (GstIvfMuxSink * ivfmuxsink)
{
  GError *err;

  if (ivfmuxsink->fd != -1)
    return TRUE;
  else if (ivfmuxsink->raw_file_name)
    ivfmuxsink->fd = g_open (ivfmuxsink->raw_file_name, O_WRONLY | O_CREAT,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
  else
    ivfmuxsink->fd =
        g_file_open_tmp ("tmp_XXXXXX.yuv", &ivfmuxsink->raw_file_name, &err);

  if (ivfmuxsink->fd == -1) {
    GST_ELEMENT_ERROR (ivfmuxsink, RESOURCE, OPEN_WRITE,
        ("failed to create output file"),
        ("reason: %s", err ? err->message : ""));
    g_error_free (err);
    return FALSE;
  }

  GST_INFO_OBJECT (ivfmuxsink, "raw file name: %s",
      ivfmuxsink->raw_file_name);
  return TRUE;
}

static gboolean
gst_ivf_mux_sink_start (GstBaseSink * sink)
{
  GstIvfMuxSink *ivfmuxsink = GST_IVF_MUX_SINK (sink);

  if (!open_raw_file (ivfmuxsink))
    return FALSE;

  return TRUE;
}


static gboolean
gst_ivf_mux_sink_stop (GstBaseSink * sink)
{
  GstIvfMuxSink *ivfmuxsink = GST_IVF_MUX_SINK (sink);

  if (ivfmuxsink->fd != -1) {
    fsync (ivfmuxsink->fd);
    close (ivfmuxsink->fd);
  }

  g_clear_pointer (&ivfmuxsink->raw_file_name, g_free);

  g_clear_pointer (&ivfmuxsink->data, g_free);
  ivfmuxsink->data_size = 0;

  return TRUE;
}

static gboolean
gst_ivf_mux_sink_set_caps (GstBaseSink * base_sink, GstCaps * caps)
{
  GstIvfMuxSink *ivfmuxsink = GST_IVF_MUX_SINK (base_sink);
  GstStructure *st;

  if (!caps)
    return FALSE;

  st = gst_caps_get_structure (caps, 0);
  if (!st)
    return FALSE;

  gst_structure_get_int (st, "width", &ivfmuxsink->width);
  gst_structure_get_int (st, "height", &ivfmuxsink->height);
  gst_structure_get_fraction (st, "framerate", &ivfmuxsink->fps_n, &ivfmuxsink->fps_d);

  return TRUE;
}

static GstBuffer *
pack_ivf_header (GstIvfMuxSink *ivfmuxsink, GstBuffer *buffer)
{
  GstBuffer *new_buf;
  GstByteWriter bs;
  gsize data_size = gst_buffer_get_size (buffer);
  GstClockTime frame_duration = gst_util_uint64_scale (1,
      GST_SECOND * ivfmuxsink->fps_d, ivfmuxsink->fps_n);
  
  if (ivfmuxsink->state == GST_IVF_FILE_START) {
    gst_byte_writer_init_with_size (&bs, (IVF_FILE_HEADER_SIZE + IVF_FRAME_HEADER_SIZE + data_size), FALSE);

    gst_byte_writer_put_data (&bs, "DKIF", 4);
    gst_byte_writer_put_uint16_le (&bs, 0);
    gst_byte_writer_put_uint16_le (&bs, 32);
    gst_byte_writer_put_data (&bs, ivfmuxsink->fourcc, 4);
    gst_byte_writer_put_uint16_le (&bs, ivfmuxsink->width);
    gst_byte_writer_put_uint16_le (&bs, ivfmuxsink->height);
    gst_byte_writer_put_uint32_le (&bs, ivfmuxsink->num_frames);
    gst_byte_writer_put_uint32_le (&bs, ivfmuxsink->fps_d);
    gst_byte_writer_put_uint32_le (&bs, ivfmuxsink->fps_n);
    gst_byte_writer_put_uint32_le (&bs, 0);
  } else{
    gst_byte_writer_init_with_size (&bs, (IVF_FRAME_HEADER_SIZE + data_size), FALSE);
  }

  {
    gst_byte_writer_put_uint32_le (&bs, data_size);
    gst_byte_writer_put_uint64_le (&bs, GST_CLOCK_TIME_NONE);
  }  

  gst_byte_writer_put_buffer (&bs, buffer, 0, data_size);

  new_buf = gst_byte_writer_reset_and_get_buffer (&bs);
  if (!new_buf)
    return NULL;

  //pts
  ivfmuxsink->pts += frame_duration;
  return new_buf;
}

static GstFlowReturn
gst_ivf_mux_sink_render (GstBaseSink * sink, GstBuffer * buffer)
{
  GstIvfMuxSink *ivfmuxsink = GST_IVF_MUX_SINK (sink);
  gchar *csum;
  GstVideoFrame frame;
  guint8 *data, *hdr_data;
  gsize size;
  GstMapInfo info;
  GstBuffer *new_buf;

  new_buf = pack_ivf_header (ivfmuxsink, buffer);

  if (!gst_buffer_map (new_buf, &info, GST_MAP_READ))
  {
    GST_ERROR ("Failed to map the final buffer");
    return GST_FLOW_ERROR;
  }

  size = info.size;
  do {
      ssize_t written = write (ivfmuxsink->fd, info.data, size);
      if (written == -1) {
        GST_ELEMENT_ERROR (ivfmuxsink, RESOURCE, WRITE,
            ("Failed to write to the file: %s", g_strerror (errno)), (NULL));
        return GST_FLOW_ERROR;
      } else if (written < info.size) {
        data = &data[written + 1];
        size = size - written;
      } else if (written == size) {
        break;
      }
    } while (TRUE);


  gst_buffer_unmap(new_buf, &info);
  gst_buffer_unref(new_buf);
  ivfmuxsink->state = GST_IVF_FRAME_START;
  return GST_FLOW_OK;
}
