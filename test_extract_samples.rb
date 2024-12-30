require "test/unit"
require "gst"
require_relative "extract_samples/extract_samples"

class TestExtractSamples < Test::Unit::TestCase
  def setup
    @bin = Gst::Pipeline.new("pipeline")
    clock = @bin.pipeline_clock
    @src = Gst::ElementFactory.make("autoaudiosrc", nil)
    raise "need audiotestsrc from gst-plugins-base" if @src.nil?
    @wavenc = Gst::ElementFactory.make("wavenc", nil)
    raise "need wavenc from gst-plugins-good" if @wavenc.nil?
    @sink = Gst::ElementFactory.make("autoaudiosink", nil)
    raise "need autoaudiosink from gst-plugins-good" if @sink.nil?

    @bin << @src << @wavenc << @sink
    @src >> @wavenc >> @sink

  end

  def test_extract_samples
    assert_not_empty ExtractSamples.extract_samples @wavenc
  end
end
