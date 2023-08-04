build:
	cargo build --release
install:
	install -D minitrace/include/minitrace_c.h /usr/local/minitrace_c/minitrace_c.h
	install -D target/release/libminitrace_c.a /usr/local/lib
	install -D target/release/libminitrace_c.so /usr/local/lib
clean:
	cargo clean

