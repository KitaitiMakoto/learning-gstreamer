require "gst"

bin = Gst::Pipeline.new("pipeline")
clock = bin.pipeline_clock
src = Gst::ElementFactory.make("autoaudiosrc", nil)
raise "need audiotestsrc from gst-plugins-base" if src.nil?
wavenc = Gst::ElementFactory.make("wavenc", nil)
raise "need wavenc from gst-plugins-good" if @wavenc.nil?
sink = Gst::ElementFactory.make("autoaudiosink", nil)
raise "need autoaudiosink from gst-plugins-good" if sink.nil?

bin << src << wavenc << sink
src >> wavenc >> sink

loop = GLib::MainLoop.new

bin.play

begin
  loop.run
ensure
  bin.stop
end
