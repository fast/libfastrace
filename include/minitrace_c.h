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

/* Create a new `mtr_span_ctx` with a random trace id. */
mtr_span_ctx mtr_create_rand_span_ctx();

/* Creates a `mtr_span_ctx` from the given [`mtr_span`]. */
mtr_span_ctx mtr_create_span_ctx(mtr_span const *span);

/* Creates a `mtr_span_ctx` from the current local parent span. */
mtr_span_ctx mtr_create_span_ctx_loc(void);

/*
 * Create a new trace and return its root span.
 *
 * Once destroyed (dropped), the root span automatically submits all associated child spans to the reporter.
 */
mtr_span mtr_create_root_span(const char *name, mtr_span_ctx parent);

/* Create a new child span associated with the specified parent span. */
mtr_span mtr_create_child_span_enter(const char *name, mtr_span const *parent);

/* Create a new child span associated with the current local span in the current thread. */
mtr_span mtr_create_child_span_enter_loc(const char *name);

/*
 * Dismisses the trace, preventing the reporting of any span records associated
 * with it.
 *
 * This is particularly useful when focusing on the tail latency of a program.
 * For instant, you can dismiss all traces finishes within the 99th percentile.
 *
 * # Note
 *
 * This method only dismisses the entire trace when called on the root span.
 * If called on a non-root span, it will only cancel the reporting of that
 * specific span.
 */
void mtr_cancel_span(mtr_span span);

/* Once destroyed (dropped), the root span automatically submits all associated child spans to the reporter. */
void mtr_destroy_span(mtr_span span);

/*
 * Sets the current `mtr_pan` as the local parent for the current thread.
 *
 * This method is used to establish a `mtr_span` as the local parent within the current scope.
 *
 * A local parent is necessary for creating a [`mtr_local_span`] using [`mtr_create_loc_span_enter()`].
 * If no local parent is set, `mtr_create_loc_span_enter()` will not perform any action.
 */
mtr_loc_par_guar mtr_set_loc_par_to_span(mtr_span const *span);

void mtr_destroy_loc_par_guar(mtr_loc_par_guar guard);

/*
 * Attach a collection of [`mtr_local_span`] instances as child spans to the current span.
 *
 * This method allows you to associate previously collected `mtr_local_span` instances with the current span.
 * This is particularly useful when the `mtr_local_span` instances were initiated before their parent span,
 * and were collected manually using a [`mtr_loc_coll`].
 *
 * # Examples:
 *
 * ````
 * // Collect local spans manually without a parent
 * mtr_loc_coll coll = mtr_start_loc_coll();
 * mtr_loc_span ls = mtr_create_loc_span_enter();
 * mtr_destroy_loc_span(ls);
 *
 * // Collect local spans manually without a parent
 * mtr_loc_spans lscoll = mtr_collect_loc_spans(coll);
 *
 * // Attach the local spans to a parent
 * mtr_span root = mtr_create_root_span("root", mtr_create_rand_span_ctx());
 * mtr_push_child_spans_to_cur(&root, lscoll);
 * ````
 */
void mtr_push_child_spans_to_cur(mtr_span const *span, mtr_loc_spans local_span);

/*
 * Create a new child span associated with the current local span in the current thread, and then
 * it will become the new local parent.
 */
mtr_loc_span mtr_create_loc_span_enter(const char *name);

void mtr_destroy_loc_span(mtr_loc_span span);

/* Collect local spans manually without a parent, see `mtr_push_child_spans_to_cur` to learn more. */
mtr_loc_coll mtr_start_loc_coll(void);

/* Collect local spans into a LocalSpans instance, see `mtr_push_child_spans_to_cur` to learn more. */
mtr_loc_spans mtr_collect_loc_spans(mtr_loc_coll lc);

/* Create default configuration of the behavior of the global collector. */
mtr_coll_cfg mtr_create_def_coll_cfg(void);

/*
 * A soft limit for the total number of spans and events for a trace, usually used
 * to avoid out-of-memory.
 *
 * # Note
 *
 * Root span will always be collected. The eventually collected spans may exceed the limit.
 */
mtr_coll_cfg mtr_set_max_spans_per_trace(mtr_coll_cfg cfg, size_t mspt);

/*
 * The time duration between two batch reports.
 *
 * The default value is 500 milliseconds.
 *
 * A batch report will be initiated by the earliest of these events:
 *
 * - When the specified time duration between two batch reports is met.
 * - When the number of spans in a batch hits its limit.
 */
mtr_coll_cfg mtr_set_batch_report_interval(mtr_coll_cfg cfg, uint64_t bri);

/*
 * The soft limit for the maximum number of spans in a batch report.
 *
 * A batch report will be initiated by the earliest of these events:
 *
 * - When the specified time duration between two batch reports is met.
 * - When the number of spans in a batch hits its limit.
 *
 * # Note
 *
 * The eventually spans being reported may exceed the limit.
 */
mtr_coll_cfg mtr_set_report_max_spans(mtr_coll_cfg cfg, size_t brms);

/* Sets console reporter for the current application, usually used for debugging. */
void mtr_set_cons_rptr(void);

mtr_otlp_exp_cfg mtr_create_def_otlp_exp_cfg(void);

mtr_otlp_grpcio_cfg mtr_create_def_otlp_grpcio_cfg(void);

/*
 * Create an `mtr_otel_rptr` to export trace records to remote agents that OpenTelemetry
 * supports, which includes Jaeger, Datadog, Zipkin, and OpenTelemetry Collector.
 */
mtr_otel_rptr mtr_create_otel_rptr(mtr_otlp_exp_cfg cfg, mtr_otlp_grpcio_cfg gcfg);

void mtr_destroy_otel_rptr(mtr_otel_rptr rptr);

/* Sets the reporter and its configuration for the current application. */
void mtr_set_otel_rptr(mtr_otel_rptr rptr, mtr_coll_cfg cfg);

/*
 * Flushes all pending span records to the reporter immediately.
 *
 * # Note
 *
 * It will create a new thread in the current thread to do this,
 * and it will block the current thread until the new thread finishes flushing
 * and exits.
 */
void mtr_flush(void);

#ifdef __cplusplus
}
#endif

#endif /* __MINITRACE_C_H */
