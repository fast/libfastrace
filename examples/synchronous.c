// Copyright 2023 Wenbo Zhang. Licensed under Apache-2.0.

#include <unistd.h>
#include <minitrace_c/minitrace_c.h>

void func2(int i)
{
	mtr_loc_span ls = mtr_create_loc_span_enter("func2");
	usleep(i * 1000);
	mtr_destroy_loc_span(ls);
}

void func1(int i) {
	mtr_loc_span ls = mtr_create_loc_span_enter("func1");
	usleep(i * 1000);
	func2(i);
	mtr_destroy_loc_span(ls);
}

int main(void)
{
        mtr_otlp_grpcio_cfg gcfg = mtr_create_def_otlp_grpcio_cfg();
	mtr_otlp_exp_cfg ecfg = mtr_create_def_otlp_exp_cfg();
	mtr_coll_cfg cfg = mtr_create_def_coll_cfg();

	mtr_otel_rptr rptr = mtr_create_otel_rptr(ecfg, gcfg);
	mtr_set_otel_rptr(rptr, cfg);

	mtr_span_ctx p = mtr_create_rand_span_ctx();
	mtr_span r = mtr_create_root_span("root", p);
	mtr_loc_par_guar g = mtr_set_loc_par_to_span(&r);
	mtr_loc_span ls = mtr_create_loc_span_enter("a span");
	// mtr_add_prop_to_loc_span(ls, "a property", "a value");

	for (int i = 1; i <= 10; i++) {
	    func1(i);
	}

	mtr_destroy_loc_span(ls);
        mtr_destroy_loc_par_guar(g);
	mtr_destroy_span(r);

	mtr_flush();

	mtr_destroy_otel_rptr(rptr);
	return 0;
}
