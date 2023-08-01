// Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0.

use minitrace::prelude::Span;
use std::ffi::CStr;
use std::ptr::NonNull;

use crate::{collector::mtr_span_ctx, local_collector::mtr_loc_spans};

pub struct mtr_span {
    pub(crate) inner: Span,
}

ffi_fn! {
    /// Create a new trace and return its root span.
    ///
    /// Once dropped, the root span automatically submits all associated child spans to the reporter.
    fn mtr_create_root_span(name: *const libc::c_char, parent: *mut mtr_span_ctx) -> *mut mtr_span {
        let name = unsafe { CStr::from_ptr(name) }.to_str().unwrap();
        Box::into_raw(Box::new(mtr_span {inner: Span::root(name, unsafe { (*parent).inner })}))
    } ?= std::ptr::null_mut()
}

ffi_fn! {
    /// Create a new child span associated with the specified parent span.
    fn mtr_create_child_span_enter(name: *const libc::c_char, parent: *mut mtr_span) -> *mut mtr_span {
        let name = unsafe { CStr::from_ptr(name) }.to_str().unwrap();
        Box::into_raw(Box::new(mtr_span {inner: Span::enter_with_parent(name, unsafe { &(*parent).inner })}))
    } ?= std::ptr::null_mut()
}

ffi_fn! {
    /// Create a new child span associated with the current local span in the current thread.
    ///
    /// If no local span is active, this function returns a no-op span.
    fn mtr_create_child_span_enter_loc(name: *const libc::c_char) -> *mut mtr_span {
        let name = unsafe { CStr::from_ptr(name) }.to_str().unwrap();
        Box::into_raw(Box::new(mtr_span {inner: Span::enter_with_local_parent(name)}))
    } ?= std::ptr::null_mut()
}

pub struct mtr_guard {
    #[allow(dyn_drop)]
    inner: NonNull<dyn Drop>,
}

ffi_fn! {
    /// Sets the current `Span` as the local parent for the current thread.
    ///
    /// This method is used to establish a `Span` as the local parent within the current scope.
    ///
    /// A local parent is necessary for creating a [`LocalSpan`] using [`LocalSpan::enter_with_local_parent()`].
    /// If no local parent is set, `enter_with_local_parent()` will not perform any action.
    fn mtr_set_loc_parent_to_span(s: *mut mtr_span) -> *mut mtr_guard {
        let s = Span::set_local_parent(unsafe { &(*s).inner }).unwrap();
        Box::into_raw(Box::new(mtr_guard {inner: NonNull::new(Box::into_raw(Box::new(s))).unwrap()}))
    }
}

ffi_fn! {
    /// Free an unused `mtr_guard *`.
    #[allow(dyn_drop)]
    fn mtr_free_guard(g: *mut mtr_guard) {
        let inner: Box<dyn Drop> = unsafe { Box::from_raw((*g).inner.as_ptr() as *mut _) };
        drop(inner);
        drop(non_null!(Box::from_raw(g) ?= ()));
    }
}

ffi_fn! {
    /// Add a single property to the `Span` and return the modified `Span`.
    ///
    /// A property is an arbitrary key-value pair associated with a span.
    fn mtr_add_prop_to_span(s: *mut mtr_span, k: *const libc::c_char, v: *const libc::c_char) -> libc::c_int {
        let k = unsafe { CStr::from_ptr(k) };
        let v = unsafe { CStr::from_ptr(v) };
        let mut tmp = unsafe { std::ptr::read(s) };
        tmp.inner = tmp.inner.with_property(|| (k.to_string_lossy(), v.to_string_lossy()));
        unsafe { std::ptr::write(s, tmp) };
        return 0;
    }
}

ffi_fn! {
    /// Add multiple properties to the `Span` and return the modified `Span`.
    fn mtr_add_props_to_span(s: *mut mtr_span, karr: *const *const libc::c_char, varr: *const *const libc::c_char, len: usize) -> libc::c_int {
        let mut tmp = unsafe { std::ptr::read(s) };
        tmp.inner = tmp.inner.with_properties(|| unsafe {
            (0..len).map(|i| {
                let k = CStr::from_ptr(*karr.offset(i as isize));
                let v = CStr::from_ptr(*varr.offset(i as isize));
                (
                    String::from_utf8_lossy(k.to_bytes()).into_owned().into(),
                    String::from_utf8_lossy(v.to_bytes()).into_owned().into(),
                )
            }).collect::<Vec<_>>()
        });
        unsafe { std::ptr::write(s, tmp) };
        return 0;
    } ?= -1
}

ffi_fn! {
    /// Attach a collection of [`LocalSpan`] instances as child spans to the current span.
    ///
    /// This method allows you to associate previously collected `LocalSpan` instances with the current span.
    /// This is particularly useful when the `LocalSpan` instances were initiated before their parent span,
    /// and were collected manually using a [`LocalCollector`].
    fn mtr_push_child_spans_to_cur(s: *mut mtr_span, local_spans: *mut mtr_loc_spans) {
        let inner = unsafe { &(*s).inner };
        inner.push_child_spans(unsafe { std::ptr::read(local_spans) }.inner)
    }
}

ffi_fn! {
    /// Free an unused `mtr_span *`.
    fn mtr_free_span(s: *mut mtr_span) {
        drop(non_null!(Box::from_raw(s) ?= ()));
    }
}
