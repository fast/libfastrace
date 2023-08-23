// Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0.

#include <chrono>
#include <minitrace_c/minitrace_c.h>
#include <thread>

void func2(int i) {
  auto ls = minitrace_glue::mtr_create_loc_span_enter("func2");
  std::this_thread::sleep_for(std::chrono::milliseconds(i * 1));
  minitrace_glue::mtr_destroy_loc_span(ls);
}

void func1(int i) {
  auto ls = minitrace_glue::mtr_create_loc_span_enter("func1");
  std::this_thread::sleep_for(std::chrono::milliseconds(i * 1));
  func2(i);
  minitrace_glue::mtr_destroy_loc_span(ls);
}

int main(void) {
  auto gcfg = minitrace_glue::mtr_create_def_otlp_grpcio_cfg();
  auto ecfg = minitrace_glue::mtr_create_def_otlp_exp_cfg();
  auto cfg = minitrace_glue::mtr_create_def_coll_cfg();

  auto rptr = minitrace_glue::mtr_create_otel_rptr(ecfg, gcfg);
  minitrace_glue::mtr_set_otel_rptr(rptr, cfg);

  auto p = minitrace_glue::mtr_create_rand_span_ctx();
  auto r = minitrace_glue::mtr_create_root_span("root", p);
  auto g = minitrace_glue::mtr_set_loc_par_to_span(r);
  auto ls = minitrace_glue::mtr_create_loc_span_enter("a span");
  // mtr_add_prop_to_loc_span(ls, "a property", "a value");

  for (auto i = 1; i <= 10; i++) {
    func1(i);
  }

  minitrace_glue::mtr_destroy_loc_span(ls);
  minitrace_glue::mtr_destroy_loc_par_guar(g);
  minitrace_glue::mtr_destroy_span(r);

  minitrace_glue::mtr_flush();

  minitrace_glue::mtr_destroy_otel_rptr(rptr);
  return 0;
}
