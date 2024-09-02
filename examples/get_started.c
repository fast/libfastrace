// Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0.

#include <fastrace_c/fastrace_c.h>

int main(void) {
  ftr_set_cons_rptr();
  ftr_span_ctx p = ftr_create_rand_span_ctx();
  ftr_span r = ftr_create_root_span("root", p);
  ftr_loc_par_guar g = ftr_set_loc_par_to_span(&r);
  ftr_loc_span ls = ftr_create_loc_span_enter("a span");

  ftr_destroy_loc_span(ls);
  ftr_destroy_loc_par_guar(g);
  ftr_destroy_span(r);

  ftr_flush();
  return 0;
}
