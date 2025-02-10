/* isoWrap~ - an isorhythmic phasor~ wrapper
 * Copyright (c) 2005-2023 Edward Kelly
 * Forinformaion on usage and distribution, and for a DICLAIMER OF ALL
 * WARRANTIES, see the file "LICENSE.txt," in this distribution. */

#ifdef __APPLE__
#include <sys/types.h>
#include <sys/time.h>
#include <sys/math.h>
#endif

#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "m_pd.h"

static t_class *isoWrap_tilde_class;

typedef struct _isoWrap_tilde {
  t_object x_obj;

  t_float den, num, nuMult, inMult;
  t_float increment, off1;
  //lcmgcd calculations:
  int lcm, gcd, result, b, iMult;

  int myBug;

  //momentary value adjustments:
  int theCycle, k_i, k_s;//, ratioBal, theLimit, overShoot;
  t_float theOffset, deNorm;

  int resetNextPhase;//, resetNextOffset;

  int deNormalize, prevDeN, reNormFlag, waitPhase;// direction;
  t_float f_s, f_i, f_o, f_prev;
  t_outlet *phOff;
} t_isoWrap_tilde;

void *isoWrap_tilde_new(void)
{
  t_isoWrap_tilde *x = (t_isoWrap_tilde *)pd_new(isoWrap_tilde_class);
  outlet_new(&x->x_obj, gensym("signal"));
  x->phOff = outlet_new(&x->x_obj, &s_float);
  x->deNormalize = 0;
  x->deNorm = 1;
  x->prevDeN = 0;
  //x->ratioBal = 0;
  x->reNormFlag = 0;
  x->f_s = x->f_i = x->f_prev = 0;
  x->num = 4;
  x->den = 4;
  x->lcm = 4;
  x->result = 1;
  x->b = 1;
  x->theCycle = 0;
  //x->theLimit = 1;
  x->theOffset = 0.0;
  x->increment = 0.0;
  x->waitPhase = 0;
  x->myBug = 0;
  return (x);
}

/*static void isoWrap_tilde_floatRes(t_isoWrap_tilde *x, t_floatarg f)
  {
  if(f > 0 && f < 0.1) x->fRes = f;
  else x->fRes = 0;
  }*/

void isoWrap_tilde_calculate_lcm(t_isoWrap_tilde *x)
{
  int a, b, t;

  a = x->result;
  b = x->b;

  while (b != 0) {
    t = b;
    b = a % b;
    a = t;
  }

  x->gcd = a;
  x->lcm = (x->result*x->b)/a;
}

/* It's definitely harder to overshoot this one,
 * since it uses wrapping for values greater than 1 to generate
 * the final wrapped signal. But maybe there's no reason to...
 */
//static void isoWrap_tilde_overShoot(t_isoWrap_tilde *x, t_floatarg f)
//{
//  if(f != 0.0) x->overShoot = 1;
//  else x->overShoot = 0;
//}

void isoWrap_tilde_resetNextPhase(t_isoWrap_tilde *x)
{
  x->resetNextPhase = 1;
}

void isoWrap_tilde_deNormalize(t_isoWrap_tilde *x, t_floatarg f)
{
  x->prevDeN = x->deNormalize;
  x->deNormalize = f == 0 ? 0 : f == 1.0? 1 : f == 2.0 ? 2 : 3;
  if(x->deNormalize == 3)
    {
      x->theCycle = 0;
      x->theOffset = 0;
      x->reNormFlag = 0;
    }
  if(x->prevDeN == 3 && x->deNormalize < 3) x->reNormFlag = 1;
}

void isoWrap_tilde_waitPhase(t_isoWrap_tilde *x, t_floatarg f)
{
  x->waitPhase = (int)f;
}

void isoWrap_tilde_debug(t_isoWrap_tilde *x, t_floatarg f)
{
  x->myBug = f != 0 ? 1 : 0;
}

void isoWrap_tilde_setFraction(t_isoWrap_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
  int numIn, denIn;
  if(argc == 2)
    {
      numIn = (int)atom_getfloat(argv);
      denIn = (int)atom_getfloat(argv+1);
      if(numIn <= 0 || denIn <= 0)
        {
          pd_error(x, "Numerator and denominator of fraction must be integers > 0!");
        }
      else
        {
          //x->ratioBal = x->num > x->den ? 1 : x->den > x->num ? -1 : 0;
          x->num = (t_float)numIn;
          x->den = (t_float)denIn;
          x->result = (int)x->num;
          x->b = (int)x->den;
          isoWrap_tilde_calculate_lcm(x);
          //x->theLimit = x->lcm - 1;
          x->nuMult = (t_float)x->lcm / (t_float)x->den;
          x->inMult = ((t_float)x->num * x->nuMult) / (t_float)x->lcm;
          if(x->myBug)post("x->inMult = %f",x->inMult);
          x->deNorm = 1 / x->inMult;
          if(x->myBug)post("x->deNorm = %f",x->deNorm);
          x->iMult = (int)x->inMult;
          x->off1 = x->inMult > (t_float)x->iMult ? 1.0 : 0.0;
          x->increment = 1 - ((t_float)x->iMult + x->off1 - x->inMult);
        }
    }
}

