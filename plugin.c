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

#include <gst/gst.h>
#include "gstivfmuxsink.h"

static gboolean
plugin_init (GstPlugin * plugin)
{
  return gst_element_register (plugin, "ivfmuxsink",
      GST_RANK_NONE, GST_TYPE_IVF_MUX_SINK);
}

GstPluginDesc gst_plugin_desc = {
  .major_version = GST_VERSION_MAJOR,
  .minor_version = GST_VERSION_MINOR,
  .name = "libgstivfmuxsink",
  .description =  "Sink element to attach ivf header in encoded frames",
  .plugin_init = plugin_init,
  .version = "0.1",
  .license = "LGPL",
  .source = "gst-ivfmuxsink",
  .package = "gst-ivfmuxsink",
  .origin = "https://github.com/sreerenjb/gst-ivfmuxsink",
};
