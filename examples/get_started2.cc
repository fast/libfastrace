// Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0.

#include <libfastrace/libfastrace.h>

#include <array>

int main(void) {
  std::array<const char*, 10> keys = {"pk1", "pk2", "pk3", "pk4", "ck1",
                                      "ck2", "ck3", "ck4", "ck5", "ck6"};
  std::array<const char*, 10> vals = {"pv1", "pv2", "pv3", "pv4", "cv1",
                                      "cv2", "cv3", "cv4", "cv5", "cv6"};

  fastrace_glue::ftr_set_cons_rptr();
  auto p = fastrace_glue::ftr_create_rand_span_ctx();
  auto r = fastrace_glue::ftr_create_root_span("root", p);

  fastrace_glue::ftr_add_ent_to_par(
      "parent event", r, rust::Slice<const char* const>(keys.data(), 2),
      rust::Slice<const char* const>(vals.data(), 2));

  fastrace_glue::ftr_span_with_prop(r, "phello", "pworld");
  fastrace_glue::ftr_span_with_props(
      r, rust::Slice<const char* const>(keys.data() + 2, 2),
      rust::Slice<const char* const>(vals.data() + 2, 2));

  auto g = fastrace_glue::ftr_set_loc_par_to_span(r);

  auto ls = fastrace_glue::ftr_create_loc_span_enter("child");
  fastrace_glue::ftr_add_ent_to_loc_par(
      "child event", rust::Slice<const char* const>(keys.data() + 4, 2),
      rust::Slice<const char* const>(vals.data() + 4, 2));

  fastrace_glue::ftr_loc_span_add_prop("chello", "cworld");
  fastrace_glue::ftr_loc_span_add_props(
      rust::Slice<const char* const>(keys.data() + 6, 2),
      rust::Slice<const char* const>(vals.data() + 6, 2));

  fastrace_glue::ftr_loc_span_with_prop(ls, "chello2", "cworld2");
  fastrace_glue::ftr_loc_span_with_props(
      ls, rust::Slice<const char* const>(keys.data() + 8, 2),
      rust::Slice<const char* const>(vals.data() + 8, 2));

  fastrace_glue::ftr_destroy_loc_span(ls);
  fastrace_glue::ftr_destroy_loc_par_guar(g);
  fastrace_glue::ftr_destroy_span(r);

  fastrace_glue::ftr_flush();
  return 0;
}
