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

rule ".o" => ".c" do |t|
  sh "gcc -c -o #{t.name} #{t.source}"
end
CLEAN.include "**/*.o"

CLEAN.include "**/*.dSYM"
