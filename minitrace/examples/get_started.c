#include <stdio.h>

#include "minitrace.h"

int main(void)
{
	mtr_coll_cfg *cfg = mtr_create_glob_coll_def_cfg();
	mtr_cons_rptr *rptr = mtr_create_cons_rptr();
	mtr_set_cons_rptr(rptr, cfg);
	mtr_span_ctx *p = mtr_create_rand_span_ctx();
	mtr_span *r = mtr_create_root_span("root", p);
	mtr_guard *g = mtr_set_loc_parent_to_span(r);
	mtr_loc_span *ls = mtr_create_loc_span_enter("a span");

	mtr_free_loc_span(ls);
	mtr_free_guard(g);
	mtr_free_span(r);
	mtr_free_span_ctx(p);
	mtr_free_cons_rptr(rptr);
	mtr_free_glob_coll_def_cfg(cfg);

	mtr_flush();
	return 0;
}
