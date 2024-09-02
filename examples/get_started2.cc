// Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0.

#include <fastrace_c/fastrace_c.h>

int main(void) {
  fastrace_glue::ftr_set_cons_rptr();
  auto p = fastrace_glue::ftr_create_rand_span_ctx();
  auto r = fastrace_glue::ftr_create_root_span("root", p);
  auto g = fastrace_glue::ftr_set_loc_par_to_span(r);
  auto ls = fastrace_glue::ftr_create_loc_span_enter("a span");

  fastrace_glue::ftr_destroy_loc_span(ls);
  fastrace_glue::ftr_destroy_loc_par_guar(g);
  fastrace_glue::ftr_destroy_span(r);

  fastrace_glue::ftr_flush();
  return 0;
}
