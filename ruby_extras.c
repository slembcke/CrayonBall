#include <stdio.h>
#include <math.h>

#include <ruby.h>

extern VALUE rb_cpVect;

VALUE rb_ary_sendEach(VALUE self, VALUE args){
	ID id = rb_to_id(rb_ary_shift(args));
	
	int len = RARRAY_LEN(self);	
	VALUE *arg_ptr = RARRAY_PTR(args);
	
	for(int i=0; i<len; i++)
		rb_funcall2(RARRAY_PTR(self)[i], id, RARRAY_LEN(args), arg_ptr);

	return self;
}

VALUE rb_float_clamp(VALUE self, VALUE min, VALUE max){
	double x = NUM2DBL(self);
	if(x < NUM2DBL(min)) return min;
	if(x > NUM2DBL(max)) return max;
	return self;
}

//VALUE rb_float_clamp_bang(VALUE self, VALUE min, VALUE max){
//	double x = RFLOAT(self)->value;
//	double i = NUM2DBL(min);
//	if(x < i){
//		RFLOAT(self)->value = i;
//	} else {
//		double a = NUM2DBL(max);
//		if(x > a) RFLOAT(self)->value = a;
//	}
//	
//	return self;
//}

VALUE rb_float_tolerance(VALUE self, VALUE tol){
	double tolerance = NUM2DBL(tol);
	if(fabs(NUM2DBL(self)) < tolerance) return rb_float_new(0.0);
	return self;
}

//VALUE rb_float_tolerance_bang(VALUE self, VALUE tol){
//	double tolerance = NUM2DBL(tol);
//	if(fabs(RFLOAT(self)->value) < tolerance) RFLOAT(self)->value = 0.0;
//	return self;
//}

VALUE rb_float_rad2deg(VALUE self)
{
	return rb_float_new(NUM2DBL(self) * 180.0/M_PI);
}

VALUE rb_float_deg2rad(VALUE self)
{
	return rb_float_new(NUM2DBL(self) * M_PI/180.0);
}

void Init_myAdditions(void){
	rb_define_method(rb_cArray, "send_each", rb_ary_sendEach, -2);
	
	rb_define_method(rb_cFloat, "clamp", rb_float_clamp, 2);
//	rb_define_method(rb_cFloat, "clamp!", rb_float_clamp_bang, 2);
	rb_define_method(rb_cFloat, "tolerance", rb_float_tolerance, 1);
//	rb_define_method(rb_cFloat, "tolerance!", rb_float_tolerance_bang, 1);
	rb_define_method(rb_cFloat, "rad2deg", rb_float_rad2deg, 0);
	rb_define_method(rb_cFloat, "deg2rad", rb_float_deg2rad, 0);
}
