/*******************************************************************************
** filter.c (filter)
*******************************************************************************/

#include <stdlib.h>
#include <math.h>

#include "clock.h"
#include "global.h"
#include "filter.h"
#include "shaping.h"
#include "tuning.h"

/*******************************************************************************
** filter_init()
*******************************************************************************/
short int filter_init(filter* fltr)
{
  int i;

  if (fltr == NULL)
    return 1;

  fltr->fc_index = 0;
  fltr->res_index = 0;

  for (i = 0; i < 2; i++)
  {
    fltr->s[i] = 0;
    fltr->v[i] = 0;
    fltr->y[i] = 0;
  }

  fltr->level = 0;

  return 0;
}

/*******************************************************************************
** filter_create()
*******************************************************************************/
filter* filter_create()
{
  filter* fltr;

  fltr = malloc(sizeof(filter));
  filter_init(fltr);

  return fltr;
}

/*******************************************************************************
** filter_deinit()
*******************************************************************************/
short int filter_deinit(filter* fltr)
{
  if (fltr == NULL)
    return 1;

  return 0;
}

/*******************************************************************************
** filter_destroy()
*******************************************************************************/
short int filter_destroy(filter* fltr)
{
  if (fltr == NULL)
    return 1;

  filter_deinit(fltr);
  free(fltr);

  return 0;
}

/*******************************************************************************
** filter_set_indices()
*******************************************************************************/
short int filter_set_indices(filter* fltr, int fc_index, int res_index)
{
  if (fltr == NULL)
    return 1;

  /* set cutoff */
  if (fc_index < 0)
    fltr->fc_index = 0;
  else if (fc_index > 4095)
    fltr->fc_index = 4095;
  else
    fltr->fc_index = fc_index;

  /* set resonance */
  if (res_index < 0)
    fltr->res_index = 0;
  else if (res_index > 31)
    fltr->res_index = 31;
  else
    fltr->res_index = res_index;

  return 0;
}

/*******************************************************************************
** filter_reset()
*******************************************************************************/
short int filter_reset(filter* fltr)
{
  int i;

  if (fltr == NULL)
    return 1;

  for (i = 0; i < 2; i++)
  {
    fltr->s[i] = 0;
    fltr->v[i] = 0;
    fltr->y[i] = 0;
  }

  fltr->level = 0;

  return 0;
}

/*******************************************************************************
** filter_update_highpass()
*******************************************************************************/
short int filter_update_highpass(filter* fltr, int input)
{
  float stage_multiplier;

  if (fltr == NULL)
    return 1;

  /* obtain multipliers from tables */
  stage_multiplier = 
    G_filter_stage_multiplier_table[fltr->fc_index];

  /* see Vadim Zavalishin's "The Art of VA Filter Design" (p. 77) */

  /* integrator 1 */
  fltr->v[0] = (int) (((input - fltr->s[0]) * stage_multiplier) + 0.5f);
  fltr->y[0] = fltr->v[0] + fltr->s[0];
  fltr->s[0] = fltr->y[0] + fltr->v[0];

  /* integrator 2 */
  fltr->v[1] = (int) ((((input - fltr->y[0]) - fltr->s[1]) * stage_multiplier) + 0.5f);
  fltr->y[1] = fltr->v[1] + fltr->s[1];
  fltr->s[1] = fltr->y[1] + fltr->v[1];

  /* set output level */
  fltr->level = input - fltr->y[0] - fltr->y[1];

  return 0;
}

/*******************************************************************************
** filter_update_lowpass()
*******************************************************************************/
short int filter_update_lowpass(filter* fltr, int input)
{
  float k;
  float g;
  float stage_multiplier;

  int   u;

  if (fltr == NULL)
    return 1;

  /* obtain multipliers from tables */
  k = G_resonance_table[fltr->res_index];

  g = G_filter_omega_0_delta_t_over_2_table[fltr->fc_index];
  stage_multiplier = G_filter_stage_multiplier_table[fltr->fc_index];

  /* resonant filter: tranposed sallen-key filter                           */
  /* Vadim Zavalishin's "The Art of VA Filter Design", Figure 5.23, p. 152  */

  /* compute input to first integrator */
  u = input + (int) ((k * (fltr->y[0] - fltr->y[1])) + 0.5f);

  /* integrator 1 */
  fltr->v[0] = (int) (((u - fltr->s[0]) * stage_multiplier) + 0.5f);
  fltr->y[0] = fltr->v[0] + fltr->s[0];
  fltr->s[0] = fltr->y[0] + fltr->v[0];

  /* integrator 2 */
  fltr->v[1] = (int) (((fltr->y[0] - fltr->s[1]) * stage_multiplier) + 0.5f);
  fltr->y[1] = fltr->v[1] + fltr->s[1];
  fltr->s[1] = fltr->y[1] + fltr->v[1];

  /* set output level */
  fltr->level = fltr->y[1];

  return 0;
}

