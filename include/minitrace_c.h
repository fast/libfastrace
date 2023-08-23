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
	uint8_t _padding[24];
} mtr_span_ctx __attribute__((aligned(8)));

typedef struct mtr_span {
	uint8_t _padding[128];
} mtr_span __attribute__((aligned(8)));

typedef struct mtr_loc_par_guar {
	uint8_t _padding[24];
} mtr_loc_par_guar __attribute__((aligned(8)));

typedef struct mtr_loc_span {
	uint8_t _padding[24];
} mtr_loc_span __attribute__((aligned(8)));

typedef struct mtr_loc_spans {
	uint8_t _padding[8];
} mtr_loc_spans __attribute__((aligned(8)));

typedef struct mtr_loc_coll {
	uint8_t _padding[16];
} mtr_loc_coll __attribute__((aligned(8)));

typedef struct mtr_coll_cfg {
	uint8_t _padding[48];
} mtr_coll_cfg __attribute__((aligned(8)));

typedef struct mtr_otel_rptr {
	uint8_t _padding[192];
} mtr_otel_rptr __attribute__((aligned(8)));

typedef struct mtr_otlp_exp_cfg {
	uint8_t _padding[48];
} mtr_otlp_exp_cfg __attribute__((aligned(8)));

typedef struct mtr_otlp_grpcio_cfg {
	uint8_t _padding[112];
} mtr_otlp_grpcio_cfg __attribute__((aligned(8)));

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
