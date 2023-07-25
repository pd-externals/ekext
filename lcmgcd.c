/* lcmgcd - find the lowest common multiple and greatest common factor of a list of numbers
 * (c) 2015 Ed Kelly
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

static t_class *lcmgcd_class;

typedef struct _lcmgcd
{
  t_object x_obj;
  int lcm, gcd, gcdAll, result, b;
  int iterations;
  t_outlet *lcmOut, *gcdOut;
} t_lcmgcd;

static void lcmgcd_calculate_lcm(t_lcmgcd *z) {
  int a, b, t;
 
  a = z->result;
  b = z->b;
 
  while (b != 0) {
    t = b;
    b = a % b;
    a = t;
  }
 
  z->gcd = a;
  z->lcm = (z->result*z->b)/a;
}

static void lcmgcd_calculate_gcd(t_lcmgcd *z) {
  int a, b, t;

  a = z->gcdAll;
  b = z->b;

  while (b != 0) {
    t = b;
    b = a % b;
    a = t;
  }

  z->gcdAll = a;
}

static void lcmgcd_list(t_lcmgcd *z, t_symbol *s, int argc, t_atom *argv)
{
  int i;
  z->iterations = 0;
  z->gcdAll = 1;
  if(argc > 1)
  {
    z->result = (int)atom_getfloat(argv++);
    argc--;
    // make sure the first value is > 0
    while(z->result < 1 && argc)
      {
	z->result = (int)atom_getfloat(argv++);
	argc--;
      }
    for(i=0;i<argc;i++)
      {
	z->b = (int)atom_getfloat(argv++);
	if(z->b > 0)
	  {
	    lcmgcd_calculate_lcm(z);
	    z->result = z->lcm;
	    if(!z->iterations) z->gcdAll = z->gcd;
	    else
	      {
		lcmgcd_calculate_gcd(z);
	      }
	    z->iterations++;
	  }
      }
    outlet_float(z->gcdOut,(t_float)z->gcdAll);
    outlet_float(z->lcmOut,(t_float)z->lcm);
 
  }
  else
    {
      post("You need to provide a list");
      post("with more than one number!");
    }
}

static void *lcmgcd_new()
{
  t_lcmgcd *z = (t_lcmgcd *)pd_new(lcmgcd_class);
  z->result = 1;  
  z->lcm = 1;
  z->gcd = 1;
  z->gcdAll = 1;
  z->lcmOut = outlet_new(&z->x_obj, &s_float);
  z->gcdOut = outlet_new(&z->x_obj, &s_float);
  return(void *)z;
}

void lcmgcd_setup(void)
{
  lcmgcd_class = class_new(gensym("lcmgcd"), (t_newmethod)lcmgcd_new, 
			      0, sizeof(t_lcmgcd), 0 ,A_GIMME, 0);
  class_addlist    (lcmgcd_class, lcmgcd_list);
}
