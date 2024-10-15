// Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0.

fn main() {
    let _build = cxx_build::bridge("src/lib.rs").file("src/libfastrace.cc");

    println!("cargo:rerun-if-changed=src/lib.rs");
    println!("cargo:rerun-if-changed=src/libfastrace.cc");
}
