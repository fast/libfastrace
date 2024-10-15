// Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0.

#include <libfastrace/libfastrace.h>
#include <unistd.h>

void func2(int i) {
  ftr_loc_span ls = ftr_create_loc_span_enter("func2");
  usleep(i * 1000);
  ftr_destroy_loc_span(ls);
}

void func1(int i) {
  ftr_loc_span ls = ftr_create_loc_span_enter("func1");
  usleep(i * 1000);
  func2(i);
  ftr_destroy_loc_span(ls);
}

int main(void) {
  ftr_otlp_exp_cfg ecfg = ftr_create_def_otlp_exp_cfg();
  ftr_coll_cfg cfg = ftr_create_def_coll_cfg();

  ftr_otel_rptr rptr = ftr_create_otel_rptr(ecfg);
  ftr_set_otel_rptr(rptr, cfg);

  ftr_span_ctx p = ftr_create_rand_span_ctx();
  ftr_span r = ftr_create_root_span("root", p);
  ftr_loc_par_guar g = ftr_set_loc_par_to_span(&r);
  ftr_loc_span ls = ftr_create_loc_span_enter("a span");
  // ftr_add_prop_to_loc_span(ls, "a property", "a value");

  for (int i = 1; i <= 10; i++) {
    func1(i);
  }

  ftr_destroy_loc_span(ls);
  ftr_destroy_loc_par_guar(g);
  ftr_destroy_span(r);

  ftr_flush();

  ftr_destroy_otel_rptr(rptr);
  return 0;
}
