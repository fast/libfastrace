// Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0.

#include "libfastrace.h"

#include <cstring>
#include <type_traits>
#include <utility>
#include <vector>

#include "lib.rs.h"

namespace {

template <typename T>
class RustTypeWrapper {
  typename std::aligned_storage<sizeof(T), alignof(T)>::type storage;
  T* ptr;

 public:
  template <typename... Args>
  RustTypeWrapper(Args&&... args)
      : ptr(new(&storage) T(std::forward<Args>(args)...)) {}

  ~RustTypeWrapper() { ptr->~T(); }

  T& get() { return *ptr; }
  const T& get() const { return *ptr; }
};

template <typename CType, typename RustType, typename... Args>
CType call_rust_function(RustType (*rust_func)(Args...),
                         typename std::decay<Args>::type... args) {
  static_assert(sizeof(CType) == sizeof(RustType),
                "CType must be the same size as RustType");
  static_assert(alignof(CType) == alignof(RustType),
                "CType must have the same alignment as RustType");

  RustTypeWrapper<RustType> wrapper(
      rust_func(std::forward<decltype(args)>(args)...));

  return *reinterpret_cast<CType*>(&wrapper.get());
}

template <typename Type, typename... Args>
Type call_rust_function(Type (*rust_func)(Args...),
                        typename std::decay<Args>::type... args) {
  return rust_func(std::forward<decltype(args)>(args)...);
}

template <typename T>
T deref_or_self(const T* ptr) {
  static const T default_value = T();
  return ptr ? *ptr : default_value;
}

}  // anonymous namespace

