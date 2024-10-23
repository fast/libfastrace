/* Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0. */

#ifndef __LIBFASTRACE_H
#define __LIBFASTRACE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
#include <vector>

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

/* Create a place-holder span that never starts recording. */
ftr_span ftr_creat_noop_span();

/*
 * Create a new trace and return its root span.
 *
 * Once destroyed (dropped), the root span automatically submits all associated
 * child spans to the reporter.
 */
ftr_span ftr_create_root_span(const char *name, ftr_span_ctx parent);

/* Create a new child span associated with the specified parent span. */
ftr_span ftr_create_child_span_enter(const char *name, ftr_span const *parent);

/* Create a new child span associated with multiple parent spans. */
ftr_span ftr_create_child_span_enter_mul(const char *name,
                                         ftr_span const *parents, size_t n);

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

/*
 * Add a single property to the `Span` and return the modified `Span`.
 *
 * A property is an arbitrary key-value pair associated with a span.
 */
void ftr_span_with_prop(ftr_span *span, const char *key, const char *val);

/* Add multiple properties to the `Span` and return the modified `Span`. */
void ftr_span_with_props(ftr_span *span, const char **keys, const char **vals,
                         size_t n);

/* Adds an event to the parent span with the given name and properties. */
void ftr_add_ent_to_par(const char *name, ftr_span *span, const char **keys,
                        const char **vals, size_t n);

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

/*
 * Add a single property to the current local parent.If the local parent is
 * a[`Span`], the property will be added to the `Span`.
 */
void ftr_loc_span_add_prop(const char *key, const char *val);

/*
 * Add multiple properties to the current local parent. If the local parent is a
 * [`Span`], the properties will be added to the `Span`.
 */
void ftr_loc_span_add_props(const char **keys, const char **val, size_t n);

/*
 * Add a single property to the `LocalSpan` and return the modified `LocalSpan`.
 *
 * A property is an arbitrary key - value pair associated with a span.
 */
void ftr_loc_span_with_prop(ftr_loc_span *span, const char *key,
                            const char *val);

/*
 * Add multiple properties to the `LocalSpan` and return the modified
 * `LocalSpan`.
 */
void ftr_loc_span_with_props(ftr_loc_span *span, const char **keys,
                             const char **vals, size_t n);

/*
 * Adds an event to the current local parent span with the given name and
 * properties.
 */
void ftr_add_ent_to_loc_par(const char *name, const char **keys,
                            const char **vals, size_t n);

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

#ifdef __cplusplus
namespace fastrace {

// C++ wrapper classes

/**
 * @brief Represents a span context in the tracing system.
 *
 * SpanContext encapsulates the identifying information for a span,
 * including trace ID, span ID, and sampling decisions.
 */
class SpanContext {
 public:
  /** @brief Creates a new SpanContext with a random trace ID. */
  SpanContext();

  /** @brief Creates a SpanContext from an existing ftr_span_ctx. */
  SpanContext(const ftr_span_ctx &ctx);

  /** @brief Returns the raw ftr_span_ctx representation. */
  ftr_span_ctx raw() const;

  /** @brief Sets the sampling flag for this SpanContext. */
  void setSampled(bool sampled);

 private:
  ftr_span_ctx ctx_;
};

/**
 * @brief Represents a span in the tracing system.
 *
 * A Span represents a single operation within a trace. It can be
 * the root of a trace or a child of another span.
 */
class Span {
 public:
  /** @brief Creates a root span with the given name and parent context. */
  Span(const char *name, const SpanContext &parent);

  /** @brief Creates a child span with the given name and parent span. */
  Span(const char *name, const Span &parent);

  /** @brief Creates a child span with the given name, using the current local
   * span as parent. */
  explicit Span(const char *name);

  /** @brief Move constructor */
  Span(Span &&other) noexcept;

  /** @brief Move assignment operator */
  Span &operator=(Span &&other) noexcept;

  /** @brief Copy constructor (deleted to ensure unique ownership) */
  Span(const Span &) = delete;

  /** @brief Copy assignment operator (deleted to ensure unique ownership) */
  Span &operator=(const Span &) = delete;

  /** @brief Destroys the span, submitting it to the reporter if it's a root
   * span. */
  ~Span();

  /** @brief Cancels the span, preventing it from being reported. */
  void cancel();

  /** @brief Adds a single key-value property to the span. */
  void addProperty(const char *key, const char *value);

  /** @brief Adds multiple key-value properties to the span. */
  void addProperties(
      const std::vector<std::pair<const char *, const char *>> &properties);

  /** @brief Adds an event with the given name and properties to the span. */
  void addEvent(
      const char *name,
      const std::vector<std::pair<const char *, const char *>> &properties);

  /** @brief Returns a pointer to the raw ftr_span representation. */
  ftr_span *raw();

