// Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0.

use crate::collector::console_reporter::mtr_cons_rptr;
use crate::collector::mtr_coll_cfg;
use crate::collector::opentelemetry_reporter::mtr_otel_rptr;

ffi_fn! {
    fn mtr_set_cons_rptr(rptr: *mut mtr_cons_rptr, cfg: *mut mtr_coll_cfg) {
        minitrace::set_reporter(unsafe { std::ptr::read(rptr) }.inner, unsafe { (*cfg).inner })
    }
}

ffi_fn! {
    fn mtr_set_otel_rptr(rptr: *mut mtr_otel_rptr, cfg: *mut mtr_coll_cfg) {
        minitrace::set_reporter(unsafe { std::ptr::read(rptr) }.inner, unsafe { (*cfg).inner })
    }
}

ffi_fn! {
    fn mtr_flush() {
        minitrace::flush();
    }
}
