// Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0.

use minitrace::prelude::LocalSpan;
use std::ffi::CStr;

pub struct mtr_loc_span {
    inner: LocalSpan,
}

ffi_fn! {
    /// Create a new child span associated with the current local span in the current thread, and then
    /// it will become the new local parent.
    ///
    /// If no local span is active, this function is no-op.
    fn mtr_create_loc_span_enter(name: *const libc::c_char) -> *mut mtr_loc_span {
        let name = unsafe { CStr::from_ptr(name) };
        let name = name.to_str().expect("Invalid UTF-8 string");
        Box::into_raw(Box::new(mtr_loc_span {inner: LocalSpan::enter_with_local_parent(name)}))
    } ?= std::ptr::null_mut()
}

ffi_fn! {
    /// Add a single property to the `LocalSpan` and return the modified `LocalSpan`.
    ///
    /// A property is an arbitrary key-value pair associated with a span.
    fn mtr_add_prop_to_loc_span(ls: *mut mtr_loc_span, k: *const libc::c_char, v: *const libc::c_char) -> libc::c_int {
        let k = unsafe { CStr::from_ptr(k) };
        let v = unsafe { CStr::from_ptr(v) };
        let mut tmp = unsafe { std::ptr::read(ls) };
        tmp.inner = tmp.inner.with_property(|| (k.to_string_lossy(), v.to_string_lossy()));
        unsafe { std::ptr::write(ls, tmp) };
        return 0;
    } ?= -1
}

ffi_fn! {
    /// Add multiple properties to the `LocalSpan` and return the modified `LocalSpan`.
    fn mtr_add_props_to_loc_span(ls: *mut mtr_loc_span, karr: *const *const libc::c_char, varr: *const *const libc::c_char, len: usize) -> libc::c_int {
        let mut tmp = unsafe { std::ptr::read(ls) };
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
        unsafe { std::ptr::write(ls, tmp) };
        return 0;
    } ?= -1
}

ffi_fn! {
    /// Free an unused `mtr_local_span *`.
    fn mtr_free_loc_span(ls: *mut mtr_loc_span) {
        drop(non_null!(Box::from_raw(ls) ?= ()));
    }
}
