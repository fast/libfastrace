// Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0.

use fastrace::{
    collector::{Config, ConsoleReporter},
    local::{LocalCollector, LocalParentGuard},
    prelude::{LocalSpan, Span, SpanContext},
};
use fastrace_opentelemetry::OpenTelemetryReporter;
use opentelemetry_otlp::WithExportConfig;
use std::borrow::Cow;
use std::{mem::transmute, time::Duration};
use once_cell::sync::Lazy;
use std::sync::Mutex;
use tokio::runtime::Runtime;

use self::ffi::*;

static RUNTIME: Lazy<Mutex<Runtime>> = Lazy::new(|| {
    Mutex::new(Runtime::new().expect("Failed to create Tokio runtime"))
});

fn initialize_runtime() {
    Lazy::force(&RUNTIME);
}

#[cxx::bridge]
mod ffi {

    #[namespace = "ffi"]
    struct ftr_span_ctx {
        _padding: [u64; 4],
    }

    #[namespace = "ffi"]
    struct ftr_span {
        _padding: [u64; 18],
    }

    #[namespace = "ffi"]
    struct ftr_loc_par_guar {
        _padding: [u64; 3],
    }

    #[namespace = "ffi"]
    struct ftr_loc_span {
        _padding: [u64; 3],
    }

    #[namespace = "ffi"]
    struct ftr_loc_spans {
        _padding: [u64; 1],
    }

    #[namespace = "ffi"]
    struct ftr_loc_coll {
        _padding: [u64; 2],
    }

    #[namespace = "ffi"]
    struct ftr_coll_cfg {
        _padding: [u64; 5],
    }

    #[namespace = "ffi"]
    struct ftr_otel_rptr {
        _padding: [u64; 15],
    }

    #[namespace = "ffi"]
    struct ftr_otlp_exp_cfg {
        _padding: [u64; 6],
    }

