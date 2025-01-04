#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/audio/audio-info.h>
#include <glib.h>

static int on_new_sample_count = 0;
static GstClockTime total_duration = 0;
static gsize total_samples = 0;

static gboolean
bus_call(GstBus *bus, GstMessage *msg, gpointer data)
{
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
  GstStructure *structure;
  GstAudioInfo *audio_info;
  const GstAudioFormatInfo *audio_finfo;
  GstClockTime duration;

  g_print("=== on_new_sample: %d ===\n", on_new_sample_count++);

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
  structure = gst_caps_get_structure(caps, 0);
  if (structure == NULL) {
    g_print("structure is NULL\n");
    return GST_FLOW_ERROR;
  }

  audio_info = gst_audio_info_new_from_caps(caps);
  if (audio_info == NULL) {
    g_print("audio_info is NULL\n");
    return GST_FLOW_ERROR;
  }
  audio_finfo = audio_info->finfo;
  if (audio_finfo == NULL) {
    g_print("audio_finfo is NULL\n");
    return GST_FLOW_ERROR;
  }

  buffer = gst_sample_get_buffer(sample);
  if (buffer == NULL) {
    g_print("buffer is NULL\n");
    return GST_FLOW_ERROR;
  }
  duration = buffer->duration;
  if (duration == GST_CLOCK_TIME_NONE) {
  } else {
    duration = duration / audio_info->channels;
    total_duration += duration;
  }

  const gboolean result = gst_buffer_map(buffer, &map_info, GST_MAP_READ);
  if (!result) {
    g_print("gst_buffer_map failed\n");
    return GST_FLOW_ERROR;
  }
  g_print("map size: %lu bytes\n", map_info.size);
  gsize n_samples = map_info.size / (audio_finfo->width / 8) / audio_info->channels;
  g_print("%lu samples\n", n_samples);
  total_samples += n_samples;

  gst_sample_unref(sample);

  return GST_FLOW_OK;
}

void
queue_callback(GstElement *queue, gpointer udata)
{
  gchar *signal;

  signal = (gchar *)udata;
  g_print("queue_callback: %s\n", signal);

}

int main(int argc, char *argv[])
{
  GMainLoop *loop;

  GstElement *pipeline, *source, *parse, *convert, *queue, *sink;
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
  queue = gst_element_factory_make("queue", NULL);
  sink = gst_element_factory_make("appsink", NULL);

  if (!pipeline || !source || !parse || !convert || !queue || !sink) {
    g_printerr("Not all elements could be created.\n");
    return -1;
  }

  g_object_set(G_OBJECT(source), "location", filename, NULL);

  g_object_set(G_OBJECT(queue),
               "min-threshold-buffers", 199,
               NULL);
  g_signal_connect(queue, "overrun", G_CALLBACK(queue_callback), "overrun");
  g_signal_connect(queue, "pushing", G_CALLBACK(queue_callback), "pushing");
  g_signal_connect(queue, "running", G_CALLBACK(queue_callback), "running");
  g_signal_connect(queue, "underrun", G_CALLBACK(queue_callback), "underrun");

  caps = gst_caps_new_simple("audio/x-raw",
                             "format", G_TYPE_STRING, "S16LE",
                             NULL);
  g_object_set(G_OBJECT(sink),
               "caps", caps,
               "emit-signals", TRUE,
               NULL);
  g_signal_connect(sink, "new-sample", G_CALLBACK(on_new_sample), NULL);

  gst_bin_add_many(GST_BIN(pipeline), source, parse, convert, queue, sink, NULL);
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

  g_print("total samples should be %d, actually %lu\n", 45056, total_samples);
  g_print("on_new_sample_count: %d\n", on_new_sample_count);
  g_print("total_duration: %llus\n", total_duration / 1000 / 1000);

  return 0;
}
