// Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0.

use crate::span::mtr_span;
use minitrace::{collector::Config, prelude::SpanContext};

pub(crate) mod console_reporter;
pub(crate) mod global_collector;

pub struct mtr_span_ctx {
    pub(crate) inner: SpanContext,
}

ffi_fn! {
    /// Create a new `SpanContext` with a random trace id.
    fn mtr_create_rand_span_ctx() -> *mut mtr_span_ctx {
        Box::into_raw(Box::new(mtr_span_ctx { inner: SpanContext::random() }))
    }
}

ffi_fn! {
    /// Creates a `SpanContext` from the given [`Span`]. If the `Span` is a noop span,
    /// this function will return `None`.
    fn mtr_create_span_ctx(span: *const mtr_span) -> *mut mtr_span_ctx {
        if span.is_null() {
            return std::ptr::null_mut();
        }
        match SpanContext::from_span(unsafe { &(*span).inner }) {
            Some(sc) => Box::into_raw(Box::new(mtr_span_ctx { inner: sc })),
            None => std::ptr::null_mut(),
        }
    }
}

ffi_fn! {
    /// Creates a `SpanContext` from the current local parent span. If there is no
    /// local parent span, this function will return `None`.
    fn mtr_create_span_ctx_loc() -> *mut mtr_span_ctx {
        match SpanContext::current_local_parent() {
            Some(sc) => Box::into_raw(Box::new(mtr_span_ctx {inner: sc})),
            None => std::ptr::null_mut()
        }
    }
}

ffi_fn! {
    /// Free an unused `mtr_span_ctx *`.
    fn mtr_free_span_ctx(c: *mut mtr_span_ctx) {
        drop(non_null!(Box::from_raw(c) ?= ()));
    }
}

pub struct mtr_coll_cfg {
    #[allow(dead_code)]
    pub(crate) inner: Config,
}

ffi_fn! {
    /// Default configuration of the behavior of the global collector.
    fn mtr_create_glob_coll_def_cfg() -> *mut mtr_coll_cfg {
        Box::into_raw(Box::new(mtr_coll_cfg { inner: Config::default() }))
    }
}

ffi_fn! {
    /// Free an unused `mtr_coll_cfg *`.
    fn mtr_free_glob_coll_def_cfg(c: *mut mtr_coll_cfg) {
        drop(non_null!(Box::from_raw(c) ?= ()));
    }
}
