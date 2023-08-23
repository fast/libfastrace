// Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0.

#include "minitrace_c.h"

extern "C" {

union _mtr_span_ctx {
  mtr_span_ctx c;
  ffi::mtr_span_ctx f;
};

union _mtr_span {
  mtr_span c;
  ffi::mtr_span f;
};

union _mtr_loc_par_guar {
  mtr_loc_par_guar c;
  ffi::mtr_loc_par_guar f;
};

union _mtr_loc_span {
  mtr_loc_span c;
  ffi::mtr_loc_span f;
};

union _mtr_loc_coll {
  mtr_loc_coll c;
  ffi::mtr_loc_coll f;
};

union _mtr_loc_spans {
  mtr_loc_spans c;
  ffi::mtr_loc_spans f;
};

union _mtr_coll_cfg {
  mtr_coll_cfg c;
  ffi::mtr_coll_cfg f;
};

union _mtr_otlp_exp_cfg {
  mtr_otlp_exp_cfg c;
  ffi::mtr_otlp_exp_cfg f;
};

union _mtr_otlp_grpcio_cfg {
  mtr_otlp_grpcio_cfg c;
  ffi::mtr_otlp_grpcio_cfg f;
};

union _mtr_otel_rptr {
  mtr_otel_rptr c;
  ffi::mtr_otel_rptr f;
};

mtr_span_ctx mtr_create_rand_span_ctx() {
  union _mtr_span_ctx _msc = {
      .f = minitrace_glue::mtr_create_rand_span_ctx(),
  };
  return std::move(_msc.c);
}

mtr_span_ctx mtr_create_span_ctx(mtr_span const *span) {
  union _mtr_span _p = {
      .c = *span,
  };
  union _mtr_span_ctx _msc = {
      .f = minitrace_glue::mtr_create_span_ctx(_p.f),
  };
  return std::move(_msc.c);
}

mtr_span_ctx mtr_create_span_ctx_loc() {
  union _mtr_span_ctx _msc = {
      .f = minitrace_glue::mtr_create_span_ctx_loc(),
  };
  return std::move(_msc.c);
}

mtr_span mtr_create_root_span(const char *name, mtr_span_ctx parent) {
  union _mtr_span _ms = {
      .f = minitrace_glue::mtr_create_root_span(
          name, *(ffi::mtr_span_ctx *)(&parent)),
  };
  return std::move(_ms.c);
}

mtr_span mtr_create_child_span_enter(const char *name, mtr_span const *parent) {
  union _mtr_span _p = {
      .c = *parent,
  };
  union _mtr_span _ms = {
      .f = minitrace_glue::mtr_create_child_span_enter(name, _p.f),
  };
  return std::move(_ms.c);
}

mtr_span mtr_create_child_span_enter_loc(const char *name) {
  union _mtr_span _ms = {
      .f = minitrace_glue::mtr_create_child_span_enter_loc(name),
  };
  return std::move(_ms.c);
}

void mtr_cancel_span(mtr_span span) {
  minitrace_glue::mtr_cancel_span(*(ffi::mtr_span *)(&span));
}

void mtr_destroy_span(mtr_span span) {
  minitrace_glue::mtr_destroy_span(*(ffi::mtr_span *)(&span));
}

mtr_loc_par_guar mtr_set_loc_par_to_span(mtr_span const *span) {
  union _mtr_span _p = {
      .c = *span,
  };
  union _mtr_loc_par_guar _mlpg = {
      .f = minitrace_glue::mtr_set_loc_par_to_span(_p.f),
  };
  return std::move(_mlpg.c);
}

void mtr_destroy_loc_par_guar(mtr_loc_par_guar guard) {
  minitrace_glue::mtr_destroy_loc_par_guar(*(ffi::mtr_loc_par_guar *)(&guard));
}

void mtr_push_child_spans_to_cur(mtr_span const *span,
                                 mtr_loc_spans local_span) {
  union _mtr_span _p = {
      .c = *span,
  };
  minitrace_glue::mtr_push_child_spans_to_cur(
      _p.f, *(ffi::mtr_loc_spans *)(&local_span));
}

mtr_loc_span mtr_create_loc_span_enter(const char *name) {
  union _mtr_loc_span _mls = {
      .f = minitrace_glue::mtr_create_loc_span_enter(name),
  };
  return std::move(_mls.c);
}

void mtr_destroy_loc_span(mtr_loc_span span) {
  minitrace_glue::mtr_destroy_loc_span(*(ffi::mtr_loc_span *)(&span));
}

mtr_loc_coll mtr_start_loc_coll(void) {
  union _mtr_loc_coll _mlc = {
      .f = minitrace_glue::mtr_start_loc_coll(),
  };
  return std::move(_mlc.c);
}

mtr_loc_spans mtr_collect_loc_spans(mtr_loc_coll lc) {
  union _mtr_loc_spans _mls = {
      .f = minitrace_glue::mtr_collect_loc_spans(*(ffi::mtr_loc_coll *)(&lc)),
  };
  return std::move(_mls.c);
}

mtr_coll_cfg mtr_create_def_coll_cfg(void) {
  union _mtr_coll_cfg _mcc = {
      .f = minitrace_glue::mtr_create_def_coll_cfg(),
  };
  return std::move(_mcc.c);
}

mtr_coll_cfg mtr_set_max_spans_per_trace(mtr_coll_cfg cfg, size_t mspt) {
  union _mtr_coll_cfg _mcc = {
      .f = minitrace_glue::mtr_set_max_spans_per_trace(
          *(ffi::mtr_coll_cfg *)(&cfg), mspt),
  };
  return std::move(_mcc.c);
}

mtr_coll_cfg mtr_set_batch_report_interval(mtr_coll_cfg cfg, uint64_t bri) {
  union _mtr_coll_cfg _mcc = {
      .f = minitrace_glue::mtr_set_batch_report_interval(
          *(ffi::mtr_coll_cfg *)(&cfg), bri),
  };
  return std::move(_mcc.c);
}

mtr_coll_cfg mtr_set_report_max_spans(mtr_coll_cfg cfg, size_t brms) {
  union _mtr_coll_cfg _mcc = {
      .f = minitrace_glue::mtr_set_report_max_spans(
          *(ffi::mtr_coll_cfg *)(&cfg), brms),
  };
  return std::move(_mcc.c);
}

void mtr_set_cons_rptr(void) { minitrace_glue::mtr_set_cons_rptr(); }

mtr_otlp_exp_cfg mtr_create_def_otlp_exp_cfg(void) {
  union _mtr_otlp_exp_cfg _moec = {
      .f = minitrace_glue::mtr_create_def_otlp_exp_cfg(),
  };
  return std::move(_moec.c);
}

mtr_otlp_grpcio_cfg mtr_create_def_otlp_grpcio_cfg(void) {
  union _mtr_otlp_grpcio_cfg _moec = {
      .f = minitrace_glue::mtr_create_def_otlp_grpcio_cfg(),
  };
  return std::move(_moec.c);
}

mtr_otel_rptr mtr_create_otel_rptr(mtr_otlp_exp_cfg cfg,
                                   mtr_otlp_grpcio_cfg gcfg) {
  union _mtr_otel_rptr _mor = {
      .f = minitrace_glue::mtr_create_otel_rptr(
          *(ffi::mtr_otlp_exp_cfg *)(&cfg),
          *(ffi::mtr_otlp_grpcio_cfg *)(&gcfg)),
  };
  return std::move(_mor.c);
}

void mtr_destroy_otel_rptr(mtr_otel_rptr rptr) {
  minitrace_glue::mtr_destroy_otel_rptr(*(ffi::mtr_otel_rptr *)(&rptr));
}

void mtr_set_otel_rptr(mtr_otel_rptr rptr, mtr_coll_cfg cfg) {
  minitrace_glue::mtr_set_otel_rptr(*(ffi::mtr_otel_rptr *)(&rptr),
                                    *(ffi::mtr_coll_cfg *)(&cfg));
}

void mtr_flush(void) { minitrace_glue::mtr_flush(); }
}
