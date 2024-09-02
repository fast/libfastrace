// Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0.

#include "fastrace_c.h"

extern "C" {

union _ftr_span_ctx {
  ftr_span_ctx c;
  ffi::ftr_span_ctx f;
};

union _ftr_span {
  ftr_span c;
  ffi::ftr_span f;
};

union _ftr_loc_par_guar {
  ftr_loc_par_guar c;
  ffi::ftr_loc_par_guar f;
};

union _ftr_loc_span {
  ftr_loc_span c;
  ffi::ftr_loc_span f;
};

union _ftr_loc_coll {
  ftr_loc_coll c;
  ffi::ftr_loc_coll f;
};

union _ftr_loc_spans {
  ftr_loc_spans c;
  ffi::ftr_loc_spans f;
};

union _ftr_coll_cfg {
  ftr_coll_cfg c;
  ffi::ftr_coll_cfg f;
};

union _ftr_otlp_exp_cfg {
  ftr_otlp_exp_cfg c;
  ffi::ftr_otlp_exp_cfg f;
};

union _ftr_otel_rptr {
  ftr_otel_rptr c;
  ffi::ftr_otel_rptr f;
};

ftr_span_ctx ftr_create_rand_span_ctx() {
  union _ftr_span_ctx _msc = {
      .f = fastrace_glue::ftr_create_rand_span_ctx(),
  };
  return std::move(_msc.c);
}

ftr_span_ctx ftr_create_span_ctx(ftr_span const *span) {
  union _ftr_span _p = {
      .c = *span,
  };
  union _ftr_span_ctx _msc = {
      .f = fastrace_glue::ftr_create_span_ctx(_p.f),
  };
  return std::move(_msc.c);
}

ftr_span_ctx ftr_create_span_ctx_loc() {
  union _ftr_span_ctx _msc = {
      .f = fastrace_glue::ftr_create_span_ctx_loc(),
  };
  return std::move(_msc.c);
}

ftr_span ftr_create_root_span(const char *name, ftr_span_ctx parent) {
  union _ftr_span _ms = {
      .f = fastrace_glue::ftr_create_root_span(name,
                                               *(ffi::ftr_span_ctx *)(&parent)),
  };
  return std::move(_ms.c);
}

ftr_span ftr_create_child_span_enter(const char *name, ftr_span const *parent) {
  union _ftr_span _p = {
      .c = *parent,
  };
  union _ftr_span _ms = {
      .f = fastrace_glue::ftr_create_child_span_enter(name, _p.f),
  };
  return std::move(_ms.c);
}

ftr_span ftr_create_child_span_enter_loc(const char *name) {
  union _ftr_span _ms = {
      .f = fastrace_glue::ftr_create_child_span_enter_loc(name),
  };
  return std::move(_ms.c);
}

void ftr_cancel_span(ftr_span span) {
  fastrace_glue::ftr_cancel_span(*(ffi::ftr_span *)(&span));
}

void ftr_destroy_span(ftr_span span) {
  fastrace_glue::ftr_destroy_span(*(ffi::ftr_span *)(&span));
}

ftr_loc_par_guar ftr_set_loc_par_to_span(ftr_span const *span) {
  union _ftr_span _p = {
      .c = *span,
  };
  union _ftr_loc_par_guar _mlpg = {
      .f = fastrace_glue::ftr_set_loc_par_to_span(_p.f),
  };
  return std::move(_mlpg.c);
}

void ftr_destroy_loc_par_guar(ftr_loc_par_guar guard) {
  fastrace_glue::ftr_destroy_loc_par_guar(*(ffi::ftr_loc_par_guar *)(&guard));
}

void ftr_push_child_spans_to_cur(ftr_span const *span,
                                 ftr_loc_spans local_span) {
  union _ftr_span _p = {
      .c = *span,
  };
  fastrace_glue::ftr_push_child_spans_to_cur(
      _p.f, *(ffi::ftr_loc_spans *)(&local_span));
}

ftr_loc_span ftr_create_loc_span_enter(const char *name) {
  union _ftr_loc_span _mls = {
      .f = fastrace_glue::ftr_create_loc_span_enter(name),
  };
  return std::move(_mls.c);
}

void ftr_destroy_loc_span(ftr_loc_span span) {
  fastrace_glue::ftr_destroy_loc_span(*(ffi::ftr_loc_span *)(&span));
}

ftr_loc_coll ftr_start_loc_coll(void) {
  union _ftr_loc_coll _mlc = {
      .f = fastrace_glue::ftr_start_loc_coll(),
  };
  return std::move(_mlc.c);
}

ftr_loc_spans ftr_collect_loc_spans(ftr_loc_coll lc) {
  union _ftr_loc_spans _mls = {
      .f = fastrace_glue::ftr_collect_loc_spans(*(ffi::ftr_loc_coll *)(&lc)),
  };
  return std::move(_mls.c);
}

ftr_coll_cfg ftr_create_def_coll_cfg(void) {
  union _ftr_coll_cfg _mcc = {
      .f = fastrace_glue::ftr_create_def_coll_cfg(),
  };
  return std::move(_mcc.c);
}

ftr_coll_cfg ftr_set_report_interval(ftr_coll_cfg cfg, uint64_t ri) {
  union _ftr_coll_cfg _mcc = {
      .f = fastrace_glue::ftr_set_report_interval(*(ffi::ftr_coll_cfg *)(&cfg),
                                                  ri),
  };
  return std::move(_mcc.c);
}

void ftr_set_cons_rptr(void) { fastrace_glue::ftr_set_cons_rptr(); }

ftr_otlp_exp_cfg ftr_create_def_otlp_exp_cfg(void) {
  union _ftr_otlp_exp_cfg _moec = {
      .f = fastrace_glue::ftr_create_def_otlp_exp_cfg(),
  };
  return std::move(_moec.c);
}

ftr_otel_rptr ftr_create_otel_rptr(ftr_otlp_exp_cfg cfg) {
  union _ftr_otel_rptr _mor = {
      .f =
          fastrace_glue::ftr_create_otel_rptr(*(ffi::ftr_otlp_exp_cfg *)(&cfg)),
  };
  return std::move(_mor.c);
}

void ftr_destroy_otel_rptr(ftr_otel_rptr rptr) {
  fastrace_glue::ftr_destroy_otel_rptr(*(ffi::ftr_otel_rptr *)(&rptr));
}

void ftr_set_otel_rptr(ftr_otel_rptr rptr, ftr_coll_cfg cfg) {
  fastrace_glue::ftr_set_otel_rptr(*(ffi::ftr_otel_rptr *)(&rptr),
                                   *(ffi::ftr_coll_cfg *)(&cfg));
}

void ftr_flush(void) { fastrace_glue::ftr_flush(); }
}