#if 0
/*******************************************************************************
** filter_update_lowpass_with_saturator()
*******************************************************************************/
short int filter_update_lowpass_with_saturator(filter* fltr, int input)
{
  float k;
  float g;
  float stage_multiplier;

  int   u;

  int   saturator_input;
  int   saturator_output;
  float m;

  float numerator;
  float denominator;

  int   feedback;

  if (fltr == NULL)
    return 1;

  /* obtain multipliers from tables */
  k = G_resonance_table[fltr->res_index];

  g = G_filter_omega_0_delta_t_over_2_table[fltr->fc_index];
  stage_multiplier = G_filter_stage_multiplier_table[fltr->fc_index];

  /* resonant filter: tranposed sallen-key filter                           */
  /* Vadim Zavalishin's "The Art of VA Filter Design", Figure 5.23, p. 152  */
  /* This version implements a saturator as depicted in Figure 6.11, p. 182 */

  /* compute previous saturator input */
  saturator_input = fltr->y[0] - fltr->y[1];

  /* compute linearization parameters */
  if (saturator_input > 32767 - 2)
  {
    m = 0.0f;
    saturator_output = G_waveshaper_sinh_inverse_table[8191];
  }
  else if (saturator_input < -32767 + 2)
  {
    m = 0.0f;
    saturator_output = -G_waveshaper_sinh_inverse_table[8191];
  }
  else if (saturator_input >= 0)
  {
    m = G_waveshaper_sinh_inverse_derivative_table[(saturator_input + 2) / 4];
    saturator_output = G_waveshaper_sinh_inverse_table[(saturator_input + 2) / 4];
  }
  else
  {
    m = G_waveshaper_sinh_inverse_derivative_table[(-saturator_input + 2) / 4];
    saturator_output = -G_waveshaper_sinh_inverse_table[(-saturator_input + 2) / 4];
  }

  /* compute current saturator input */

  /* we have u as the input to the saturator (i.e., the highpass output */
  /* of the second integrator).                                         */
  /*                                                                    */
  /* u = (1 - g)(g(x + kf(u)) + s_1) - s_2                              */
  /* u = (1 - g)(gx + gkf(u) + s_1) - s_2                               */
  /* u = g(1 - g)x + g(1 - g)kf(u) + (1 - g)s_1 - s_2                   */
  /*                                                                    */
  /* letting f(u) = mu + b:                                             */
  /*                                                                    */
  /* u = g(1 - g)x + g(1 - g)k(mu + b) + (1 - g)s_1 - s_2               */
  /* [1 - g(1 - g)km]u = g(1 - g)x + g(1 - g)kb + (1 - g)s_1 - s_2      */
  /* u = [g(1 - g)x + g(1 - g)kb + (1 - g)s_1 - s_2] / [1 - g(1 - g)km] */

  numerator = (g * (1 - g) * input) + 
              (g * (1 - g) * k * (saturator_output - m * saturator_input)) + 
              ((1 - g) * fltr->s[0]) - fltr->s[1];

  denominator = 1.0f - (g * (1.0f - g) * k * m);

  u = (int) ((numerator / denominator) + 0.5f);

  /* compute feedback */
  if (u > 32767 - 2)
    feedback = (int) ((k * G_waveshaper_sinh_inverse_table[8191]) + 0.5f);
  else if (u < -32767 + 2)
    feedback = (int) ((k * -G_waveshaper_sinh_inverse_table[8191]) + 0.5f);
  else if (u >= 0)
    feedback = (int) ((k * G_waveshaper_sinh_inverse_table[(u + 2) / 4]) + 0.5f);
  else
    feedback = (int) ((k * -G_waveshaper_sinh_inverse_table[(-u + 2) / 4]) + 0.5f);

  /* integrator 1 */
  fltr->v[0] = (int) (((input + feedback - fltr->s[0]) * stage_multiplier) + 0.5f);
  fltr->y[0] = fltr->v[0] + fltr->s[0];
  fltr->s[0] = fltr->y[0] + fltr->v[0];

  /* integrator 2 */
  fltr->v[1] = (int) (((fltr->y[0] - fltr->s[1]) * stage_multiplier) + 0.5f);
  fltr->y[1] = fltr->v[1] + fltr->s[1];
  fltr->s[1] = fltr->y[1] + fltr->v[1];

  /* set output level */
  fltr->level = fltr->y[1];

  return 0;
}
#endif

