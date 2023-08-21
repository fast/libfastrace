// Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0.

use minitrace::{
    collector::{Config, ConsoleReporter},
    local::LocalCollector,
    prelude::{LocalSpan, Span},
};
use std::{mem::transmute, time::Duration};

use self::ffi::*;

#[cxx::bridge]
mod ffi {
    struct mtr_span_ctx {
        _padding: [u8; 24],
    }

    struct mtr_span {
        _padding: [u8; 128],
    }

    struct mtr_loc_par_guar {
        _padding: [u8; 24],
    }

    struct mtr_loc_span {
        _padding: [u8; 24],
    }

    struct mtr_loc_spans {
        _padding: [u8; 8],
    }

    struct mtr_loc_coll {
        _padding: [u8; 16],
    }

    struct mtr_coll_cfg {
        _padding: [u8; 48],
    }

    extern "Rust" {
        fn mtr_create_root_span(name: &'static str, parent: mtr_span_ctx) -> mtr_span;

        fn mtr_create_child_span_enter(name: &'static str, parent: &mtr_span) -> mtr_span;

        fn mtr_create_child_span_enter_loc(name: &'static str) -> mtr_span;

        fn mtr_set_loc_par_to_span(span: &mtr_span) -> mtr_loc_par_guar;

        fn mtr_push_child_spans_to_cur(span: &mtr_span, local_span: mtr_loc_spans);

        fn mtr_create_loc_span_enter(name: &'static str) -> mtr_loc_span;

        fn mtr_start_loc_coll() -> mtr_loc_coll;

        fn mtr_collect_loc_spans(lc: mtr_loc_coll) -> mtr_loc_spans;

        fn mtr_set_max_spans_per_trace(cfg: mtr_coll_cfg, mspt: usize) -> mtr_coll_cfg;

        fn mtr_set_batch_report_interval(cfg: mtr_coll_cfg, bri: u64) -> mtr_coll_cfg;

        fn mtr_set_report_max_spans(cfg: mtr_coll_cfg, brms: usize) -> mtr_coll_cfg;

        fn mtr_set_cons_rptr();

        fn mtr_flush();
    }
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

fn mtr_set_loc_par_to_span(span: &mtr_span) -> mtr_loc_par_guar {
    unsafe { transmute(transmute::<&mtr_span, &Span>(span).set_local_parent()) }
}

fn mtr_push_child_spans_to_cur(span: &mtr_span, local_span: mtr_loc_spans) {
    unsafe { transmute::<&mtr_span, &Span>(span).push_child_spans(transmute(local_span)) }
}

fn mtr_create_loc_span_enter(name: &'static str) -> mtr_loc_span {
    unsafe { transmute(LocalSpan::enter_with_local_parent(name)) }
}

fn mtr_start_loc_coll() -> mtr_loc_coll {
    unsafe { transmute(LocalCollector::start()) }
}

fn mtr_collect_loc_spans(lc: mtr_loc_coll) -> mtr_loc_spans {
    unsafe { transmute(transmute::<mtr_loc_coll, LocalCollector>(lc).collect()) }
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

fn mtr_flush() {
    minitrace::flush()
}
