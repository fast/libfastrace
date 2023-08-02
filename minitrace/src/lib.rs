// Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0.

// We have a lot of c-types in here, stop warning about their names!
#![allow(non_camel_case_types)]
// fmt::Debug isn't helpful on FFI types
#![allow(missing_debug_implementations)]
// unreable_pub warns `#[no_mangle] pub extern fn` in private mod.
#![allow(unreachable_pub)]

//! # minitrace C API

#[macro_use]
mod macros;

pub mod collector;
mod local;
mod span;

pub use self::local::*;

/// cbindgen:ignore
static VERSION_CSTR: &str = concat!(env!("CARGO_PKG_VERSION"), "\0");

ffi_fn! {
    fn mtr_c_ver() -> *const libc::c_char {
        VERSION_CSTR.as_ptr() as _
    } ?= std::ptr::null()
}
