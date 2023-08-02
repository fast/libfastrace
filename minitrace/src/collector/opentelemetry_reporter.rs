pub struct mtr_otel_rptr {
    pub(crate) inner: minitrace_opentelemetry::OpenTelemetryReporter,
}

ffi_fn! {
    fn mtr_create_otel_rptr() -> *mut mtr_otel_rptr {
        let r = minitrace_opentelemetry::OpenTelemetryReporter::new(
            opentelemetry_otlp::SpanExporter::new_grpcio(
                opentelemetry_otlp::ExportConfig {
                    endpoint: std::env::var("OTEL_EXPORTER_OTLP_ENDPOINT").unwrap_or( "localhost:4317".to_string()),
                    protocol: opentelemetry_otlp::Protocol::Grpc,
                    timeout: std::time::Duration::from_secs(
                        opentelemetry_otlp::OTEL_EXPORTER_OTLP_TIMEOUT_DEFAULT,
                    ),
                },
                opentelemetry_otlp::GrpcioConfig::default(),
            ),
            opentelemetry::trace::SpanKind::Server,
            std::borrow::Cow::Owned(opentelemetry::sdk::Resource::new([
                opentelemetry::KeyValue::new("service.name", std::env::var("SERVICE_NAME").unwrap_or("unknown".to_string())),
            ])),
            opentelemetry::InstrumentationLibrary::new(
                "minitrace-opentelemetry-c",
                Some(env!("CARGO_PKG_VERSION")),
                None,
            ),
        );
        Box::into_raw(Box::new(mtr_otel_rptr { inner: r }))
    } ?= std::ptr::null_mut()
}

ffi_fn! {
    fn mtr_free_otel_rptr(r: *mut mtr_otel_rptr) {
        drop(non_null!(Box::from_raw(r) ?= ()));
    }
}