  /** @brief Returns a const pointer to the raw ftr_span representation. */
  const ftr_span *raw() const;

  /**
   * @brief Returns the elapsed time since the span was created in nanoseconds.
   * @return Elapsed time in nanoseconds, or 0 if the span is not active.
   */
  uint64_t elapsed() const;

 private:
  ftr_span span_;
};

/**
 * @brief RAII guard for setting and unsetting a local parent span.
 *
 * This guard sets the given span as the local parent for the current thread
 * and automatically unsets it when the guard goes out of scope.
 */
class LocalParentGuard {
 public:
  /** @brief Sets the given span as the local parent for the current thread. */
  LocalParentGuard(const Span &span);

  /** @brief Unsets the local parent span. */
  ~LocalParentGuard();

 private:
  ftr_loc_par_guar guard_;
};

/**
 * @brief Represents a local span in the tracing system.
 *
 * A LocalSpan is associated with the current thread and becomes
 * the new local parent when created.
 */
class LocalSpan {
 public:
  /** @brief Creates a new local span with the given name. */
  explicit LocalSpan(const char *name);

  /** @brief Copy constructor (deleted to ensure unique ownership) */
  LocalSpan(const LocalSpan &) = delete;

  /** @brief Copy assignment operator (deleted to ensure unique ownership) */
  LocalSpan &operator=(const LocalSpan &) = delete;

  /** @brief Move constructor */
  LocalSpan(LocalSpan &&other) noexcept;

  /** @brief Move assignment operator */
  LocalSpan &operator=(LocalSpan &&other) noexcept;

  /** @brief Destroys the local span. */
  ~LocalSpan();

  /** @brief Adds a single key-value property to the current local parent span.
   */
  void addProperty(const char *key, const char *value);

  /** @brief Adds multiple key-value properties to the current local parent
   * span. */
  void addProperties(
      const std::vector<std::pair<const char *, const char *>> &properties);

  /** @brief Adds a single key-value property to the local span and returns the
   * modified local span. */
  void withProperty(const char *key, const char *value);

  /** @brief Adds multiple key-value properties to the local span and returns
   * the modified local span. */
  void withProperties(
      const std::vector<std::pair<const char *, const char *>> &properties);

  /** @brief Adds an event with the given name and properties to the current
   * local parent span. */
  void addEvent(
      const char *name,
      const std::vector<std::pair<const char *, const char *>> &properties);

 private:
  ftr_loc_span span_;
};

/**
 * @brief Configuration for the global collector.
 *
 * This class allows setting various parameters for the trace collector.
 */
class CollectorConfig {
 public:
  /** @brief Creates a default collector configuration. */
  CollectorConfig();

  /** @brief Sets the maximum number of spans per trace. */
  void setMaxSpansPerTrace(size_t max);

  /** @brief Sets the interval between batch reports in milliseconds. */
  void setReportInterval(uint64_t interval);

  /** @brief Returns the raw ftr_coll_cfg representation. */
  ftr_coll_cfg raw() const;

 private:
  ftr_coll_cfg cfg_;
};

/**
 * @brief Configuration for the OTLP (OpenTelemetry Protocol) exporter.
 */
class OTLPExporterConfig {
 public:
  /** @brief Creates a default OTLP exporter configuration. */
  OTLPExporterConfig();

  /** @brief Returns the raw ftr_otlp_exp_cfg representation. */
  ftr_otlp_exp_cfg raw() const;

 private:
  ftr_otlp_exp_cfg cfg_;
};

/**
 * @brief Reporter for exporting traces using the OpenTelemetry protocol.
 */
class OpenTelemetryReporter {
 public:
  /** @brief Creates an OpenTelemetry reporter with the given configuration. */
  OpenTelemetryReporter(const OTLPExporterConfig &config);

  /** @brief Destroys the OpenTelemetry reporter. */
  ~OpenTelemetryReporter();

  /** @brief Sets the collector configuration for this reporter. */
  void setReporter(const CollectorConfig &config);

 private:
  ftr_otel_rptr reporter_;
};

// Global functions

/** @brief Creates a default OTLP exporter configuration. */
OTLPExporterConfig createDefaultOTLPExporterConfig();

/** @brief Creates a default collector configuration. */
CollectorConfig createDefaultCollectorConfig();

/** @brief Creates an OpenTelemetry reporter with the given configuration. */
OpenTelemetryReporter createOpenTelemetryReporter(
    const OTLPExporterConfig &config);

/** @brief Sets the OpenTelemetry reporter with the given configuration. */
void setOpenTelemetryReporter(OpenTelemetryReporter &reporter,
                              const CollectorConfig &config);

/** @brief Sets the console reporter for debugging purposes. */
void setConsoleReporter();

/** @brief Flushes all pending span records to the reporter immediately. */
void flush();

}  // namespace fastrace
#endif

#endif /* __LIBFASTRACE_H */
