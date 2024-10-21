// Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0.

#include <libfastrace/libfastrace.h>

#include <thread>

void func2(int i) {
  fastrace::LocalSpan ls("func2");
  std::this_thread::sleep_for(std::chrono::milliseconds(i * 1));
}

void func1(int i) {
  fastrace::LocalSpan ls("func1");
  std::this_thread::sleep_for(std::chrono::milliseconds(i * 1));
  func2(i);
}

int main(void) {
  auto ecfg = fastrace::createDefaultOTLPExporterConfig();
  auto cfg = fastrace::createDefaultCollectorConfig();

  auto rptr = fastrace::createOpenTelemetryReporter(ecfg);
  fastrace::setOpenTelemetryReporter(rptr, cfg);

  {
    fastrace::SpanContext p;
    fastrace::Span rootSpan("root", p);
    fastrace::LocalParentGuard g(rootSpan);
    fastrace::LocalSpan ls("a span");
    ls.addProperty("k1", "v1");

    for (auto i = 1; i <= 10; i++) {
      func1(i);
    }
  }

  fastrace::flush();

  return 0;
}
