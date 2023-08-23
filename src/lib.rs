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
        fn mtr_create_rand_span_ctx() -> mtr_span_ctx;

        fn mtr_create_root_span(name: &'static str, parent: mtr_span_ctx) -> mtr_span;

        fn mtr_create_child_span_enter(name: &'static str, parent: &mtr_span) -> mtr_span;

        fn mtr_create_child_span_enter_loc(name: &'static str) -> mtr_span;

        fn mtr_destroy_span(span: mtr_span);

        fn mtr_set_loc_par_to_span(span: &mtr_span) -> mtr_loc_par_guar;

        fn mtr_destroy_loc_par_guar(guard: mtr_loc_par_guar);

        fn mtr_push_child_spans_to_cur(span: &mtr_span, local_span: mtr_loc_spans);

        fn mtr_create_loc_span_enter(name: &'static str) -> mtr_loc_span;

        fn mtr_destroy_loc_span(span: mtr_loc_span);

        fn mtr_start_loc_coll() -> mtr_loc_coll;

        fn mtr_collect_loc_spans(lc: mtr_loc_coll) -> mtr_loc_spans;

        fn mtr_create_def_coll_cfg() -> mtr_coll_cfg;

        fn mtr_set_max_spans_per_trace(cfg: mtr_coll_cfg, mspt: usize) -> mtr_coll_cfg;

        fn mtr_set_batch_report_interval(cfg: mtr_coll_cfg, bri: u64) -> mtr_coll_cfg;

        fn mtr_set_report_max_spans(cfg: mtr_coll_cfg, brms: usize) -> mtr_coll_cfg;

        fn mtr_set_cons_rptr();

        fn mtr_create_def_otlp_exp_cfg() -> mtr_otlp_exp_cfg;

        fn mtr_create_def_otlp_grpcio_cfg() -> mtr_otlp_grpcio_cfg;

        fn mtr_create_otel_rptr(cfg: mtr_otlp_exp_cfg, gcfg: mtr_otlp_grpcio_cfg) -> mtr_otel_rptr;

        fn mtr_destroy_otel_rptr(rptr: mtr_otel_rptr);

        fn mtr_set_otel_rptr(rptr: mtr_otel_rptr, cfg: mtr_coll_cfg);

        fn mtr_flush();
    }
}

fn mtr_create_rand_span_ctx() -> mtr_span_ctx {
    unsafe { transmute(SpanContext::random()) }
}

fn mtr_create_root_span(name: &'static str, parent: mtr_span_ctx) -> mtr_span {
    unsafe { transmute(Span::root(name, transmute(parent))) }
}

fn mtr_create_child_span_enter(name: &'static str, parent: &mtr_span) -> mtr_span {
    unsafe { transmute(Span::enter_with_parent(name, transmute(parent))) }
}

fn mtr_create_child_span_enter_loc(name: &'static str) -> mtr_span {
    unsafe { transmute(Span::enter_with_local_parent(name)) }
}

fn mtr_destroy_span(span: mtr_span) {
    unsafe { drop(transmute::<mtr_span, Span>(span)) }
}

fn mtr_set_loc_par_to_span(span: &mtr_span) -> mtr_loc_par_guar {
    unsafe { transmute(transmute::<&mtr_span, &Span>(span).set_local_parent()) }
}

fn mtr_destroy_loc_par_guar(guard: mtr_loc_par_guar) {
    unsafe { drop(transmute::<mtr_loc_par_guar, LocalParentGuard>(guard)) }
}

fn mtr_push_child_spans_to_cur(span: &mtr_span, local_span: mtr_loc_spans) {
    unsafe { transmute::<&mtr_span, &Span>(span).push_child_spans(transmute(local_span)) }
}

fn mtr_create_loc_span_enter(name: &'static str) -> mtr_loc_span {
    unsafe { transmute(LocalSpan::enter_with_local_parent(name)) }
}

fn mtr_destroy_loc_span(span: mtr_loc_span) {
    unsafe { drop(transmute::<mtr_loc_span, LocalSpan>(span)) }
}

fn mtr_start_loc_coll() -> mtr_loc_coll {
    unsafe { transmute(LocalCollector::start()) }
}

fn mtr_collect_loc_spans(lc: mtr_loc_coll) -> mtr_loc_spans {
    unsafe { transmute(transmute::<mtr_loc_coll, LocalCollector>(lc).collect()) }
}

fn mtr_create_def_coll_cfg() -> mtr_coll_cfg {
    unsafe { transmute(Config::default()) }
}

fn mtr_set_max_spans_per_trace(cfg: mtr_coll_cfg, mspt: usize) -> mtr_coll_cfg {
    unsafe { transmute(transmute::<mtr_coll_cfg, Config>(cfg).max_spans_per_trace(Some(mspt))) }
}

fn mtr_set_batch_report_interval(cfg: mtr_coll_cfg, bri: u64) -> mtr_coll_cfg {
    unsafe {
        transmute(
            transmute::<mtr_coll_cfg, Config>(cfg)
                .batch_report_interval(Duration::from_millis(bri)),
        )
    }
}

fn mtr_set_report_max_spans(cfg: mtr_coll_cfg, brms: usize) -> mtr_coll_cfg {
    unsafe { transmute(transmute::<mtr_coll_cfg, Config>(cfg).batch_report_max_spans(Some(brms))) }
}

fn mtr_set_cons_rptr() {
    minitrace::set_reporter(ConsoleReporter, Config::default())
}

fn mtr_create_def_otlp_exp_cfg() -> mtr_otlp_exp_cfg {
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

fn mtr_create_def_otlp_grpcio_cfg() -> mtr_otlp_grpcio_cfg {
    unsafe { transmute(opentelemetry_otlp::GrpcioConfig::default()) }
}

fn mtr_create_otel_rptr(cfg: mtr_otlp_exp_cfg, gcfg: mtr_otlp_grpcio_cfg) -> mtr_otel_rptr {
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

fn mtr_destroy_otel_rptr(rptr: mtr_otel_rptr) {
    unsafe { drop(transmute::<mtr_otel_rptr, OpenTelemetryReporter>(rptr)) }
}

fn mtr_set_otel_rptr(rptr: mtr_otel_rptr, cfg: mtr_coll_cfg) {
    unsafe {
        minitrace::set_reporter(
            transmute::<mtr_otel_rptr, OpenTelemetryReporter>(rptr),
            transmute(cfg),
        )
    }
}

fn mtr_flush() {
    minitrace::flush()
}