extern "C" {

ftr_span_ctx ftr_create_rand_span_ctx() {
  return call_rust_function<ftr_span_ctx>(
      &fastrace_glue::ftr_create_rand_span_ctx);
}

ftr_span_ctx ftr_create_span_ctx(const ftr_span* span) {
  ffi::ftr_span rust_span =
      deref_or_self(reinterpret_cast<const ffi::ftr_span*>(span));
  return call_rust_function<ftr_span_ctx>(&fastrace_glue::ftr_create_span_ctx,
                                          rust_span);
}

ftr_span_ctx ftr_create_span_ctx_loc() {
  return call_rust_function<ftr_span_ctx>(
      &fastrace_glue::ftr_create_span_ctx_loc);
}

ftr_span_ctx ftr_span_ctx_set_sampled(ftr_span_ctx ctx, bool sampled) {
  return call_rust_function<ftr_span_ctx>(
      &fastrace_glue::ftr_span_ctx_set_sampled,
      *reinterpret_cast<ffi::ftr_span_ctx*>(&ctx), sampled);
}

ftr_span ftr_create_root_span(const char* name, ftr_span_ctx parent) {
  const ffi::ftr_span_ctx& rust_parent =
      *reinterpret_cast<const ffi::ftr_span_ctx*>(&parent);
  return call_rust_function<ftr_span>(&fastrace_glue::ftr_create_root_span,
                                      rust::Str(name), rust_parent);
}

ftr_span ftr_create_child_span_enter(const char* name, const ftr_span* parent) {
  ffi::ftr_span rust_parent =
      deref_or_self(reinterpret_cast<const ffi::ftr_span*>(parent));
  return call_rust_function<ftr_span>(
      &fastrace_glue::ftr_create_child_span_enter, rust::Str(name),
      rust_parent);
}

ftr_span ftr_create_child_span_enter_mul(const char* name,
                                         const ftr_span* parents, size_t n) {
  return call_rust_function<ftr_span>(
      &fastrace_glue::ftr_create_child_span_enter_mul, rust::Str(name),
      rust::Slice<const ffi::ftr_span>(
          reinterpret_cast<const ffi::ftr_span*>(parents), n));
}

ftr_span ftr_create_child_span_enter_loc(const char* name) {
  return call_rust_function<ftr_span>(
      &fastrace_glue::ftr_create_child_span_enter_loc, rust::Str(name));
}

void ftr_cancel_span(ftr_span span) {
  fastrace_glue::ftr_cancel_span(*reinterpret_cast<ffi::ftr_span*>(&span));
}

void ftr_destroy_span(ftr_span span) {
  fastrace_glue::ftr_destroy_span(*reinterpret_cast<ffi::ftr_span*>(&span));
}

ftr_loc_par_guar ftr_set_loc_par_to_span(const ftr_span* span) {
  ffi::ftr_span rust_span =
      deref_or_self(reinterpret_cast<const ffi::ftr_span*>(span));
  return call_rust_function<ftr_loc_par_guar>(
      &fastrace_glue::ftr_set_loc_par_to_span, rust_span);
}

void ftr_span_with_prop(ftr_span* span, const char* key, const char* val) {
  fastrace_glue::ftr_span_with_prop(*reinterpret_cast<ffi::ftr_span*>(span),
                                    rust::Str(key), rust::Str(val));
}

void ftr_span_with_props(ftr_span* span, const char** keys, const char** vals,
                         size_t n) {
  fastrace_glue::ftr_span_with_props(*reinterpret_cast<ffi::ftr_span*>(span),
                                     rust::Slice<const char* const>(keys, n),
                                     rust::Slice<const char* const>(vals, n));
}

void ftr_add_ent_to_par(const char* name, ftr_span* span, const char** keys,
                        const char** vals, size_t n) {
  std::vector<const char*> rust_keys(keys, keys + n);
  std::vector<const char*> rust_vals(vals, vals + n);
  fastrace_glue::ftr_add_ent_to_par(
      rust::Str(name), *reinterpret_cast<ffi::ftr_span*>(span),
      rust::Slice<const char* const>(rust_keys.data(), n),
      rust::Slice<const char* const>(rust_vals.data(), n));
}

void ftr_destroy_loc_par_guar(ftr_loc_par_guar guard) {
  fastrace_glue::ftr_destroy_loc_par_guar(
      *reinterpret_cast<ffi::ftr_loc_par_guar*>(&guard));
}

void ftr_push_child_spans_to_cur(const ftr_span* span,
                                 ftr_loc_spans local_span) {
  fastrace_glue::ftr_push_child_spans_to_cur(
      *reinterpret_cast<const ffi::ftr_span*>(span),
      *reinterpret_cast<ffi::ftr_loc_spans*>(&local_span));
}

ftr_loc_span ftr_create_loc_span_enter(const char* name) {
  return call_rust_function<ftr_loc_span>(
      &fastrace_glue::ftr_create_loc_span_enter, rust::Str(name));
}

void ftr_loc_span_add_prop(const char* key, const char* val) {
  fastrace_glue::ftr_loc_span_add_prop(rust::Str(key), rust::Str(val));
}

void ftr_loc_span_add_props(const char** keys, const char** vals, size_t n) {
  std::vector<const char*> rust_keys(keys, keys + n);
  std::vector<const char*> rust_vals(vals, vals + n);
  fastrace_glue::ftr_loc_span_add_props(
      rust::Slice<const char* const>(rust_keys.data(), n),
      rust::Slice<const char* const>(rust_vals.data(), n));
}

void ftr_loc_span_with_prop(ftr_loc_span* span, const char* key,
                            const char* val) {
  fastrace_glue::ftr_loc_span_with_prop(
      *reinterpret_cast<ffi::ftr_loc_span*>(span), rust::Str(key),
      rust::Str(val));
}

void ftr_loc_span_with_props(ftr_loc_span* span, const char** keys,
                             const char** vals, size_t n) {
  std::vector<const char*> rust_keys(keys, keys + n);
  std::vector<const char*> rust_vals(vals, vals + n);
  fastrace_glue::ftr_loc_span_with_props(
      *reinterpret_cast<ffi::ftr_loc_span*>(span),
      rust::Slice<const char* const>(rust_keys.data(), n),
      rust::Slice<const char* const>(rust_vals.data(), n));
}

void ftr_add_ent_to_loc_par(const char* name, const char** keys,
                            const char** vals, size_t n) {
  std::vector<const char*> rust_keys(keys, keys + n);
  std::vector<const char*> rust_vals(vals, vals + n);
  fastrace_glue::ftr_add_ent_to_loc_par(
      rust::Str(name), rust::Slice<const char* const>(rust_keys.data(), n),
      rust::Slice<const char* const>(rust_vals.data(), n));
}

void ftr_destroy_loc_span(ftr_loc_span span) {
  fastrace_glue::ftr_destroy_loc_span(
      *reinterpret_cast<ffi::ftr_loc_span*>(&span));
}

ftr_loc_coll ftr_start_loc_coll() {
  return call_rust_function<ftr_loc_coll>(&fastrace_glue::ftr_start_loc_coll);
}

ftr_loc_spans ftr_collect_loc_spans(ftr_loc_coll lc) {
  return call_rust_function<ftr_loc_spans>(
      &fastrace_glue::ftr_collect_loc_spans,
      *reinterpret_cast<ffi::ftr_loc_coll*>(&lc));
}

ftr_coll_cfg ftr_create_def_coll_cfg() {
  return call_rust_function<ftr_coll_cfg>(
      &fastrace_glue::ftr_create_def_coll_cfg);
}

ftr_coll_cfg ftr_set_max_spans_per_trace(ftr_coll_cfg cfg, size_t max) {
  return call_rust_function<ftr_coll_cfg>(
      &fastrace_glue::ftr_set_max_spans_per_trace,
      *reinterpret_cast<ffi::ftr_coll_cfg*>(&cfg), max);
}

ftr_coll_cfg ftr_set_report_interval(ftr_coll_cfg cfg, uint64_t ri) {
  return call_rust_function<ftr_coll_cfg>(
      &fastrace_glue::ftr_set_report_interval,
      *reinterpret_cast<ffi::ftr_coll_cfg*>(&cfg), ri);
}

void ftr_set_cons_rptr() { fastrace_glue::ftr_set_cons_rptr(); }

ftr_otlp_exp_cfg ftr_create_def_otlp_exp_cfg() {
  return call_rust_function<ftr_otlp_exp_cfg>(
      &fastrace_glue::ftr_create_def_otlp_exp_cfg);
}

ftr_otel_rptr ftr_create_otel_rptr(ftr_otlp_exp_cfg cfg) {
  return call_rust_function<ftr_otel_rptr>(
      &fastrace_glue::ftr_create_otel_rptr,
      *reinterpret_cast<ffi::ftr_otlp_exp_cfg*>(&cfg));
}

void ftr_destroy_otel_rptr(ftr_otel_rptr rptr) {
  fastrace_glue::ftr_destroy_otel_rptr(
      *reinterpret_cast<ffi::ftr_otel_rptr*>(&rptr));
}

void ftr_set_otel_rptr(ftr_otel_rptr rptr, ftr_coll_cfg cfg) {
  fastrace_glue::ftr_set_otel_rptr(
      *reinterpret_cast<ffi::ftr_otel_rptr*>(&rptr),
      *reinterpret_cast<ffi::ftr_coll_cfg*>(&cfg));
}

void ftr_flush() { fastrace_glue::ftr_flush(); }

}  // extern "C"

