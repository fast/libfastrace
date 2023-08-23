// Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0.

#include <minitrace_c/minitrace_c.h>

int main(void) {
  minitrace_glue::mtr_set_cons_rptr();
  auto p = minitrace_glue::mtr_create_rand_span_ctx();
  auto r = minitrace_glue::mtr_create_root_span("root", p);
  auto g = minitrace_glue::mtr_set_loc_par_to_span(r);
  auto ls = minitrace_glue::mtr_create_loc_span_enter("a span");

  minitrace_glue::mtr_destroy_loc_span(ls);
  minitrace_glue::mtr_destroy_loc_par_guar(g);
  minitrace_glue::mtr_destroy_span(r);

  minitrace_glue::mtr_flush();
  return 0;
}
