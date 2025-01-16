require "gst"

pipeline = Gst::Pipeline.new("audio-extractor")
src = Gst::ElementFactory.make("filesrc", nil)
parse = Gst::ElementFactory.make("wavparse", nil)
convert = Gst::ElementFactory.make("audioconvert", nil)
resample = Gst::ElementFactory.make("audioresample", nil)
sink = Gst::ElementFactory.make("appsink", nil)

src.location = ARGV[0]

caps = Gst::Caps.new("audio/x-raw")
caps.set_value("format", "F32LE")
caps.set_value("rate", 16_000)
caps.set_value("channels", 1)
caps.set_value("layout", "interleaved");

sink.caps = caps
sink.emit_signals = true

samples = []

sink.signal_connect(:new_sample) do |_|
  sample = sink.pull_sample
  samples << sample
  Gst::FlowReturn::OK
end

pipeline << src << parse << convert << resample << sink
src >> parse >> convert >> resample >> sink

loop = GLib::MainLoop.new

bus = pipeline.bus
bus.add_watch do |bus, message|
  case message.type
  when Gst::MessageType::EOS
    puts "End-of-stream"
    loop.quit
  when Gst::MessageType::ERROR
    error, debug = message.parse_error
    puts "Debugging info: #{debug || 'none'}"
    puts "Error: #{error.message}"
    loop.quit
  end
  true
end

pipeline.play
begin
  loop.run
rescue Interrupt
  puts "Interrupt"
rescue => error
  puts "Error: #{error.message}"
ensure
  pipeline.stop
end

buffer_list = Gst::BufferList.new
samples.each do |sample|
  buffer_list.insert -1, sample.buffer
end
sample = Gst::Sample.new(
  Gst::Buffer.new,
  caps,
  Gst::Segment.new,
  caps.structures[0]
)

sample.buffer_list = buffer_list
pp sample.buffer_list.length
pp sample.buffer_list.calculate_size
