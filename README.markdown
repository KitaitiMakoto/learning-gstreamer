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
* Can `gst_pad_add_probe` register an event on buffer which includes multiple samples?

extract_samples
---------------

Attempt to receive samples from audio source and expose it as MemoryView.
