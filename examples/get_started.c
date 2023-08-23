// Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0.

#include <stdio.h>
#include <minitrace_c/minitrace_c.h>

int main(void)
{
	mtr_set_cons_rptr();
	mtr_span_ctx p = mtr_create_rand_span_ctx();
	mtr_span r = mtr_create_root_span("root", p);
	mtr_loc_par_guar g = mtr_set_loc_par_to_span(&r);
	mtr_loc_span ls = mtr_create_loc_span_enter("a span");

        mtr_destroy_loc_span(ls);
        mtr_destroy_loc_par_guar(g);
	mtr_destroy_span(r);

	mtr_flush();
	return 0;
}
