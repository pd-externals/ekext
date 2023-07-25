/*
 * ninjacount - section number skip forwards or backwards for section-based arrangements
 * Copyright (c) 2012, Dr Edward Kelly <morph_2016@yahoo.co.uk>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, 
 * this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright notice, this list 
 * of conditions and the following disclaimer in the documentation and/or other materials 
 * 
 * provided with the distribution.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "m_pd.h"

t_class *ninjacount_class;

typedef struct _ninjacount
{
  t_object x_obj;
  t_int f_count;
  t_float f_dir, f_prevdir, upper, lower, newfloat;
  t_outlet *count; //, *dir;
} t_ninjacount;

void ninjacount_float(t_ninjacount *y, t_floatarg f)
{
  y->f_count = f;
  y->newfloat = 1;
}

void ninjacount_bang(t_ninjacount *y)
{
  if(y->f_dir == 0)
  {
    if(y->f_count < y->upper)
    {
        y->f_count++;
        outlet_float(y->count, y->f_count);
    }
  }
  else if(y->f_dir > y->f_prevdir)
  {
    outlet_float(y->count, y->f_count);
    y->newfloat = 0;
  }
  else if(y->f_dir == 1)
  {
    if(y->newfloat == 0)
    {
      y->f_count--;
      if(y->f_count < y->lower)
      {
      y->f_count = y->lower;
      }
    }
    outlet_float(y->count, y->f_count);
    y->newfloat = 0;
  }
  y->f_prevdir = y->f_dir;
}

void ninjacount_limits(t_ninjacount *y, t_floatarg f1, t_floatarg f2)
{
  y->lower = f1;
  y->upper = f2 >= f1 ? f2 : f1;
}

void *ninjacount_new(t_floatarg f1, t_floatarg f2, t_floatarg f3)
{
  t_ninjacount *y = (t_ninjacount *)pd_new(ninjacount_class);
  y->f_dir = f1;
  y->lower = f2;
  y->upper = f3 > f2 ? f3 : 127;
  y->f_count = y->f_dir == 0 ? y->lower : y->upper;
  y->newfloat = 0;
  floatinlet_new(&y->x_obj, &y->f_dir);
  y->count = outlet_new(&y->x_obj, gensym("float"));
//  y->dir = outlet_new(&y->x_obj, gensym("float"));
  return(void *)y;
}

void ninjacount_setup(void) 
{
  ninjacount_class = class_new(gensym("ninjacount"),
  (t_newmethod)ninjacount_new,
  0, sizeof(t_ninjacount),
			       0, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
  post("ninjacount counts up normally");
  post("and counts down after two bangs");

  class_addbang(ninjacount_class, ninjacount_bang);
  class_addfloat(ninjacount_class, ninjacount_float);
  class_addmethod(ninjacount_class, (t_method)ninjacount_limits, gensym("limits"), A_DEFFLOAT, A_DEFFLOAT, 0);
}
