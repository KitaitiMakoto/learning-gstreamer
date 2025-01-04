Learning GStreamer
==================

gst_behavior
------------

Code to learn GStreamer behavior.

* Does `GstSample` represent *single* sample?
* Does `GstBuffer` referred from `GstSample` represent corresponding *single* sample?
* Array items in `GstBuffer` can be treated as `float`s when `appsink`'s caps's format is `F32LE`? Or needs to convert from `int`s?

filesrc
-------

Code to learn GStreamer behavior on handling audio file.

* How to catch EOS from appsink?
  * -> Call `gst_app_sink_pull_sample()` in `new-sample` callback. Without that, the app never end.
    * [pull-sample action's document][pull-sample] says:  
      > This function blocks until a sample or EOS becomes available
* Is `on_new_sample()` called as many times as the number of samples?
  * -> No. `on_new_sample()` is called 26 times but the audio file has 45056 samples.
  * It should be 22528 even though the function is called for each pair of channels (left and right).
  * In that case, how can we retrieve all samples?
    * Probably `pull-sample` action. [Its document][pull-sample] says:  
      > All rendered samples will be put in a queue so that the application can pull samples at its own rate.
      
      Note that it uses plural ("samples").
  * The number of samples is calculated by `gsize n_samples = map_info.size / (audio_finfo->width / 8) / audio_info->channels;` in single `new-samples` callback.
* What's the difference between the combination of `new-sample` signal and `gst_app_sink_pull_sample()`, and `pull-sample` action?
  * The former handles single sample and the latter does multiple samples at a time?
* Can `gst_pad_add_probe` register an event on buffer which includes multiple samples?

[pull-sample]: https://gstreamer.freedesktop.org/documentation/app/appsink.html?gi-language=c#appsink-page

extract_samples
---------------

Attempt to receive samples from audio source and expose it as MemoryView.
