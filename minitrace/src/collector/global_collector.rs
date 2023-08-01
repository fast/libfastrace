// Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0.

use crate::collector::console_reporter::mtr_cons_rptr;
use crate::collector::mtr_coll_cfg;

ffi_fn! {
    fn mtr_set_rptr(rptr: *mut mtr_cons_rptr, cfg: *mut mtr_coll_cfg) {
        minitrace::set_reporter(unsafe { std::ptr::read(rptr) }.inner, unsafe { (*cfg).inner })
    }
}

ffi_fn! {
    fn mtr_flush() {
        minitrace::flush();
    }
}
