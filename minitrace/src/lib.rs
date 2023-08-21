// Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0.

use minitrace::prelude::Span;
use std::mem::transmute;

use self::ffi::*;

#[cxx::bridge]
mod ffi {
    struct mtr_span_ctx {
        _padding: [u8; 24],
    }

    struct mtr_span {
        _padding: [u8; 128],
    }

    extern "Rust" {
        fn mtr_create_root_span(name: &'static str, parent: mtr_span_ctx) -> mtr_span;

        fn mtr_create_child_span_enter(name: &'static str, parent: &mtr_span) -> mtr_span;

        fn mtr_create_child_span_enter_loc(name: &'static str) -> mtr_span;
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
