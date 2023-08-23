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

## Run example

```bash
# start an OTEL Exporter, eg Jaeger
docker run -d --name jaeger -e COLLECTOR_OTLP_ENABLED=true -e SPAN_STORAGE_TYPE=badger  -e BADGER_EPHEMERAL=false -e BADGER_DIRECTORY_VALUE=/badger/data -e BADGER_DIRECTORY_KEY=/badger/key   -v /mnt/badger:/badger  -p 16686:16686 -p 4317:4317 -p 4318:4318 jaegertracing/all-in-one:1.46

cd examples && cmake -S . -B build && cmake --build build
# run example
SERVICE_NAME=asynchronous ./build/asynchronous      # capi
SERVICE_NAME=asynchronous2 ./build/asynchronous2    # c++ api
```
