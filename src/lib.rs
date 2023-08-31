// Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0.

use minitrace::{
    collector::{Config, ConsoleReporter},
    local::{LocalCollector, LocalParentGuard},
    prelude::{LocalSpan, Span, SpanContext},
};
use minitrace_opentelemetry::OpenTelemetryReporter;
use std::{mem::transmute, time::Duration};

use self::ffi::*;

#[cxx::bridge]
mod ffi {

    #[namespace = "ffi"]
    struct mtr_span_ctx {
        _padding: [u64; 3],
    }

    #[namespace = "ffi"]
    struct mtr_span {
        _padding: [u64; 16],
    }

    #[namespace = "ffi"]
    struct mtr_loc_par_guar {
        _padding: [u64; 3],
    }

    #[namespace = "ffi"]
    struct mtr_loc_span {
        _padding: [u64; 3],
    }

    #[namespace = "ffi"]
    struct mtr_loc_spans {
        _padding: [u64; 1],
    }

    #[namespace = "ffi"]
    struct mtr_loc_coll {
        _padding: [u64; 2],
    }

    #[namespace = "ffi"]
    struct mtr_coll_cfg {
        _padding: [u64; 6],
    }

    #[namespace = "ffi"]
    struct mtr_otel_rptr {
        _padding: [u64; 24],
    }

    #[namespace = "ffi"]
    struct mtr_otlp_exp_cfg {
        _padding: [u64; 6],
    }

    #[namespace = "ffi"]
    struct mtr_otlp_grpcio_cfg {
        _padding: [u64; 14],
    }

