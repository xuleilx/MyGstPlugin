/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2021 xuleilx <<user@hostname.org>>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
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

/**
 * SECTION:element-myfilter
 *
 * FIXME:Describe myfilter here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! myfilter ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gst/gst.h>

#include "gstmyfilter.h"

GST_DEBUG_CATEGORY_STATIC(gst_my_filter_debug);
#define GST_CAT_DEFAULT gst_my_filter_debug

/* Filter signals and args */
enum
{
    /* FILL ME */
    LAST_SIGNAL
};

enum
{
    PROP_0,
    PROP_SILENT
};

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE("sink",
                                                                   GST_PAD_SINK,
                                                                   GST_PAD_ALWAYS,
                                                                   GST_STATIC_CAPS("ANY"));

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE("src",
                                                                  GST_PAD_SRC,
                                                                  GST_PAD_ALWAYS,
                                                                  GST_STATIC_CAPS("ANY"));

#define gst_my_filter_parent_class parent_class
G_DEFINE_TYPE(GstMyFilter, gst_my_filter, GST_TYPE_ELEMENT);

static void gst_my_filter_set_property(GObject *object,
                                       guint prop_id, const GValue *value, GParamSpec *pspec);
static void gst_my_filter_get_property(GObject *object,
                                       guint prop_id, GValue *value, GParamSpec *pspec);
static GstStateChangeReturn gst_my_filter_change_state(GstElement *element,
                                                       GstStateChange transition);

static gboolean gst_my_filter_sink_event(GstPad *pad,
                                         GstObject *parent, GstEvent *event);
static GstFlowReturn gst_my_filter_chain(GstPad *pad,
                                         GstObject *parent, GstBuffer *buf);
static gboolean gst_my_filter_query(GstPad *pad,
                                    GstObject *parent, GstQuery *query);
/* GObject vmethod implementations */

