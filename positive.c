/*
 * positive - wraps floats back into positive numbers from negative
 * Copyright (c) 2005, Edward Kelly <morph_2016@yahoo.co.uk>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "m_pd.h"

t_class *positive_class;

typedef struct _positive
{
  t_object x_obj;
  t_float value;
  t_outlet *positive;
} t_positive;

void positive_float(t_positive *y, t_floatarg f)
{
	y->value = f;
	if(y->value < 0)
	{
		int i = (int)y->value;
		i = i * -1 + 1;
		y->value += (float)i;
	}
	outlet_float(y->positive, y->value);
}

void positive_bang(t_positive *y)
{
    outlet_float(y->positive, y->value);
}

void *positive_new(t_floatarg f)
{
  t_positive *y = (t_positive *)pd_new(positive_class);
  y->value = f;
  y->positive = outlet_new(&y->x_obj, gensym("float"));
  return(void *)y;
}

void positive_setup(void) 
{
  positive_class = class_new(gensym("positive"),
  (t_newmethod)positive_new,
  0, sizeof(t_positive),
			       0, A_DEFFLOAT, 0);
  post("positive always returns a positive value");
  post("negative values are shifted up");

  class_addbang(positive_class, positive_bang);
  class_addfloat(positive_class, positive_float);
}
