/* GStreamer
 * Copyright (C) 2020 Attila T. <tokes_atti@yahoo.com>
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
 * Free Software Foundation, Inc., 51 Franklin Street, Suite 500,
 * Boston, MA 02110-1335, USA.
 */
/**
 * SECTION:element-gstvaissd
 *
 * Vitis AI based Single Shot Detector (SSD).
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v ... ! vaissd model=<model> ! ...
 * ]|
 * Describe what the pipeline does.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideofilter.h>
#include "gstvaissd.h"

/* OpenCV header files */
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

/* Vitis-AI-Library specific header files */
#include <vitis/ai/ssd.hpp>

GST_DEBUG_CATEGORY_STATIC (gst_vaissd_debug_category);
#define GST_CAT_DEFAULT gst_vaissd_debug_category

/* prototypes */


static void gst_vaissd_set_property (GObject * object,
    guint property_id, const GValue * value, GParamSpec * pspec);
static void gst_vaissd_get_property (GObject * object,
    guint property_id, GValue * value, GParamSpec * pspec);
static void gst_vaissd_dispose (GObject * object);
static void gst_vaissd_finalize (GObject * object);

static gboolean gst_vaissd_start (GstBaseTransform * trans);
static gboolean gst_vaissd_stop (GstBaseTransform * trans);
static gboolean gst_vaissd_set_info (GstVideoFilter * filter, GstCaps * incaps,
    GstVideoInfo * in_info, GstCaps * outcaps, GstVideoInfo * out_info);
static GstFlowReturn gst_vaissd_transform_frame (GstVideoFilter * filter,
    GstVideoFrame * inframe, GstVideoFrame * outframe);
static GstFlowReturn gst_vaissd_transform_frame_ip (GstVideoFilter * filter,
    GstVideoFrame * frame);

enum
{
  PROP_0,
  PROP_MODEL_NAME
};

/* pad templates */

/* Input format */
#define VIDEO_SRC_CAPS \
    GST_VIDEO_CAPS_MAKE("{ BGR }")

/* Output format */
#define VIDEO_SINK_CAPS \
    GST_VIDEO_CAPS_MAKE("{ BGR }")


/* class initialization */

G_DEFINE_TYPE_WITH_CODE (GstVaissd, gst_vaissd, GST_TYPE_VIDEO_FILTER,
  GST_DEBUG_CATEGORY_INIT (gst_vaissd_debug_category, "vaissd", 0,
  "debug category for vaissd element"));