t_int *isoWrap_tilde_perform(t_int *w)
{
  t_isoWrap_tilde *x = (t_isoWrap_tilde *)(w[1]);
  t_sample *in  = (t_sample *)(w[2]);
  t_sample *out = (t_sample *)(w[3]);
  int   n       = (int)(w[4]);
  while(n--)
    {
      x->f_i = *in++ * x->inMult;
      if(x->f_i < x->f_prev)
        {
          if(x->waitPhase && x->reNormFlag)
            {
              x->theCycle = 0;
              x->theOffset = 0;
              x->reNormFlag = 0;
            }
          //resetNextPhase and resetNextOffset? need to be dealt with here
          else if(x->resetNextPhase)// || x->reNormFlag)
            {
              x->theCycle = 0;
              x->theOffset = 0;
              x->resetNextPhase = 0;
            }
          else if(x->deNormalize == 3 && x->num >= x->den)// || x->reNormFlag)
            {
              x->theCycle = 0;
              x->theOffset = 0;
            }
          else if(x->deNormalize < 3 || x->num < x->den)// && !x->reNormFlag)
            {
              x->theCycle++;
              x->theCycle = x->theCycle % x->lcm;
              x->k_s = (int)((t_float)x->theCycle * x->increment);
              x->theOffset = ((t_float)x->theCycle * x->increment) - x->k_s;
              if(x->myBug) post("The increment: %f",x->increment);
            }
        }
      x->f_s = x->f_i + x->theOffset;
      x->k_i = x->f_s;
      if(x->deNormalize < 3 && (!x->waitPhase || !x->reNormFlag))
        {
          x->f_o = x->f_s - x->k_i;
        }
      else if(x->deNormalize == 3)
        {
          if(x->num >= x->den)
            {
              if(x->f_s < 1.0)
                {
                  x->f_o = x->f_s;
                }
              else
                {
                  x->f_o = 1.0;
                }
            }
          else if(x->num < x->den)
            {
              x->f_o = x->f_s - x->k_i;
            }
        }
      else
        {
          x->f_o = x->f_s - x->k_i;
        }
      if(x->num >= x->den)
        {
          if(x->deNormalize >= 2)// && x->num >= x->den)
            {
              x->f_o = x->f_o * x->deNorm;
            }
          else if(x->deNormalize == 1)
            {
              x->f_o = x->f_o * x->inMult;
            }
        }
      else if(x->num < x->den)
        {
          if(x->deNormalize == 3)
            {
              x->f_o = x->f_o * x->deNorm;
              if(x->f_o > 1.0)
                {
                  x->f_o = 1.0;
                }
            }
          else if(x->deNormalize == 2)
            {
              x->f_o = x->f_o * x->inMult;
            }
          else if(x->deNormalize == 1)
            {
              x->f_o = x->f_o * x->deNorm;
            }
        }
      *out++ = x->f_o;
      x->f_prev = x->f_i;
    }
  outlet_float(x->phOff, x->theOffset);
  return(w+5);
}

void isoWrap_tilde_dsp(t_isoWrap_tilde *x, t_signal **sp) {
  dsp_add(isoWrap_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void isoWrap_tilde_setup(void)
{
  isoWrap_tilde_class = class_new(gensym("isoWrap~"),  (t_newmethod)isoWrap_tilde_new,
                                  0, sizeof(t_isoWrap_tilde), CLASS_DEFAULT, A_DEFFLOAT, 0);
  CLASS_MAINSIGNALIN(isoWrap_tilde_class, t_isoWrap_tilde, f_s);
  class_addmethod(isoWrap_tilde_class, (t_method)isoWrap_tilde_dsp, gensym("dsp"), A_CANT, 0);
  class_addmethod(isoWrap_tilde_class, (t_method)isoWrap_tilde_setFraction, gensym("setFraction"), A_GIMME, 0);
  class_addmethod(isoWrap_tilde_class, (t_method)isoWrap_tilde_resetNextPhase, gensym("resetNextPhase"), A_DEFFLOAT, 0);
  class_addmethod(isoWrap_tilde_class, (t_method)isoWrap_tilde_deNormalize, gensym("deNormalize"), A_DEFFLOAT, 0);
  class_addmethod(isoWrap_tilde_class, (t_method)isoWrap_tilde_debug, gensym("debug"), A_DEFFLOAT, 0);
}
