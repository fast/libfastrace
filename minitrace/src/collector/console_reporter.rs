// Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0.

use minitrace::collector::ConsoleReporter;

pub struct mtr_cons_rptr {
    pub(crate) inner: ConsoleReporter,
}

ffi_fn! {
    fn mtr_create_cons_rptr() -> *mut mtr_cons_rptr {
        Box::into_raw(Box::new(mtr_cons_rptr {inner: ConsoleReporter {}}))
    }
}

ffi_fn! {
    fn mtr_free_cons_rptr(r: *mut mtr_cons_rptr) {
        drop(non_null!(Box::from_raw(r) ?= ()));
    }
}
