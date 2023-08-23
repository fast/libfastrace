// Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0.

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

#include <minitrace_c/minitrace_c.h>

template <class T> class CQueue {
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
  void *data;
  ffi::mtr_span *s;
};

static CQueue<data *> q;

#define MAX_NUM_PRODUCED 10
#define MAX_NUM_CONSUMED 5

std::atomic<int> unfinished(2);

void __attribute__((noinline)) baz(void *data) {
  auto ls = minitrace_glue::mtr_create_loc_span_enter("baz");
  std::this_thread::sleep_for(
      std::chrono::milliseconds((int)(unsigned long long)data * 1));
  minitrace_glue::mtr_destroy_loc_span(ls);
}

void consume(const char *arg) {
  auto i = 0;

  while (i < MAX_NUM_CONSUMED) {
    auto n = q.dequeue();
    if (n == nullptr)
      continue;
    auto s = minitrace_glue::mtr_create_child_span_enter(arg, *n->s);
    auto g = minitrace_glue::mtr_set_loc_par_to_span(s);
    baz(n->data);
    delete n;
    minitrace_glue::mtr_destroy_span(s);
    minitrace_glue::mtr_destroy_loc_par_guar(g);
    i++;
  }

  std::atomic_fetch_sub(&unfinished, 1);
}

void __attribute__((noinline)) wait(void) {
  auto ls = minitrace_glue::mtr_create_loc_span_enter("wait");
  while (1) {
    if (!std::atomic_load(&unfinished))
      break;
    std::this_thread::sleep_for(std::chrono::microseconds(1));
  }
  minitrace_glue::mtr_destroy_loc_span(ls);
}

void __attribute__((noinline)) boo(void) {
  auto s = minitrace_glue::mtr_create_child_span_enter_loc("boo");

  for (auto i = 1; i <= MAX_NUM_PRODUCED; i++) {
    auto n = new data;
    assert(n != nullptr);
    n->data = (void *)(unsigned long long)i;
    n->s = &s;
    q.enqueue(n);
  }

  wait();

  minitrace_glue::mtr_destroy_span(s);
}

void __attribute__((noinline)) bar(void) {
  auto ls = minitrace_glue::mtr_create_loc_span_enter("bar");
  boo();
  minitrace_glue::mtr_destroy_loc_span(ls);
}

void __attribute__((noinline)) foo(void) {
  auto ls = minitrace_glue::mtr_create_loc_span_enter("foo");
  bar();
  minitrace_glue::mtr_destroy_loc_span(ls);
}

void produce(void) {
  auto p = minitrace_glue::mtr_create_rand_span_ctx();
  auto r = minitrace_glue::mtr_create_root_span("root", p);
  auto g = minitrace_glue::mtr_set_loc_par_to_span(r);

  foo();

  minitrace_glue::mtr_destroy_loc_par_guar(g);
  minitrace_glue::mtr_destroy_span(r);
}

int main(void) {
  auto gcfg = minitrace_glue::mtr_create_def_otlp_grpcio_cfg();
  auto ecfg = minitrace_glue::mtr_create_def_otlp_exp_cfg();
  auto cfg = minitrace_glue::mtr_create_def_coll_cfg();
  auto rptr = minitrace_glue::mtr_create_otel_rptr(ecfg, gcfg);
  minitrace_glue::mtr_set_otel_rptr(rptr, cfg);

  std::thread p(produce);
  std::thread c1(consume, "consumer-0");
  std::thread c2(consume, "consumer-1");

  p.join();
  c1.join();
  c2.join();

  minitrace_glue::mtr_flush();

  minitrace_glue::mtr_destroy_otel_rptr(rptr);
  return 0;
}
