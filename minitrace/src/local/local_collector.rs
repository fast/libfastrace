// Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0.

use minitrace::local::LocalCollector;
use minitrace::local::LocalSpans;

pub struct mtr_loc_spans {
    pub(crate) inner: LocalSpans,
}

pub struct mtr_loc_coll {
    inner: LocalCollector,
}

ffi_fn! {
    fn mtr_start_loc_coll() -> *mut mtr_loc_coll {
        Box::into_raw(Box::new(mtr_loc_coll { inner: LocalCollector::start() }))
    }
}

ffi_fn! {
    fn mtr_collect_loc_spans(lc: *mut mtr_loc_coll) -> *mut mtr_loc_spans {
        let lc = unsafe { std::ptr::read(lc) };
        Box::into_raw(Box::new(mtr_loc_spans { inner: lc.inner.collect() }))
    }
}
