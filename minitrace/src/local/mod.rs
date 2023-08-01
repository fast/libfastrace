// Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0.

//! Non thread-safe span with low overhead.

pub(crate) mod local_collector;
pub(crate) mod local_span;

pub use self::local_collector::mtr_loc_coll;
pub use self::local_collector::mtr_loc_spans;
pub use self::local_span::mtr_loc_span;
