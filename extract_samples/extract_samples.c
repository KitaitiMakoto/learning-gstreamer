#include <ruby.h>
#include <ruby/memory_view.h>

static VALUE extract_samples(VALUE self, VALUE obj)
{
  if (!rb_memory_view_available_p(obj)) {
    rb_raise(rb_eTypeError, "obj must be a memory view object");
  }

  rb_memory_view_t view;
  if (!rb_memory_view_get(obj, &view, RUBY_MEMORY_VIEW_SIMPLE)) {
    rb_raise(rb_eTypeError, "Failed to get memory view");
  }

  const ssize_t n_samples = view.byte_size / view.item_size;
  VALUE samples = rb_ary_new2(n_samples);

  return samples;
}

void Init_extract_samples()
{
    VALUE mExtractSamples = rb_define_module("ExtractSamples");
    rb_define_module_function(mExtractSamples, "extract_samples", extract_samples, 1);
}
