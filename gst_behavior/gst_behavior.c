#include <gst/gst.h>
#include <glib.h>

static int bus_call_count = 0;
static int on_new_sample_count = 0;

static gboolean
bus_call(GstBus *bus, GstMessage *msg, gpointer data)
{
  g_print("bus_call: %d\n", bus_call_count++);

  g_print("message type: %s\n", gst_message_type_get_name(GST_MESSAGE_TYPE(msg)));

  switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_EOS:
      g_print("End of stream\n");
      g_main_loop_quit((GMainLoop *)data);
      break;
    case GST_MESSAGE_ERROR: {
      gchar *debug;
      GError *error;

      gst_message_parse_error(msg, &error, &debug);
      g_free(debug);

      g_printerr("Error: %s\n", error->message);
      g_error_free(error);

      g_main_loop_quit((GMainLoop *)data);
      break;
    }
    default:
      break;
  }

  return TRUE;
}

static GstFlowReturn
on_new_sample(GstElement *sink, gpointer data)
{
  g_print("on_new_sample: %d\n", on_new_sample_count++);
  return GST_FLOW_OK;
}

int
main(int argc, char *argv[])
{
  GMainLoop *loop;

  GstElement *pipeline, *source, *conv, *sink;
  GstBus *bus;
  guint bus_watch_id;
  GstCaps *caps;

  gst_init(&argc, &argv);

  loop = g_main_loop_new(NULL, FALSE);

  pipeline = gst_pipeline_new("my_pipeline");
  source = gst_element_factory_make("audiotestsrc", NULL);
  conv = gst_element_factory_make("audioconvert", NULL);
  sink = gst_element_factory_make("appsink", NULL);

  if (!pipeline || !source || !conv || !sink) {
    g_printerr("One element could not be created. Exiting.\n");
    return -1;
  }

  bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
  bus_watch_id = gst_bus_add_watch(bus, bus_call, loop);
  gst_object_unref(bus);

  g_object_set(G_OBJECT(source),
               "num-buffers", 5,
               "samplesperbuffer", 1024,
               NULL);

  caps = gst_caps_new_simple("audio/x-raw",
    "format", G_TYPE_STRING, "F32LE",
    "rate", G_TYPE_INT, 16000,
    "channels", G_TYPE_INT, 1,
    NULL);
  g_object_set(G_OBJECT(sink),
               "caps", caps,
               "emit-signals", TRUE,
               NULL);
  g_signal_connect(sink, "new-sample", G_CALLBACK(on_new_sample), NULL);

  gst_bin_add_many(GST_BIN(pipeline), source, conv, sink, NULL);
  gst_element_link_many(source, conv, sink, NULL);

  gst_element_set_state(pipeline, GST_STATE_PLAYING);

  g_main_loop_run(loop);

  gst_element_set_state(pipeline, GST_STATE_NULL);
  gst_object_unref(GST_OBJECT(pipeline));
  g_source_remove(bus_watch_id);
  g_main_loop_unref(loop);

  g_print("num-buffers property: %d, new-sample callback count: %d\n", 5, on_new_sample_count);

  return 0;
}