    #[namespace = "minitrace_glue"]
    extern "Rust" {

        /// Create a new `mtr_span_context` with a random trace id.
        fn mtr_create_rand_span_ctx() -> mtr_span_ctx;

        /// Creates a `mtr_span_context` from the given [`mtr_span`].
        fn mtr_create_span_ctx(span: &mtr_span) -> mtr_span_ctx;

        ///Creates a `mtr_span_context` from the current local parent span.
        fn mtr_create_span_ctx_loc() -> mtr_span_ctx;

        /// Create a new trace and return its root span.
        ///
        /// Once destroyed (dropped), the root span automatically submits all associated child spans to the reporter.
        fn mtr_create_root_span(name: &'static str, parent: mtr_span_ctx) -> mtr_span;

        /// Create a new child span associated with the specified parent span.
        fn mtr_create_child_span_enter(name: &'static str, parent: &mtr_span) -> mtr_span;

        /// Create a new child span associated with the current local span in the current thread.
        fn mtr_create_child_span_enter_loc(name: &'static str) -> mtr_span;

        /// Dismisses the trace, preventing the reporting of any span records associated with it.
        ///
        /// This is particularly useful when focusing on the tail latency of a program. For instant,
        /// you can dismiss all traces finishes within the 99th percentile.
        ///
        /// # Note
        ///
        /// This method only dismisses the entire trace when called on the root span.
        /// If called on a non-root span, it will only cancel the reporting of that specific span.
        fn mtr_cancel_span(span: mtr_span);

        /// Once destroyed (dropped), the root span automatically submits all associated child spans to the reporter.
        fn mtr_destroy_span(span: mtr_span);

        /// Sets the current `mtr_pan` as the local parent for the current thread.
        ///
        /// This method is used to establish a `span` as the local parent within the current scope.
        ///
        /// A local parent is necessary for creating a [`mtr_local_span`] using [`mtr_create_loc_span_enter()`].
        /// If no local parent is set, `mtr_create_loc_span_enter()` will not perform any action.
        fn mtr_set_loc_par_to_span(span: &mtr_span) -> mtr_loc_par_guar;

        fn mtr_destroy_loc_par_guar(guard: mtr_loc_par_guar);

        /// Attach a collection of [`mtr_local_span`] instances as child spans to the current span.
        ///
        /// This method allows you to associate previously collected `mtr_local_span` instances with the current span.
        /// This is particularly useful when the `mtr_local_span` instances were initiated before their parent span,
        /// and were collected manually using a [`mtr_loc_coll`].
        ///
        /// # Examples:
        ///
        /// ````
        /// // Collect local spans manually without a parent
        /// let coll = mtr_start_loc_coll();
        /// let ls = mtr_create_loc_span_enter();
        /// mtr_destroy_loc_span(ls);
        ///
        /// // Collect local spans manually without a parent
        /// let lscoll = mtr_collect_loc_spans(coll);
        ///
        /// // Attach the local spans to a parent
        /// let root = mtr_create_root_span("root", mtr_create_rand_span_ctx());
        /// mtr_push_child_spans_to_cur(&root, lscoll);
        /// ````
        fn mtr_push_child_spans_to_cur(span: &mtr_span, local_span: mtr_loc_spans);

        /// Create a new child span associated with the current local span in the current thread, and then
        /// it will become the new local parent.
        fn mtr_create_loc_span_enter(name: &'static str) -> mtr_loc_span;

        fn mtr_destroy_loc_span(span: mtr_loc_span);

        /// Collect local spans manually without a parent, see `mtr_push_child_spans_to_cur` to learn more.
        fn mtr_start_loc_coll() -> mtr_loc_coll;

        /// Collect local spans into a LocalSpans instance, see `mtr_push_child_spans_to_cur` to learn more.
        fn mtr_collect_loc_spans(lc: mtr_loc_coll) -> mtr_loc_spans;

        /// Create default configuration of the behavior of the global collector.
        fn mtr_create_def_coll_cfg() -> mtr_coll_cfg;

        /// A soft limit for the total number of spans and events for a trace, usually used
        /// to avoid out-of-memory.
        ///
        /// # Note
        ///
        /// Root span will always be collected. The eventually collected spans may exceed the limit.
        fn mtr_set_max_spans_per_trace(cfg: mtr_coll_cfg, mspt: usize) -> mtr_coll_cfg;

        /// The time duration between two batch reports.
        ///
        /// The default value is 500 milliseconds.
        ///
        /// A batch report will be initiated by the earliest of these events:
        ///
        /// - When the specified time duration between two batch reports is met.
        /// - When the number of spans in a batch hits its limit.
        fn mtr_set_batch_report_interval(cfg: mtr_coll_cfg, bri: u64) -> mtr_coll_cfg;

        /// The soft limit for the maximum number of spans in a batch report.
        ///
        /// A batch report will be initiated by the earliest of these events:
        ///
        /// - When the specified time duration between two batch reports is met.
        /// - When the number of spans in a batch hits its limit.
        ///
        /// # Note
        ///
        /// The eventually spans being reported may exceed the limit.
        fn mtr_set_report_max_spans(cfg: mtr_coll_cfg, brms: usize) -> mtr_coll_cfg;

        /// Sets console reporter for the current application, usually used for debugging.
        fn mtr_set_cons_rptr();

        fn mtr_create_def_otlp_exp_cfg() -> mtr_otlp_exp_cfg;

        fn mtr_create_def_otlp_grpcio_cfg() -> mtr_otlp_grpcio_cfg;

        /// Create an `mtr_otel_rptr` to export trace records to remote agents that OpenTelemetry
        /// supports, which includes Jaeger, Datadog, Zipkin, and OpenTelemetry Collector.
        fn mtr_create_otel_rptr(cfg: mtr_otlp_exp_cfg, gcfg: mtr_otlp_grpcio_cfg) -> mtr_otel_rptr;

        fn mtr_destroy_otel_rptr(rptr: mtr_otel_rptr);

        /// Sets the reporter and its configuration for the current application.
        fn mtr_set_otel_rptr(rptr: mtr_otel_rptr, cfg: mtr_coll_cfg);

        /// Flushes all pending span records to the reporter immediately.
        ///
        /// # Note
        ///
        /// It will create a new thread in the current thread to do this,
        /// and it will block the current thread until the new thread finishes flushing and exits.
        fn mtr_flush();
    }
}

pub fn mtr_create_rand_span_ctx() -> mtr_span_ctx {
    unsafe { transmute(SpanContext::random()) }
}

pub fn mtr_create_span_ctx(span: &mtr_span) -> mtr_span_ctx {
    unsafe { transmute(SpanContext::from_span(transmute(span)).unwrap()) }
}

pub fn mtr_create_span_ctx_loc() -> mtr_span_ctx {
    unsafe { transmute(SpanContext::current_local_parent().unwrap()) }
}

pub fn mtr_create_root_span(name: &'static str, parent: mtr_span_ctx) -> mtr_span {
    unsafe { transmute(Span::root(name, transmute(parent))) }
}

pub fn mtr_create_child_span_enter(name: &'static str, parent: &mtr_span) -> mtr_span {
    unsafe { transmute(Span::enter_with_parent(name, transmute(parent))) }
}

pub fn mtr_create_child_span_enter_loc(name: &'static str) -> mtr_span {
    unsafe { transmute(Span::enter_with_local_parent(name)) }
}

pub fn mtr_cancel_span(span: mtr_span) {
    unsafe { transmute::<mtr_span, Span>(span).cancel() }
}

pub fn mtr_destroy_span(span: mtr_span) {
    unsafe { drop(transmute::<mtr_span, Span>(span)) }
}

pub fn mtr_set_loc_par_to_span(span: &mtr_span) -> mtr_loc_par_guar {
    unsafe { transmute(transmute::<&mtr_span, &Span>(span).set_local_parent()) }
}

pub fn mtr_destroy_loc_par_guar(guard: mtr_loc_par_guar) {
    unsafe { drop(transmute::<mtr_loc_par_guar, LocalParentGuard>(guard)) }
}

pub fn mtr_push_child_spans_to_cur(span: &mtr_span, local_span: mtr_loc_spans) {
    unsafe { transmute::<&mtr_span, &Span>(span).push_child_spans(transmute(local_span)) }
}

pub fn mtr_create_loc_span_enter(name: &'static str) -> mtr_loc_span {
    unsafe { transmute(LocalSpan::enter_with_local_parent(name)) }
}

pub fn mtr_destroy_loc_span(span: mtr_loc_span) {
    unsafe { drop(transmute::<mtr_loc_span, LocalSpan>(span)) }
}

pub fn mtr_start_loc_coll() -> mtr_loc_coll {
    unsafe { transmute(LocalCollector::start()) }
}

pub fn mtr_collect_loc_spans(lc: mtr_loc_coll) -> mtr_loc_spans {
    unsafe { transmute(transmute::<mtr_loc_coll, LocalCollector>(lc).collect()) }
}

pub fn mtr_create_def_coll_cfg() -> mtr_coll_cfg {
    unsafe { transmute(Config::default()) }
}

pub fn mtr_set_max_spans_per_trace(cfg: mtr_coll_cfg, mspt: usize) -> mtr_coll_cfg {
    unsafe { transmute(transmute::<mtr_coll_cfg, Config>(cfg).max_spans_per_trace(Some(mspt))) }
}

pub fn mtr_set_batch_report_interval(cfg: mtr_coll_cfg, bri: u64) -> mtr_coll_cfg {
    unsafe {
        transmute(
            transmute::<mtr_coll_cfg, Config>(cfg)
                .batch_report_interval(Duration::from_millis(bri)),
        )
    }
}

pub fn mtr_set_report_max_spans(cfg: mtr_coll_cfg, brms: usize) -> mtr_coll_cfg {
    unsafe { transmute(transmute::<mtr_coll_cfg, Config>(cfg).batch_report_max_spans(Some(brms))) }
}

pub fn mtr_set_cons_rptr() {
    minitrace::set_reporter(ConsoleReporter, Config::default())
}

pub fn mtr_create_def_otlp_exp_cfg() -> mtr_otlp_exp_cfg {
    unsafe {
        transmute(opentelemetry_otlp::ExportConfig {
            endpoint: std::env::var("OTEL_EXPORTER_OTLP_ENDPOINT")
                .unwrap_or("localhost:4317".to_string()),
            protocol: opentelemetry_otlp::Protocol::Grpc,
            timeout: std::time::Duration::from_secs(
                opentelemetry_otlp::OTEL_EXPORTER_OTLP_TIMEOUT_DEFAULT,
            ),
        })
    }
}

pub fn mtr_create_def_otlp_grpcio_cfg() -> mtr_otlp_grpcio_cfg {
    unsafe { transmute(opentelemetry_otlp::GrpcioConfig::default()) }
}

pub fn mtr_create_otel_rptr(cfg: mtr_otlp_exp_cfg, gcfg: mtr_otlp_grpcio_cfg) -> mtr_otel_rptr {
    unsafe {
        transmute(minitrace_opentelemetry::OpenTelemetryReporter::new(
            opentelemetry_otlp::SpanExporter::new_grpcio(transmute(cfg), transmute(gcfg)),
            opentelemetry::trace::SpanKind::Server,
            std::borrow::Cow::Owned(opentelemetry::sdk::Resource::new([
                opentelemetry::KeyValue::new(
                    "service.name",
                    std::env::var("SERVICE_NAME").unwrap_or("unknown".to_string()),
                ),
            ])),
            opentelemetry::InstrumentationLibrary::new(
                "minitrace-opentelemetry-c",
                Some(env!("CARGO_PKG_VERSION")),
                None,
            ),
        ))
    }
}

pub fn mtr_destroy_otel_rptr(rptr: mtr_otel_rptr) {
    unsafe { drop(transmute::<mtr_otel_rptr, OpenTelemetryReporter>(rptr)) }
}

pub fn mtr_set_otel_rptr(rptr: mtr_otel_rptr, cfg: mtr_coll_cfg) {
    unsafe {
        minitrace::set_reporter(
            transmute::<mtr_otel_rptr, OpenTelemetryReporter>(rptr),
            transmute(cfg),
        )
    }
}

pub fn mtr_flush() {
    minitrace::flush()
}
