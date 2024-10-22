# fastrace for C/C++

fastrace for C/C++.

It's built on top of
[fastrace in Rust](https://github.com/fast/fastrace) via [cxx](https://github.com/dtolnay/cxx).

## Requirements

- C++11 or later
- Rust environment

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

## Uninstall

To uninstall the library, use the following command:

```bash
sudo cmake --build build --target uninstall
```

## Run example

```bash
# start an OTEL Exporter, eg Jaeger
docker run -d --name jaeger -e COLLECTOR_OTLP_ENABLED=true -e SPAN_STORAGE_TYPE=badger  -e BADGER_EPHEMERAL=false -e BADGER_DIRECTORY_VALUE=/badger/data -e BADGER_DIRECTORY_KEY=/badger/key   -v /mnt/badger:/badger  -p 16686:16686 -p 4317:4317 -p 4318:4318 jaegertracing/all-in-one:1.46

cd examples && cmake -S . -B build && cmake --build build
# run example
SERVICE_NAME=asynchronous ./build/asynchronous      # c api
SERVICE_NAME=asynchronous2 ./build/asynchronous2    # c++ api
```
