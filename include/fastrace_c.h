/* Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0. */

#ifndef __fastrace_C_H
#define __fastrace_C_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
#include "lib.rs.h"

extern "C" {
#endif

typedef struct ftr_span_ctx {
  uint64_t _padding[4];
} ftr_span_ctx;

typedef struct ftr_span {
  uint64_t _padding[18];
} ftr_span;

typedef struct ftr_loc_par_guar {
  uint64_t _padding[3];
} ftr_loc_par_guar;

typedef struct ftr_loc_span {
  uint64_t _padding[3];
} ftr_loc_span;

typedef struct ftr_loc_spans {
  uint64_t _padding[1];
} ftr_loc_spans;

typedef struct ftr_loc_coll {
  uint64_t _padding[2];
} ftr_loc_coll;

typedef struct ftr_coll_cfg {
  uint64_t _padding[5];
} ftr_coll_cfg;

typedef struct ftr_otel_rptr {
  uint64_t _padding[15];
} ftr_otel_rptr;

typedef struct ftr_otlp_exp_cfg {
  uint64_t _padding[6];
} ftr_otlp_exp_cfg;

/* Create a new `ftr_span_ctx` with a random trace id. */
ftr_span_ctx ftr_create_rand_span_ctx();

/* Creates a `ftr_span_ctx` from the given [`ftr_span`]. */
ftr_span_ctx ftr_create_span_ctx(ftr_span const *span);

/* Creates a `ftr_span_ctx` from the current local parent span. */
ftr_span_ctx ftr_create_span_ctx_loc(void);

/* Sets the `sampled` flag of the `SpanContext`. */
ftr_span_ctx ftr_span_ctx_set_sampled(ftr_span_ctx ctx, bool sampled);

/*
 * Create a new trace and return its root span.
 *
 * Once destroyed (dropped), the root span automatically submits all associated
 * child spans to the reporter.
 */
ftr_span ftr_create_root_span(const char *name, ftr_span_ctx parent);

/* Create a new child span associated with the specified parent span. */
ftr_span ftr_create_child_span_enter(const char *name, ftr_span const *parent);

/* Create a new child span associated with the current local span in the current
 * thread. */
ftr_span ftr_create_child_span_enter_loc(const char *name);

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
void ftr_cancel_span(ftr_span span);

/* Once destroyed (dropped), the root span automatically submits all associated
 * child spans to the reporter. */
void ftr_destroy_span(ftr_span span);

/*
 * Sets the current `ftr_pan` as the local parent for the current thread.
 *
 * This method is used to establish a `ftr_span` as the local parent within the
 * current scope.
 *
 * A local parent is necessary for creating a [`ftr_local_span`] using
 * [`ftr_create_loc_span_enter()`]. If no local parent is set,
 * `ftr_create_loc_span_enter()` will not perform any action.
 */
ftr_loc_par_guar ftr_set_loc_par_to_span(ftr_span const *span);

void ftr_destroy_loc_par_guar(ftr_loc_par_guar guard);

/*
 * Attach a collection of [`ftr_local_span`] instances as child spans to the
 * current span.
 *
 * This method allows you to associate previously collected `ftr_local_span`
 * instances with the current span. This is particularly useful when the
 * `ftr_local_span` instances were initiated before their parent span, and were
 * collected manually using a [`ftr_loc_coll`].
 *
 * # Examples:
 *
 * ````
 * // Collect local spans manually without a parent
 * ftr_loc_coll coll = ftr_start_loc_coll();
 * ftr_loc_span ls = ftr_create_loc_span_enter();
 * ftr_destroy_loc_span(ls);
 *
 * // Collect local spans manually without a parent
 * ftr_loc_spans lscoll = ftr_collect_loc_spans(coll);
 *
 * // Attach the local spans to a parent
 * ftr_span root = ftr_create_root_span("root", ftr_create_rand_span_ctx());
 * ftr_push_child_spans_to_cur(&root, lscoll);
 * ````
 */
void ftr_push_child_spans_to_cur(ftr_span const *span,
                                 ftr_loc_spans local_span);

/*
 * Create a new child span associated with the current local span in the current
 * thread, and then it will become the new local parent.
 */
ftr_loc_span ftr_create_loc_span_enter(const char *name);

void ftr_destroy_loc_span(ftr_loc_span span);

/* Collect local spans manually without a parent, see
 * `ftr_push_child_spans_to_cur` to learn more. */
ftr_loc_coll ftr_start_loc_coll(void);

/* Collect local spans into a LocalSpans instance, see
 * `ftr_push_child_spans_to_cur` to learn more. */
ftr_loc_spans ftr_collect_loc_spans(ftr_loc_coll lc);

/* Create default configuration of the behavior of the global collector. */
ftr_coll_cfg ftr_create_def_coll_cfg(void);

/*
 * A soft limit for the total number of spans and events for a trace, usually
 * used to avoid out-of-memory.
 *
 * # Note
 *
 * Root span will always be collected. The eventually collected spans may exceed
 * the limit.
 */
ftr_coll_cfg ftr_set_max_spans_per_trace(ftr_coll_cfg cfg, size_t mspt);

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
ftr_coll_cfg ftr_set_report_interval(ftr_coll_cfg cfg, uint64_t ri);

/* Sets console reporter for the current application, usually used for
 * debugging. */
void ftr_set_cons_rptr(void);

ftr_otlp_exp_cfg ftr_create_def_otlp_exp_cfg(void);

/*
 * Create an `ftr_otel_rptr` to export trace records to remote agents that
 * OpenTelemetry supports, which includes Jaeger, Datadog, Zipkin, and
 * OpenTelemetry Collector.
 */
ftr_otel_rptr ftr_create_otel_rptr(ftr_otlp_exp_cfg cfg);

void ftr_destroy_otel_rptr(ftr_otel_rptr rptr);

/* Sets the reporter and its configuration for the current application. */
void ftr_set_otel_rptr(ftr_otel_rptr rptr, ftr_coll_cfg cfg);

/*
 * Flushes all pending span records to the reporter immediately.
 *
 * # Note
 *
 * It will create a new thread in the current thread to do this,
 * and it will block the current thread until the new thread finishes flushing
 * and exits.
 */
void ftr_flush(void);

#ifdef __cplusplus
}
#endif

#endif /* __fastrace_C_H */