/* initialize the myfilter's class */
static void
gst_my_filter_class_init(GstMyFilterClass *klass)
{
    GObjectClass *gobject_class;
    GstElementClass *gstelement_class;

    gobject_class = (GObjectClass *)klass;
    gstelement_class = (GstElementClass *)klass;

    gobject_class->set_property = gst_my_filter_set_property;
    gobject_class->get_property = gst_my_filter_get_property;

    g_object_class_install_property(gobject_class, PROP_SILENT,
                                    g_param_spec_boolean("silent", "Silent", "Produce verbose output / 是不是安静模式? ",
                                                         FALSE, G_PARAM_READWRITE));

    gstelement_class->change_state = gst_my_filter_change_state;
    gst_element_class_set_details_simple(gstelement_class,
                                         "MyFilter",
                                         "FIXME:Generic",
                                         "FIXME:Generic Template Element", "xuleilx <<user@hostname.org>>");

    gst_element_class_add_pad_template(gstelement_class,
                                       gst_static_pad_template_get(&src_factory));
    gst_element_class_add_pad_template(gstelement_class,
                                       gst_static_pad_template_get(&sink_factory));
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static void
gst_my_filter_init(GstMyFilter *filter)
{
    filter->sinkpad = gst_pad_new_from_static_template(&sink_factory, "sink");
    gst_pad_set_event_function(filter->sinkpad,
                               GST_DEBUG_FUNCPTR(gst_my_filter_sink_event));
    gst_pad_set_chain_function(filter->sinkpad,
                               GST_DEBUG_FUNCPTR(gst_my_filter_chain));
    gst_pad_set_query_function(filter->sinkpad,
                               GST_DEBUG_FUNCPTR(gst_my_filter_query));
    GST_PAD_SET_PROXY_CAPS(filter->sinkpad);
    gst_element_add_pad(GST_ELEMENT(filter), filter->sinkpad);

    filter->srcpad = gst_pad_new_from_static_template(&src_factory, "src");
    GST_PAD_SET_PROXY_CAPS(filter->srcpad);
    gst_element_add_pad(GST_ELEMENT(filter), filter->srcpad);

    filter->silent = FALSE;
}

static void
gst_my_filter_set_property(GObject *object, guint prop_id,
                           const GValue *value, GParamSpec *pspec)
{
    GstMyFilter *filter = GST_MY_FILTER(object);

    switch (prop_id)
    {
    case PROP_SILENT:
        filter->silent = g_value_get_boolean(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void
gst_my_filter_get_property(GObject *object, guint prop_id,
                           GValue *value, GParamSpec *pspec)
{
    GstMyFilter *filter = GST_MY_FILTER(object);

    switch (prop_id)
    {
    case PROP_SILENT:
        g_value_set_boolean(value, filter->silent);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static GstStateChangeReturn
gst_my_filter_change_state(GstElement *element, GstStateChange transition)
{
    GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;
    GstMyFilter *filter = GST_MY_FILTER(element);

    g_assert(filter);
    g_print("%s got state change:%s\n", __FUNCTION__, 
        gst_element_state_get_name(GST_STATE_TRANSITION_NEXT(transition)));

    switch (transition)
    {
    case GST_STATE_CHANGE_NULL_TO_READY:
        break;
    default:
        break;
    }

    ret = GST_ELEMENT_CLASS(parent_class)->change_state(element, transition);
    if (ret == GST_STATE_CHANGE_FAILURE)
        return ret;

    switch (transition)
    {
    case GST_STATE_CHANGE_READY_TO_NULL:
        break;
    default:
        break;
    }

    return ret;
}
/* GstElement vmethod implementations */

/* this function handles sink events */
static gboolean
gst_my_filter_sink_event(GstPad *pad, GstObject *parent,
                         GstEvent *event)
{
    GstMyFilter *filter;
    gboolean ret;

    filter = GST_MY_FILTER(parent);

    GST_LOG_OBJECT(filter, "Received %s event: %" GST_PTR_FORMAT,
                   GST_EVENT_TYPE_NAME(event), event);

    switch (GST_EVENT_TYPE(event))
    {
    case GST_EVENT_CAPS:
    {
        GstCaps *caps;

        gst_event_parse_caps(event, &caps);
        /* do something with the caps */

        /* and forward */
        ret = gst_pad_event_default(pad, parent, event);
        break;
    }
    case GST_EVENT_EOS:
    {
        /* end-of-stream, we should close down all stream leftovers here */
        // gst_my_filter_stop_processing(filter);
        g_print("%s eos\n", __FUNCTION__);
        ret = gst_pad_event_default(pad, parent, event);
        break;
    }
    default:
        ret = gst_pad_event_default(pad, parent, event);
        break;
    }
    return ret;
}

/* chain function
 * this function does the actual processing
 */
static GstFlowReturn
gst_my_filter_chain(GstPad *pad, GstObject *parent, GstBuffer *buf)
{
    GstMyFilter *filter = GST_MY_FILTER(parent);

    if (!filter->silent)
        g_print("Have data of size %" G_GSIZE_FORMAT " bytes!\n",
                gst_buffer_get_size(buf));

    return gst_pad_push(filter->srcpad, buf);
}

static gboolean gst_my_filter_query(GstPad *pad, GstObject *parent, GstQuery *query)
{
    gboolean ret;
    // GstMyFilter *filter = GST_MY_FILTER(parent);

    g_print("Here we are, in gst_my_filter_query :%s.\n", GST_QUERY_TYPE_NAME(query));
    switch (GST_QUERY_TYPE(query))
    {
    case GST_QUERY_POSITION:
    {
        GstFormat fmt;

        gst_query_parse_position(query, &fmt, NULL);
        if (fmt == GST_FORMAT_TIME)
        {
            g_print("Here we are, in gst_query_set_position.\n");
            gst_query_set_position(query, GST_FORMAT_TIME, 5 * GST_SECOND);
            ret = TRUE;
        }
    }
    break;
    case GST_QUERY_DURATION:
    {
        GstFormat fmt;

        gst_query_parse_duration(query, &fmt, NULL);
        if (fmt == GST_FORMAT_TIME)
        {
            /* First try to query upstream */
            ret = gst_pad_query_default(pad, parent, query);
            if (!ret)
            {
                gst_query_set_duration(query, GST_FORMAT_TIME, 10 * GST_SECOND);
                ret = TRUE;
            }
        }
        break;
    }
    case GST_QUERY_CAPS:
        /* we should report the supported caps here */

        ret = gst_pad_query_default(pad, parent, query);
        break;
    default:
        /* just call the default handler */
        ret = gst_pad_query_default(pad, parent, query);
        break;
    }
    return ret;
}

/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
myfilter_init(GstPlugin *myfilter)
{
    /* debug category for filtering log messages
   *
   * exchange the string 'Template myfilter' with your description
   */
    GST_DEBUG_CATEGORY_INIT(gst_my_filter_debug, "myfilter",
                            0, "Template myfilter");

    return gst_element_register(myfilter, "myfilter", GST_RANK_NONE,
                                GST_TYPE_MY_FILTER);
}

/* PACKAGE: this is usually set by meson depending on some _INIT macro
 * in meson.build and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use meson to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "myfirstmyfilter"
#endif

/* gstreamer looks for this structure to register myfilters
 *
 * exchange the string 'Template myfilter' with your myfilter description
 */
GST_PLUGIN_DEFINE(GST_VERSION_MAJOR,
                  GST_VERSION_MINOR,
                  myfilter,
                  "Template myfilter",
                  myfilter_init,
                  PACKAGE_VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)
//