static void
gst_vaissd_class_init (GstVaissdClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstBaseTransformClass *base_transform_class = GST_BASE_TRANSFORM_CLASS (klass);
  GstVideoFilterClass *video_filter_class = GST_VIDEO_FILTER_CLASS (klass);

  /* Setting up pads and setting metadata should be moved to
     base_class_init if you intend to subclass this class. */
  gst_element_class_add_pad_template (GST_ELEMENT_CLASS(klass),
      gst_pad_template_new ("src", GST_PAD_SRC, GST_PAD_ALWAYS,
        gst_caps_from_string (VIDEO_SRC_CAPS ",width = (int) [1, 640], height = (int) [1, 360]")));
  gst_element_class_add_pad_template (GST_ELEMENT_CLASS(klass),
      gst_pad_template_new ("sink", GST_PAD_SINK, GST_PAD_ALWAYS,
        gst_caps_from_string (VIDEO_SINK_CAPS ", width = (int) [1, 640], height = (int) [1, 360]")));

  gst_element_class_set_static_metadata (GST_ELEMENT_CLASS(klass),
      "Single Shot Detector based on the Vitis-AI-Library", 
      "Video Filter", 
      "Single Shot Detector (SSD)",
      "Attila T. <tokes_atti@yahoo.com>");
      
  gobject_class->set_property = gst_vaissd_set_property;
  gobject_class->get_property = gst_vaissd_get_property;
  gobject_class->dispose = gst_vaissd_dispose;
  gobject_class->finalize = gst_vaissd_finalize;
  base_transform_class->start = GST_DEBUG_FUNCPTR (gst_vaissd_start);
  base_transform_class->stop = GST_DEBUG_FUNCPTR (gst_vaissd_stop);
  video_filter_class->set_info = GST_DEBUG_FUNCPTR (gst_vaissd_set_info);
  video_filter_class->transform_frame_ip = GST_DEBUG_FUNCPTR (gst_vaissd_transform_frame_ip);
  
  /* define properties */
  g_object_class_install_property (gobject_class, PROP_MODEL_NAME,
    g_param_spec_string ("model", "Model Name", "Model Name",
              "ssd_pedestrain_pruned_0_97", (GParamFlags) (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

}

static void
gst_vaissd_init (GstVaissd *vaissd)
{
}

void
gst_vaissd_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  GstVaissd *vaissd = GST_VAISSD (object);

  GST_DEBUG_OBJECT (vaissd, "set_property");

  switch (property_id) {
    case PROP_MODEL_NAME:
      vaissd->model_name = g_value_dup_string(value);
      g_print ("Vitis AI SSD Model Name: %s\n", vaissd->model_name);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_vaissd_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  GstVaissd *vaissd = GST_VAISSD (object);

  GST_DEBUG_OBJECT (vaissd, "get_property");

  switch (property_id) {
    case PROP_MODEL_NAME:
      g_value_set_string(value, vaissd->model_name);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_vaissd_dispose (GObject * object)
{
  GstVaissd *vaissd = GST_VAISSD (object);

  GST_DEBUG_OBJECT (vaissd, "dispose");

  /* clean up as possible.  may be called multiple times */

  G_OBJECT_CLASS (gst_vaissd_parent_class)->dispose (object);
}

void
gst_vaissd_finalize (GObject * object)
{
  GstVaissd *vaissd = GST_VAISSD (object);

  GST_DEBUG_OBJECT (vaissd, "finalize");

  /* clean up object here */

  G_OBJECT_CLASS (gst_vaissd_parent_class)->finalize (object);
}

static gboolean
gst_vaissd_start (GstBaseTransform * trans)
{
  GstVaissd *vaissd = GST_VAISSD (trans);

  GST_DEBUG_OBJECT (vaissd, "start");

  return TRUE;
}

static gboolean
gst_vaissd_stop (GstBaseTransform * trans)
{
  GstVaissd *vaissd = GST_VAISSD (trans);

  GST_DEBUG_OBJECT (vaissd, "stop");

  return TRUE;
}

static gboolean
gst_vaissd_set_info (GstVideoFilter * filter, GstCaps * incaps,
    GstVideoInfo * in_info, GstCaps * outcaps, GstVideoInfo * out_info)
{
  GstVaissd *vaissd = GST_VAISSD (filter);

  GST_DEBUG_OBJECT (vaissd, "set_info");

  return TRUE;
}

/* transform */
static GstFlowReturn
gst_vaissd_transform_frame (GstVideoFilter * filter, GstVideoFrame * inframe,
    GstVideoFrame * outframe)
{
  GstVaissd *vaissd = GST_VAISSD (filter);

  GST_DEBUG_OBJECT (vaissd, "transform_frame");

  return GST_FLOW_OK;
}

static GstFlowReturn
gst_vaissd_transform_frame_ip (GstVideoFilter * filter, GstVideoFrame * frame)
{
  GstVaissd *vaissd = GST_VAISSD (filter);

  /* Create Single Shot Detector object */
  thread_local auto ssd = vitis::ai::SSD::create((char*) vaissd->model_name, "true");

  /* Copy frame data into OpenCV Mat */
  cv::Mat img(360, 640, CV_8UC3, GST_VIDEO_FRAME_PLANE_DATA(frame, 0));

  /* Perform Single Shot Detection */
  auto results = ssd->run(img);

  /* Draw bounding boxes */
  for (auto &box : results.bboxes)
  {
    int xmin = box.x * img.cols;
    int ymin = box.y * img.rows;
    int xmax = xmin + box.width * img.cols;
    int ymax = ymin + box.height * img.rows;

    xmin = std::min(std::max(xmin, 0), img.cols);
    xmax = std::min(std::max(xmax, 0), img.cols);
    ymin = std::min(std::max(ymin, 0), img.rows);
    ymax = std::min(std::max(ymax, 0), img.rows);

    cv::rectangle(img, cv::Point(xmin, ymin), cv::Point(xmax, ymax), cv::Scalar(0, 0, 255), 2, 1, 0);
  }

  GST_DEBUG_OBJECT (vaissd, "transform_frame_ip");

  return GST_FLOW_OK;

}

static gboolean
plugin_init (GstPlugin * plugin)
{

  /* FIXME Remember to set the rank if it's an element that is meant
     to be autoplugged by decodebin. */
  return gst_element_register (plugin, "vaissd", GST_RANK_NONE,
      GST_TYPE_VAISSD);
}

/* FIXME: these are normally defined by the GStreamer build system.
   If you are creating an element to be included in gst-plugins-*,
   remove these, as they're always defined.  Otherwise, edit as
   appropriate for your external plugin package. */
#ifndef VERSION
#define VERSION "0.1.0"
#endif
#ifndef PACKAGE
#define PACKAGE "vaissd"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "Vitis AI Single Shot Detector (SSD)"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "https://bluetiger9.github.io/"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    vaissd,
    "Vitis AI Single Shot Detector (SSD)",
    plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN)

