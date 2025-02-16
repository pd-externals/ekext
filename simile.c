/*
 * simile : windowed similarity comparison
 * Copyright (c) 2005-2023 Edward Kelly
 * Forinformaion on usage and distribution, and for a DICLAIMER OF ALL
 * WARRANTIES, see the file "LICENSE.txt," in this distribution. */

#include "m_pd.h"

typedef struct _simile {
  t_object  x_obj;
  t_float x_win;
  t_float min;
  t_float sign;
  t_float x_in1;
  t_float x_in2;
  t_float x_result;
  t_outlet *x_sim, *x_sign;
} t_simile;

void simile_float(t_simile *x, t_floatarg in1) {
  x->x_in1 = in1;
  x->x_win = x->x_win > 0 ? x->x_win : 0.01; /* defaults to 0.01 so that we avoid /0 errors */
  x->min = ( x->x_in1 > x->x_in2 ) ? (x->x_in1 - x->x_in2) : (x->x_in2 - x->x_in1);
  x->sign = ( x->x_in1 >= x->x_in2 ) ? 1 : -1;
  x->x_result = 1 / ((x->min / x->x_win) + 1);
  outlet_float(x->x_sign, x->sign);
  outlet_float(x->x_sim, x->x_result);
}

void simile_bang(t_simile *x) {
  outlet_float(x->x_sign, x->sign);
  outlet_float(x->x_sim, x->x_result);
}

t_class *simile_class;

void *simile_new(t_floatarg x_win) {
  t_simile *x = (t_simile *)pd_new(simile_class);
  x->x_sim = outlet_new(&x->x_obj, gensym("float"));
  x->x_sign = outlet_new(&x->x_obj, gensym("float"));
  floatinlet_new(&x->x_obj, &x->x_in2);
  floatinlet_new(&x->x_obj, &x->x_win);
  return (void *)x;
}

void simile_setup(void) {
  simile_class = class_new(gensym("simile"),
                           (t_newmethod)simile_new,
                           0, sizeof(t_simile),
                           0, A_DEFFLOAT, 0);
  post("|------------->simile<--------------|");
  post("|->weighted similarity measurement<-|");
  post("|-->edward<----->kelly<----->2005<--|");

  class_addbang(simile_class, simile_bang);
  class_addfloat(simile_class, simile_float);
}
