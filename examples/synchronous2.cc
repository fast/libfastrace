// Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0.

#include <libfastrace/libfastrace.h>

#include <thread>

void func2(int i) {
  auto ls = fastrace_glue::ftr_create_loc_span_enter("func2");
  std::this_thread::sleep_for(std::chrono::milliseconds(i * 1));
  fastrace_glue::ftr_destroy_loc_span(ls);
}

void func1(int i) {
  auto ls = fastrace_glue::ftr_create_loc_span_enter("func1");
  std::this_thread::sleep_for(std::chrono::milliseconds(i * 1));
  func2(i);
  fastrace_glue::ftr_destroy_loc_span(ls);
}

int main(void) {
  auto ecfg = fastrace_glue::ftr_create_def_otlp_exp_cfg();
  auto cfg = fastrace_glue::ftr_create_def_coll_cfg();

  auto rptr = fastrace_glue::ftr_create_otel_rptr(ecfg);
  fastrace_glue::ftr_set_otel_rptr(rptr, cfg);

  auto p = fastrace_glue::ftr_create_rand_span_ctx();
  auto r = fastrace_glue::ftr_create_root_span("root", p);
  auto g = fastrace_glue::ftr_set_loc_par_to_span(r);
  auto ls = fastrace_glue::ftr_create_loc_span_enter("a span");
  // ftr_add_prop_to_loc_span(ls, "a property", "a value");

  for (auto i = 1; i <= 10; i++) {
    func1(i);
  }

  fastrace_glue::ftr_destroy_loc_span(ls);
  fastrace_glue::ftr_destroy_loc_par_guar(g);
  fastrace_glue::ftr_destroy_span(r);

  fastrace_glue::ftr_flush();

  fastrace_glue::ftr_destroy_otel_rptr(rptr);
  return 0;
}
