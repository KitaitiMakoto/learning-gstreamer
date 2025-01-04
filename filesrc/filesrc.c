#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/audio/audio-info.h>
#include <glib.h>

static int bus_call_count = 0;
static int on_new_sample_count = 0;

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

static GstFlowReturn
on_new_sample(GstElement *sink, gpointer data)
{
  GstSample *sample;
  GstBuffer *buffer;
  GstMapInfo map_info;
  GstCaps *caps;
  GstAudioInfo *audio_info;
  const GstAudioFormatInfo *audio_finfo;
  GstClockTime duration;

  g_print("on_new_sample: %d\n", on_new_sample_count++);

  sample = gst_app_sink_pull_sample(GST_APP_SINK(sink));
  if (sample == NULL) {
    g_print("sample is NULL\n");
    return GST_FLOW_ERROR;
  }

  caps = gst_sample_get_caps(sample);
  if (caps == NULL) {
    g_print("caps is NULL\n");
    return GST_FLOW_ERROR;
  }

  audio_info = gst_audio_info_new_from_caps(caps);
  if (audio_info == NULL) {
    g_print("audio_info is NULL\n");
    return GST_FLOW_ERROR;
  }
  g_print("channels: %d, flags: %d\n", audio_info->channels, audio_info->flags);
  audio_finfo = audio_info->finfo;
  if (audio_finfo == NULL) {
    g_print("audio_finfo is NULL\n");
    return GST_FLOW_ERROR;
  }
  g_print("audio format: %s, width: %d, depth: %d, unpack_format: %s, has unpack_func: %s\n",
          gst_audio_format_to_string(audio_finfo->format),
          audio_finfo->width,
          audio_finfo->depth,
          gst_audio_format_to_string(audio_finfo->unpack_format),
          audio_finfo->unpack_func ? "true" : "false");

  buffer = gst_sample_get_buffer(sample);
  if (buffer == NULL) {
    g_print("buffer is NULL\n");
    return GST_FLOW_ERROR;
  }
  duration = buffer->duration;
  if (duration == GST_CLOCK_TIME_NONE) {
    g_print("duration is GST_CLOCK_TIME_NONE\n");
  } else {
    duration = duration / audio_info->channels;
    g_print("buffer duration: %llums\n", duration / 1000 / 1000);
  }

  const gboolean result = gst_buffer_map(buffer, &map_info, GST_MAP_READ);
  if (!result) {
    g_print("gst_buffer_map failed\n");
    return GST_FLOW_ERROR;
  }
  g_print("map size: %lu bytes\n", map_info.size);
  g_print("%lu samples?\n", map_info.size / (audio_finfo->width / 8));

  gst_sample_unref(sample);

  return GST_FLOW_OK;
}

void
on_eos(GstElement *appsink, gpointer user_data)
{
  g_print("on_eos\n");
}

int main(int argc, char *argv[])
{
  GMainLoop *loop;

  GstElement *pipeline, *source, *parse, *convert, *sink;
  GstBus *bus;
  GstCaps *caps;
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
  parse = gst_element_factory_make("wavparse", NULL);
  convert = gst_element_factory_make("audioconvert", NULL);
  sink = gst_element_factory_make("appsink", NULL);

  if (!pipeline || !source || !sink) {
    g_printerr("Not all elements could be created.\n");
    return -1;
  }

  g_object_set(G_OBJECT(source), "location", filename, NULL);

  caps = gst_caps_new_simple("audio/x-raw",
                             "format", G_TYPE_STRING, "S16LE",
                             NULL);
  g_object_set(G_OBJECT(sink),
               "caps", caps,
               "emit-signals", TRUE,
               NULL);
  g_signal_connect(sink, "new-sample", G_CALLBACK(on_new_sample), NULL);
  g_signal_connect(sink, "eos", G_CALLBACK(on_eos), NULL);

  gst_bin_add_many(GST_BIN(pipeline), source, parse, convert, sink, NULL);
  gst_element_link_many(source, parse, convert, sink, NULL);

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
  g_print("on_new_sample_count: %d\n", on_new_sample_count);

  return 0;
}
