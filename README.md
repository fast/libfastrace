# minitrace for C/C++

minitrace for C/C++.

It's built on top of
[minitrace in Rust](https://github.com/tikv/minitrace-rust) via [cxx](https://github.com/dtolnay/cxx).

This lib is still in the stage of prove-of-concept and under heavy development.

## Prepare 

```bash
# install rust environment
curl https://sh.rustup.rs -sSf | sh
```

## Build

```bash
## compile in build directory
cmake -S . -B build && cmake --build build
## install to /usr/local
sudo cmake --install build
```
