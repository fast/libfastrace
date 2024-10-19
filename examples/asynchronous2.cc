// Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0.

#include <libfastrace/libfastrace.h>

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

template <class T>
class CQueue {
 public:
  CQueue(void) : q(), m(), c() {}
  ~CQueue(void) {}

  void enqueue(T t) {
    std::lock_guard<std::mutex> lock(m);
    q.push(t);
    c.notify_one();
  }

  T dequeue(void) {
    std::unique_lock<std::mutex> lock(m);
    while (q.empty()) {
      c.wait(lock);
    }
    T val = q.front();
    q.pop();
    return val;
  }

 private:
  std::queue<T> q;
  mutable std::mutex m;
  std::condition_variable c;
};

struct data {
  void* data;
  fastrace::Span* s;
};

static CQueue<data*> q;

#define MAX_NUM_PRODUCED 10
#define MAX_NUM_CONSUMED 5

std::atomic<int> unfinished(2);

void __attribute__((noinline)) baz(void* data) {
  fastrace::LocalSpan ls("baz");
  std::this_thread::sleep_for(
      std::chrono::milliseconds((int)(unsigned long long)data * 1));
}

void consume(const char* arg) {
  auto i = 0;

  while (i < MAX_NUM_CONSUMED) {
    auto n = q.dequeue();
    if (n == nullptr)
      continue;
    fastrace::Span s(arg, *(n->s));
    fastrace::LocalParentGuard g(s);
    baz(n->data);
    delete n;
    i++;
  }

  std::atomic_fetch_sub(&unfinished, 1);
}

void __attribute__((noinline)) wait(void) {
  fastrace::LocalSpan ls("wait");
  while (1) {
    if (!std::atomic_load(&unfinished))
      break;
    std::this_thread::sleep_for(std::chrono::microseconds(1));
  }
}

void __attribute__((noinline)) boo(void) {
  fastrace::Span s("boo");

  for (auto i = 1; i <= MAX_NUM_PRODUCED; i++) {
    auto n = new data;
    assert(n != nullptr);
    n->data = (void*)(unsigned long long)i;
    n->s = &s;
    q.enqueue(n);
  }

  wait();
}

void __attribute__((noinline)) bar(void) {
  fastrace::LocalSpan ls("bar");
  boo();
}

void __attribute__((noinline)) foo(void) {
  fastrace::LocalSpan ls("foo");
  bar();
}

void produce(void) {
  fastrace::SpanContext p;
  fastrace::Span r("root", p);
  fastrace::LocalParentGuard g(r);

  foo();
}

int main(void) {
  auto ecfg = fastrace::createDefaultOTLPExporterConfig();
  auto cfg = fastrace::createDefaultCollectorConfig();
  auto rptr = fastrace::createOpenTelemetryReporter(ecfg);
  fastrace::setOpenTelemetryReporter(rptr, cfg);

  std::thread p(produce);
  std::thread c1(consume, "consumer-0");
  std::thread c2(consume, "consumer-1");

  p.join();
  c1.join();
  c2.join();

  fastrace::flush();

  return 0;
}
