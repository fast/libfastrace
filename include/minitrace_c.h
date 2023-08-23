/* Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0. */

#ifndef __MINITRACE_C_H
#define __MINITRACE_C_H

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
#include "lib.rs.h"

extern "C" {
#endif

typedef struct mtr_span_ctx {
	uint64_t _padding[3];
} mtr_span_ctx;

typedef struct mtr_span {
	uint64_t _padding[16];
} mtr_span;

typedef struct mtr_loc_par_guar {
	uint64_t _padding[3];
} mtr_loc_par_guar;

typedef struct mtr_loc_span {
	uint64_t _padding[3];
} mtr_loc_span;

typedef struct mtr_loc_spans {
	uint64_t _padding[1];
} mtr_loc_spans;

typedef struct mtr_loc_coll {
	uint64_t _padding[2];
} mtr_loc_coll;

typedef struct mtr_coll_cfg {
	uint64_t _padding[6];
} mtr_coll_cfg;

typedef struct mtr_otel_rptr {
	uint64_t _padding[24];
} mtr_otel_rptr;

typedef struct mtr_otlp_exp_cfg {
	uint64_t _padding[6];
} mtr_otlp_exp_cfg;

typedef struct mtr_otlp_grpcio_cfg {
	uint64_t _padding[14];
} mtr_otlp_grpcio_cfg;

mtr_span_ctx mtr_create_rand_span_ctx();

mtr_span mtr_create_root_span(const char *name, mtr_span_ctx parent);

mtr_span mtr_create_child_span_enter(const char *name, mtr_span const *parent);

mtr_span mtr_create_child_span_enter_loc(const char *name);

void mtr_destroy_span(mtr_span span);

mtr_loc_par_guar mtr_set_loc_par_to_span(mtr_span const *span);

void mtr_destroy_loc_par_guar(mtr_loc_par_guar guard);

void mtr_push_child_spans_to_cur(mtr_span const *span, mtr_loc_spans local_span);

mtr_loc_span mtr_create_loc_span_enter(const char *name);

void mtr_destroy_loc_span(mtr_loc_span span);

mtr_loc_coll mtr_start_loc_coll(void);

mtr_loc_spans mtr_collect_loc_spans(mtr_loc_coll lc);

mtr_coll_cfg mtr_create_def_coll_cfg(void);

mtr_coll_cfg mtr_set_max_spans_per_trace(mtr_coll_cfg cfg, size_t mspt);

mtr_coll_cfg mtr_set_batch_report_interval(mtr_coll_cfg cfg, uint64_t bri);

mtr_coll_cfg mtr_set_report_max_spans(mtr_coll_cfg cfg, size_t brms);

void mtr_set_cons_rptr(void);

mtr_otlp_exp_cfg mtr_create_def_otlp_exp_cfg(void);

mtr_otlp_grpcio_cfg mtr_create_def_otlp_grpcio_cfg(void);

mtr_otel_rptr mtr_create_otel_rptr(mtr_otlp_exp_cfg cfg, mtr_otlp_grpcio_cfg gcfg);

void mtr_destroy_otel_rptr(mtr_otel_rptr rptr);

void mtr_set_otel_rptr(mtr_otel_rptr rptr, mtr_coll_cfg cfg);

void mtr_flush(void);

#ifdef __cplusplus
}
#endif

#endif /* __MINITRACE_C_H */
