require "rake/clean"


task default: :all
task all: ["ruby_ext:test", "gst_behavior:test"]

namespace :ruby_ext do
  LIB_DIR = "extract_samples"
  LIB_PATH = "#{LIB_DIR}/extract_samples.#{RbConfig::CONFIG['DLEXT']}"
  CLEAN.include LIB_PATH

  task test: LIB_PATH do
    ruby "test_extract_samples.rb"
  end

  file LIB_PATH => "#{LIB_DIR}/Makefile" do
    cd LIB_DIR do
      sh "make"
    end
  end

  file "#{LIB_DIR}/Makefile" => "#{LIB_DIR}/extconf.rb" do
    cd LIB_DIR do
      ruby "extconf.rb"
    end
  end
  CLEAN.include "#{LIB_DIR}/Makefile"
end

namespace :gst_behavior do
  task test: "gst_behavior/gst_behavior" do |t|
    sh t.source
  end
  CLEAN.include "gst_behavior/gst_behavior"

  file "gst_behavior/gst_behavior" => "gst_behavior/gst_behavior.c" do |t|
    cd "gst_behavior" do
      sh "gcc -Wall gst_behavior.c -o gst_behavior $(pkg-config --cflags --libs gstreamer-1.0) $(pkg-config --cflags --libs gstreamer-app-1.0) $(pkg-config --cflags --libs gstreamer-audio-1.0)"
    end
  end
end

SAMPLE_AUDIO = "one-second-audio.wav"
file SAMPLE_AUDIO do |t|
  duration = 1

  samplesperbuffer = 1024
  rate = 44100
  channels = 1

  num_samples_per_channel = rate * duration
  num_samples = num_samples_per_channel * channels
  num_buffers = (num_samples.to_f / samplesperbuffer).ceil

  sh "gst-launch-1.0 audiotestsrc num-buffers=#{num_buffers} ! audioconvert ! audio/x-raw,format=S16LE,channels=2 ! wavenc ! filesink location=#{t.name}"
  show_audio_info t.name
end
CLEAN.include SAMPLE_AUDIO

EXPECTED_AUDIO = "expected-audio.wav"
file EXPECTED_AUDIO do |t|
  duration = 1

  samplesperbuffer = 1024
  rate = 44100
  channels = 1

  num_samples_per_channel = rate * duration
  num_samples = num_samples_per_channel * channels
  num_buffers = (num_samples.to_f / samplesperbuffer).ceil

  sh "gst-launch-1.0 audiotestsrc num-buffers=#{num_buffers} ! audioconvert ! audio/x-raw,format=F32LE,channels=1,rate=16000 ! wavenc ! filesink location=#{t.name}"
  show_audio_info t.name
end
CLEAN.include EXPECTED_AUDIO

namespace :filesrc do
  task test: ["filesrc/filesrc", SAMPLE_AUDIO] do |t|
    sh t.source, SAMPLE_AUDIO
  end

  file "filesrc/filesrc" => "filesrc/filesrc.c" do |t|
    cd "filesrc" do
      sh "gcc -Wall #{t.source.pathmap('%f')} -o #{t.name.pathmap('%f')} $(pkg-config --cflags --libs gstreamer-1.0) $(pkg-config --cflags --libs gstreamer-app-1.0) $(pkg-config --cflags --libs gstreamer-audio-1.0)"
    end
  end
  CLEAN.include "filesrc/filesrc"
end

namespace :queue do
  task test: ["queue/queue", SAMPLE_AUDIO] do |t|
    sh *t.sources
  end

  file "queue/queue" => "queue/queue.c" do |t|
    cd "queue" do
      sh "gcc -Wall #{t.source.pathmap('%f')} -o #{t.name.pathmap('%f')} $(pkg-config --cflags --libs gstreamer-1.0) $(pkg-config --cflags --libs gstreamer-app-1.0) $(pkg-config --cflags --libs gstreamer-audio-1.0)"
    end
  end
  CLEAN.include "queue/queue"
end

namespace :buffer_list do
  task test: ["buffer_list/buffer_list", SAMPLE_AUDIO] do |t|
    sh t.source, t.sources[1]
  end

  file "buffer_list/buffer_list" => "buffer_list/buffer_list.c" do |t|
    cd "buffer_list" do
      sh "gcc -Wall #{t.source.pathmap('%f')} -o #{t.name.pathmap('%f')} $(pkg-config --cflags --libs gstreamer-1.0) $(pkg-config --cflags --libs gstreamer-app-1.0) $(pkg-config --cflags --libs gstreamer-audio-1.0)"
    end
  end
  CLEAN.include "buffer_list/buffer_list"

  task ruby: ["buffer_list/create-buffer-list.rb", SAMPLE_AUDIO] do |t|
    ruby t.source, t.sources[1]
  end
end

rule ".o" => ".c" do |t|
  sh "gcc -c -o #{t.name} #{t.source}"
end
CLEAN.include "**/*.o"

CLEAN.include "**/*.dSYM"

namespace :audio_info do
  [SAMPLE_AUDIO, EXPECTED_AUDIO].each do |audio|
    task audio => audio do |t|
      show_audio_info t.source
    end
  end
end

require "wavefile"
def show_audio_info(file)
  sh "ffprobe", file
  reader = WaveFile::Reader.new(file)
  pp reader
end