namespace fastrace {

SpanContext::SpanContext() : ctx_(ftr_create_rand_span_ctx()) {}

SpanContext::SpanContext(const ftr_span_ctx& ctx) : ctx_(ctx) {}

ftr_span_ctx SpanContext::raw() const { return ctx_; }

void SpanContext::setSampled(bool sampled) {
  ctx_ = ftr_span_ctx_set_sampled(ctx_, sampled);
}

Span::Span(const char* name, const SpanContext& parent)
    : span_(new ftr_span(ftr_create_root_span(name, parent.raw()))) {}

Span::Span(const char* name, const Span& parent)
    : span_(new ftr_span(ftr_create_child_span_enter(name, parent.raw()))) {}

Span::Span(const char* name)
    : span_(new ftr_span(ftr_create_child_span_enter_loc(name))) {}

Span::Span(Span&& other) noexcept : span_(std::move(other.span_)) {}

Span& Span::operator=(Span&& other) noexcept {
  if (this != &other) {
    span_ = std::move(other.span_);
  }
  return *this;
}

Span::~Span() {
  if (span_) {
    ftr_destroy_span(*span_);
  }
}

void Span::cancel() {
  if (span_) {
    ftr_cancel_span(*span_);
  }
}

void Span::addProperty(const std::string& key, const std::string& value) {
  if (span_) {
    ftr_span_with_prop(span_.get(), key.c_str(), value.c_str());
  }
}

void Span::addProperties(
    const std::vector<std::pair<std::string, std::string> >& properties) {
  if (span_ && !properties.empty()) {
    std::vector<const char*> keys;
    std::vector<const char*> values;
    for (size_t i = 0; i < properties.size(); ++i) {
      keys.push_back(properties[i].first.c_str());
      values.push_back(properties[i].second.c_str());
    }
    ftr_span_with_props(span_.get(), &keys[0], &values[0], properties.size());
  }
}

void Span::addEvent(
    const std::string& name,
    const std::vector<std::pair<std::string, std::string> >& properties) {
  if (span_ && !properties.empty()) {
    std::vector<const char*> keys;
    std::vector<const char*> values;
    for (size_t i = 0; i < properties.size(); ++i) {
      keys.push_back(properties[i].first.c_str());
      values.push_back(properties[i].second.c_str());
    }
    ftr_add_ent_to_par(name.c_str(), span_.get(), &keys[0], &values[0],
                       properties.size());
  }
}

ftr_span* Span::raw() { return span_.get(); }

const ftr_span* Span::raw() const { return span_.get(); }

LocalParentGuard::LocalParentGuard(const Span& span)
    : guard_(ftr_set_loc_par_to_span(span.raw())) {}

LocalParentGuard::~LocalParentGuard() { ftr_destroy_loc_par_guar(guard_); }

LocalSpan::LocalSpan(const char* name)
    : span_(ftr_create_loc_span_enter(name)) {}

LocalSpan::LocalSpan(LocalSpan&& other) noexcept : span_(other.span_) {
  other.span_ = ftr_loc_span{};
}

LocalSpan& LocalSpan::operator=(LocalSpan&& other) noexcept {
  if (this != &other) {
    ftr_destroy_loc_span(span_);
    span_ = other.span_;
    other.span_ = ftr_loc_span{};
  }
  return *this;
}
LocalSpan::~LocalSpan() { ftr_destroy_loc_span(span_); }

void LocalSpan::addProperty(const std::string& key, const std::string& value) {
  ftr_loc_span_with_prop(&span_, key.c_str(), value.c_str());
}

void LocalSpan::addProperties(
    const std::vector<std::pair<std::string, std::string> >& properties) {
  if (!properties.empty()) {
    std::vector<const char*> keys;
    std::vector<const char*> values;
    for (size_t i = 0; i < properties.size(); ++i) {
      keys.push_back(properties[i].first.c_str());
      values.push_back(properties[i].second.c_str());
    }
    ftr_loc_span_with_props(&span_, &keys[0], &values[0], properties.size());
  }
}

void LocalSpan::addEvent(
    const std::string& name,
    const std::vector<std::pair<std::string, std::string> >& properties) {
  if (!properties.empty()) {
    std::vector<const char*> keys;
    std::vector<const char*> values;
    for (size_t i = 0; i < properties.size(); ++i) {
      keys.push_back(properties[i].first.c_str());
      values.push_back(properties[i].second.c_str());
    }
    ftr_add_ent_to_loc_par(name.c_str(), &keys[0], &values[0],
                           properties.size());
  }
}

CollectorConfig::CollectorConfig() : cfg_(ftr_create_def_coll_cfg()) {}

void CollectorConfig::setMaxSpansPerTrace(size_t max) {
  cfg_ = ftr_set_max_spans_per_trace(cfg_, max);
}

void CollectorConfig::setReportInterval(uint64_t interval) {
  cfg_ = ftr_set_report_interval(cfg_, interval);
}

ftr_coll_cfg CollectorConfig::raw() const { return cfg_; }

OTLPExporterConfig::OTLPExporterConfig()
    : cfg_(ftr_create_def_otlp_exp_cfg()) {}

ftr_otlp_exp_cfg OTLPExporterConfig::raw() const { return cfg_; }

OpenTelemetryReporter::OpenTelemetryReporter(const OTLPExporterConfig& config)
    : reporter_(ftr_create_otel_rptr(config.raw())) {}

OpenTelemetryReporter::~OpenTelemetryReporter() {
  ftr_destroy_otel_rptr(reporter_);
}

void OpenTelemetryReporter::setReporter(const CollectorConfig& config) {
  ftr_set_otel_rptr(reporter_, config.raw());
}

OTLPExporterConfig createDefaultOTLPExporterConfig() {
  return OTLPExporterConfig();
}

CollectorConfig createDefaultCollectorConfig() { return CollectorConfig(); }

OpenTelemetryReporter createOpenTelemetryReporter(
    const OTLPExporterConfig& config) {
  return OpenTelemetryReporter(config);
}

void setOpenTelemetryReporter(OpenTelemetryReporter& reporter,
                              const CollectorConfig& config) {
  reporter.setReporter(config);
}

void setConsoleReporter() { ftr_set_cons_rptr(); }

void flush() { ftr_flush(); }
}  // namespace fastrace
