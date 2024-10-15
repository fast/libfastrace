// Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0.

#include <assert.h>
#include <libfastrace/libfastrace.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <unistd.h>

struct node {
  void *data;
  ftr_span *s;
  TAILQ_ENTRY(node) entries;
};

static struct queue {
  pthread_spinlock_t sl;
  TAILQ_HEAD(tailq_head, node) head;
} q = {};

void tq_init(struct queue *q) {
  assert(q);
  TAILQ_INIT(&q->head);
  assert(!pthread_spin_init(&q->sl, PTHREAD_PROCESS_PRIVATE));
}

void tq_finit(struct queue *q) {
  assert(q);
  assert(!pthread_spin_destroy(&q->sl));
}

void tq_enqueue(struct queue *q, void *data) {
  struct node *n = data;
  assert(q);
  assert(n);
  pthread_spin_lock(&q->sl);
  TAILQ_INSERT_TAIL(&q->head, n, entries);
  pthread_spin_unlock(&q->sl);
}

void *tq_dequeue(struct queue *q) {
  struct node *n;
  assert(q);
  pthread_spin_lock(&q->sl);
  if (TAILQ_EMPTY(&q->head)) {
    pthread_spin_unlock(&q->sl);
    return NULL;
  }
  n = TAILQ_FIRST(&q->head);
  TAILQ_REMOVE(&q->head, n, entries);
  pthread_spin_unlock(&q->sl);
  return n;
}

bool tq_empty(struct queue *q) {
  bool ret;
  pthread_spin_lock(&q->sl);
  ret = TAILQ_EMPTY(&q->head);
  pthread_spin_unlock(&q->sl);
  return ret;
}

#define MAX_NUM_PRODUCED 10
#define MAX_NUM_CONSUMED 5

_Atomic int unfinished = 2;

void __attribute__((noinline)) baz(void *data) {
  ftr_loc_span ls = ftr_create_loc_span_enter("baz");
  usleep((int)(unsigned long long)data * 1000);
  ftr_destroy_loc_span(ls);
}

void *consume(void *arg) {
  int i = 0;

  while (i < MAX_NUM_CONSUMED) {
    while (tq_empty(&q))
      ;
    struct node *n = tq_dequeue(&q);
    if (!n)
      continue;
    ftr_span s = ftr_create_child_span_enter((char *)arg, n->s);
    ftr_loc_par_guar g = ftr_set_loc_par_to_span(&s);
    baz(n->data);
    free(n);
    ftr_destroy_span(s);
    ftr_destroy_loc_par_guar(g);
    i++;
  }

  atomic_fetch_sub(&unfinished, 1);
  return NULL;
}

void __attribute__((noinline)) wait(void) {
  ftr_loc_span ls = ftr_create_loc_span_enter("wait");
  while (1) {
    if (!atomic_load(&unfinished))
      break;
    usleep(1);
  }
  ftr_destroy_loc_span(ls);
}

void __attribute__((noinline)) boo(void) {
  ftr_span s = ftr_create_child_span_enter_loc("boo");
  tq_init(&q);

  for (int i = 1; i <= MAX_NUM_PRODUCED; i++) {
    struct node *n = malloc(sizeof(*n));
    assert(n);
    n->data = (void *)(unsigned long long)i;
    n->s = &s;
    tq_enqueue(&q, n);
  }

  wait();

  tq_finit(&q);
  ftr_destroy_span(s);
}

void __attribute__((noinline)) bar(void) {
  ftr_loc_span ls = ftr_create_loc_span_enter("bar");
  boo();
  ftr_destroy_loc_span(ls);
}

void __attribute__((noinline)) foo(void) {
  ftr_loc_span ls = ftr_create_loc_span_enter("foo");
  bar();
  ftr_destroy_loc_span(ls);
}

void *produce(void *arg) {
  ftr_span_ctx p = ftr_create_rand_span_ctx();
  ftr_span r = ftr_create_root_span("root", p);
  ftr_loc_par_guar g = ftr_set_loc_par_to_span(&r);

  foo();

  ftr_destroy_loc_par_guar(g);
  ftr_destroy_span(r);

  return NULL;
}

int main(void) {
  ftr_otlp_exp_cfg ecfg = ftr_create_def_otlp_exp_cfg();
  ftr_coll_cfg cfg = ftr_create_def_coll_cfg();

  ftr_otel_rptr rptr = ftr_create_otel_rptr(ecfg);

  pthread_t t[3];

  ftr_set_otel_rptr(rptr, cfg);

  pthread_create(&t[0], NULL, produce, NULL);
  pthread_create(&t[1], NULL, consume, "consumer-0");
  pthread_create(&t[2], NULL, consume, "consumer-1");

  pthread_join(t[0], NULL);
  pthread_join(t[1], NULL);
  pthread_join(t[2], NULL);

  ftr_flush();

  ftr_destroy_otel_rptr(rptr);
  return 0;
}
