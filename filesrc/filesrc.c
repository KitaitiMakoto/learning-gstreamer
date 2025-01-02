#include <gst/gst.h>
#include <glib.h>

static int bus_call_count = 0;

static gboolean
bus_call(GstBus *bus, GstMessage *msg, gpointer data)
{
  g_print("bus_call: %d\n", bus_call_count++);
  g_print("message type: %s(%d)\n", gst_message_type_get_name(GST_MESSAGE_TYPE(msg)), GST_MESSAGE_TYPE(msg));

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

int main(int argc, char *argv[])
{
  GMainLoop *loop;

  GstElement *pipeline, *source, *sink;
  GstBus *bus;
  guint bus_watch_id;

  if (argc != 2) {
    g_print("Usage: %s <filename>\n", argv[0]);
    return -1;
  }

  const char *filename = argv[1];
  g_print("filename: %s\n", filename);

  gst_init(&argc, &argv);

  loop = g_main_loop_new(NULL, FALSE);

  pipeline = gst_pipeline_new("watch_filesrc");
  source = gst_element_factory_make("filesrc", NULL);
  sink = gst_element_factory_make("fakesink", NULL);

  if (!pipeline || !source || !sink) {
    g_printerr("Not all elements could be created.\n");
    return -1;
  }

  g_object_set(G_OBJECT(source), "location", filename, NULL);

  gst_bin_add_many(GST_BIN(pipeline), source, sink, NULL);
  gst_element_link(source, sink);

  bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
  bus_watch_id = gst_bus_add_watch(bus, bus_call, loop);
  gst_object_unref(bus);

  gst_element_set_state(pipeline, GST_STATE_PLAYING);
  g_main_loop_run(loop);

  gst_element_set_state(pipeline, GST_STATE_NULL);
  gst_object_unref(GST_OBJECT(pipeline));
  g_source_remove(bus_watch_id);
  g_main_loop_unref(loop);

  g_print("bus_call_count: %d\n", bus_call_count);

  return 0;
}
