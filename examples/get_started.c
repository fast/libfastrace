// Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0.

#include <libfastrace/libfastrace.h>

static const char* keys[] = {"pk1", "pk2", "pk3", "pk4", "ck1",
                             "ck2", "ck3", "ck4", "ck5", "ck6"};
static const char* vals[] = {"pv1", "pv2", "pv3", "pv4", "cv1",
                             "cv2", "cv3", "cv4", "cv5", "cv6"};

int main(void) {
  ftr_set_cons_rptr();
  ftr_span_ctx p = ftr_create_rand_span_ctx();
  ftr_span r = ftr_create_root_span("root", p);
  ftr_add_ent_to_par("parent event", &r, keys, vals, 2);
  ftr_span_with_prop(&r, "phello", "pworld");
  ftr_span_with_props(&r, &keys[2], &vals[2], 2);

  ftr_loc_par_guar g = ftr_set_loc_par_to_span(&r);

  ftr_loc_span ls = ftr_create_loc_span_enter("child");
  ftr_add_ent_to_loc_par("child event", &keys[4], &vals[4], 2);
  ftr_loc_span_add_prop("chello", "cworld");
  ftr_loc_span_add_props(&keys[6], &vals[6], 2);
  ftr_loc_span_with_prop(&ls, "chello2", "cworld2");
  ftr_loc_span_with_props(&ls, &keys[8], &vals[8], 2);

  ftr_destroy_loc_span(ls);
  ftr_destroy_loc_par_guar(g);
  ftr_destroy_span(r);

  ftr_flush();
  return 0;
}