    #[namespace = "fastrace_glue"]
    extern "Rust" {

        /// Create a new `ftr_span_context` with a random trace id.
        fn ftr_create_rand_span_ctx() -> ftr_span_ctx;

        /// Creates a `ftr_span_context` from the given [`ftr_span`].
        fn ftr_create_span_ctx(span: &ftr_span) -> ftr_span_ctx;

        ///Creates a `ftr_span_context` from the current local parent span.
        fn ftr_create_span_ctx_loc() -> ftr_span_ctx;

        /// Sets the `sampled` flag of the `SpanContext`.
        fn ftr_span_ctx_set_sampled(ctx: ftr_span_ctx, sampled: bool) -> ftr_span_ctx;

        /// Create a new trace and return its root span.
        ///
        /// Once destroyed (dropped), the root span automatically submits all associated child spans to the reporter.
        fn ftr_create_root_span(name: &'static str, parent: ftr_span_ctx) -> ftr_span;

        /// Create a new child span associated with the specified parent span.
        fn ftr_create_child_span_enter(name: &'static str, parent: &ftr_span) -> ftr_span;

        /// Create a new child span associated with the current local span in the current thread.
        fn ftr_create_child_span_enter_loc(name: &'static str) -> ftr_span;

        /// Dismisses the trace, preventing the reporting of any span records associated with it.
        ///
        /// This is particularly useful when focusing on the tail latency of a program. For instant,
        /// you can dismiss all traces finishes within the 99th percentile.
        ///
        /// # Note
        ///
        /// This method only dismisses the entire trace when called on the root span.
        /// If called on a non-root span, it will only cancel the reporting of that specific span.
        fn ftr_cancel_span(span: ftr_span);

        /// Once destroyed (dropped), the root span automatically submits all associated child spans to the reporter.
        fn ftr_destroy_span(span: ftr_span);

        /// Sets the current `ftr_pan` as the local parent for the current thread.
        ///
        /// This method is used to establish a `span` as the local parent within the current scope.
        ///
        /// A local parent is necessary for creating a [`ftr_local_span`] using [`ftr_create_loc_span_enter()`].
        /// If no local parent is set, `ftr_create_loc_span_enter()` will not perform any action.
        fn ftr_set_loc_par_to_span(span: &ftr_span) -> ftr_loc_par_guar;

        fn ftr_destroy_loc_par_guar(guard: ftr_loc_par_guar);

        /// Attach a collection of [`ftr_local_span`] instances as child spans to the current span.
        ///
        /// This method allows you to associate previously collected `ftr_local_span` instances with the current span.
        /// This is particularly useful when the `ftr_local_span` instances were initiated before their parent span,
        /// and were collected manually using a [`ftr_loc_coll`].
        ///
        /// # Examples:
        ///
        /// ````
        /// // Collect local spans manually without a parent
        /// let coll = ftr_start_loc_coll();
        /// let ls = ftr_create_loc_span_enter();
        /// ftr_destroy_loc_span(ls);
        ///
        /// // Collect local spans manually without a parent
        /// let lscoll = ftr_collect_loc_spans(coll);
        ///
        /// // Attach the local spans to a parent
        /// let root = ftr_create_root_span("root", ftr_create_rand_span_ctx());
        /// ftr_push_child_spans_to_cur(&root, lscoll);
        /// ````
        fn ftr_push_child_spans_to_cur(span: &ftr_span, local_span: ftr_loc_spans);

        /// Create a new child span associated with the current local span in the current thread, and then
        /// it will become the new local parent.
        fn ftr_create_loc_span_enter(name: &'static str) -> ftr_loc_span;

        fn ftr_destroy_loc_span(span: ftr_loc_span);

        /// Collect local spans manually without a parent, see `ftr_push_child_spans_to_cur` to learn more.
        fn ftr_start_loc_coll() -> ftr_loc_coll;

        /// Collect local spans into a LocalSpans instance, see `ftr_push_child_spans_to_cur` to learn more.
        fn ftr_collect_loc_spans(lc: ftr_loc_coll) -> ftr_loc_spans;

        /// Create default configuration of the behavior of the global collector.
        fn ftr_create_def_coll_cfg() -> ftr_coll_cfg;

        /// A soft limit for the total number of spans and events for a trace, usually used
        /// to avoid out-of-memory.
        ///
        /// # Note
        ///
        /// Root span will always be collected. The eventually collected spans may exceed the limit.
        fn ftr_set_max_spans_per_trace(cfg: ftr_coll_cfg, mspt: usize) -> ftr_coll_cfg;

        /// The time duration between two batch reports.
        ///
        /// The default value is 500 milliseconds.
        ///
        /// A batch report will be initiated by the earliest of these events:
        ///
        /// - When the specified time duration between two batch reports is met.
        /// - When the number of spans in a batch hits its limit.
        fn ftr_set_report_interval(cfg: ftr_coll_cfg, ri: u64) -> ftr_coll_cfg;

        /// Sets console reporter for the current application, usually used for debugging.
        fn ftr_set_cons_rptr();

        fn ftr_create_def_otlp_exp_cfg() -> ftr_otlp_exp_cfg;

        /// Create an `ftr_otel_rptr` to export trace records to remote agents that OpenTelemetry
        /// supports, which includes Jaeger, Datadog, Zipkin, and OpenTelemetry Collector.
        fn ftr_create_otel_rptr(cfg: ftr_otlp_exp_cfg) -> ftr_otel_rptr;

        fn ftr_destroy_otel_rptr(rptr: ftr_otel_rptr);

        /// Sets the reporter and its configuration for the current application.
        fn ftr_set_otel_rptr(rptr: ftr_otel_rptr, cfg: ftr_coll_cfg);

        /// Flushes all pending span records to the reporter immediately.
        ///
        /// # Note
        ///
        /// It will create a new thread in the current thread to do this,
        /// and it will block the current thread until the new thread finishes flushing and exits.
        fn ftr_flush();
    }
}

pub fn ftr_create_rand_span_ctx() -> ftr_span_ctx {
    unsafe { transmute(SpanContext::random()) }
}

pub fn ftr_create_span_ctx(span: &ftr_span) -> ftr_span_ctx {
    unsafe { transmute(SpanContext::from_span(transmute(span)).unwrap()) }
}

pub fn ftr_create_span_ctx_loc() -> ftr_span_ctx {
    unsafe { transmute(SpanContext::current_local_parent().unwrap()) }
}

pub fn ftr_span_ctx_set_sampled(ctx: ftr_span_ctx, sampled: bool) -> ftr_span_ctx {
    unsafe { transmute(transmute::<ftr_span_ctx, SpanContext>(ctx).sampled(sampled)) }
}

pub fn ftr_create_root_span(name: &'static str, parent: ftr_span_ctx) -> ftr_span {
    unsafe { transmute(Span::root(name, transmute(parent))) }
}

pub fn ftr_create_child_span_enter(name: &'static str, parent: &ftr_span) -> ftr_span {
    unsafe { transmute(Span::enter_with_parent(name, transmute(parent))) }
}

pub fn ftr_create_child_span_enter_loc(name: &'static str) -> ftr_span {
    unsafe { transmute(Span::enter_with_local_parent(name)) }
}

pub fn ftr_cancel_span(span: ftr_span) {
    unsafe { transmute::<ftr_span, Span>(span).cancel() }
}

pub fn ftr_destroy_span(span: ftr_span) {
    unsafe { drop(transmute::<ftr_span, Span>(span)) }
}

pub fn ftr_set_loc_par_to_span(span: &ftr_span) -> ftr_loc_par_guar {
    unsafe { transmute(transmute::<&ftr_span, &Span>(span).set_local_parent()) }
}

pub fn ftr_destroy_loc_par_guar(guard: ftr_loc_par_guar) {
    unsafe { drop(transmute::<ftr_loc_par_guar, LocalParentGuard>(guard)) }
}

pub fn ftr_push_child_spans_to_cur(span: &ftr_span, local_span: ftr_loc_spans) {
    unsafe { transmute::<&ftr_span, &Span>(span).push_child_spans(transmute(local_span)) }
}

pub fn ftr_create_loc_span_enter(name: &'static str) -> ftr_loc_span {
    unsafe { transmute(LocalSpan::enter_with_local_parent(name)) }
}

pub fn ftr_destroy_loc_span(span: ftr_loc_span) {
    unsafe { drop(transmute::<ftr_loc_span, LocalSpan>(span)) }
}

pub fn ftr_start_loc_coll() -> ftr_loc_coll {
    unsafe { transmute(LocalCollector::start()) }
}

pub fn ftr_collect_loc_spans(lc: ftr_loc_coll) -> ftr_loc_spans {
    unsafe { transmute(transmute::<ftr_loc_coll, LocalCollector>(lc).collect()) }
}

pub fn ftr_create_def_coll_cfg() -> ftr_coll_cfg {
    unsafe { transmute(Config::default()) }
}

pub fn ftr_set_max_spans_per_trace(cfg: ftr_coll_cfg, mspt: usize) -> ftr_coll_cfg {
    unsafe { transmute(transmute::<ftr_coll_cfg, Config>(cfg).max_spans_per_trace(Some(mspt))) }
}

pub fn ftr_set_report_interval(cfg: ftr_coll_cfg, ri: u64) -> ftr_coll_cfg {
    unsafe {
        transmute(transmute::<ftr_coll_cfg, Config>(cfg).report_interval(Duration::from_millis(ri)))
    }
}

pub fn ftr_set_cons_rptr() {
    fastrace::set_reporter(ConsoleReporter, Config::default())
}

pub fn ftr_create_def_otlp_exp_cfg() -> ftr_otlp_exp_cfg {
    unsafe {
        transmute(opentelemetry_otlp::ExportConfig {
            endpoint: std::env::var("OTEL_EXPORTER_OTLP_ENDPOINT")
                .unwrap_or("http://127.0.0.1:4317".to_string()),
            protocol: opentelemetry_otlp::Protocol::Grpc,
            timeout: std::time::Duration::from_secs(
                opentelemetry_otlp::OTEL_EXPORTER_OTLP_TIMEOUT_DEFAULT,
            ),
        })
    }
}

pub fn ftr_create_otel_rptr(cfg: ftr_otlp_exp_cfg) -> ftr_otel_rptr {
    initialize_runtime();

    let reporter = RUNTIME.lock().unwrap().block_on(async {
        fastrace_opentelemetry::OpenTelemetryReporter::new(
            opentelemetry_otlp::new_exporter()
                .tonic()
                .with_export_config(unsafe { transmute(cfg) })
                .build_span_exporter()
                .expect("initialize oltp exporter"),
            opentelemetry::trace::SpanKind::Server,
            Cow::Owned(opentelemetry_sdk::Resource::new([
                opentelemetry::KeyValue::new("service.name", std::env::var("SERVICE_NAME").unwrap_or("unknown".to_string())),
            ])),
            opentelemetry::InstrumentationLibrary::builder("fastrace-c")
                .with_version(env!("CARGO_PKG_VERSION"))
                .build(),
        )
    });

    unsafe { transmute(reporter) }
}

pub fn ftr_destroy_otel_rptr(rptr: ftr_otel_rptr) {
    unsafe {
        drop(transmute::<ftr_otel_rptr, OpenTelemetryReporter>(rptr));
    }
}

pub fn ftr_set_otel_rptr(rptr: ftr_otel_rptr, cfg: ftr_coll_cfg) {
    unsafe {
        fastrace::set_reporter(
            transmute::<ftr_otel_rptr, OpenTelemetryReporter>(rptr),
            transmute(cfg),
        )
    }
}

pub fn ftr_flush() {
    fastrace::flush()
}
