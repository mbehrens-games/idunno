/*******************************************************************************
** shaping.c (waveshaping)
*******************************************************************************/

#include <math.h>

#include "shaping.h"

/* hyperbolic tangent waveshaper table  */
/* 13-bit index: 8192 entries           */
int G_waveshaper_tanh_table[8192];

#if 0
/* inverse hyperbolic sine & its derivative tables  */
/* 13-bit index: 8192 entries                       */
int   G_waveshaper_sinh_inverse_table[8192];
float G_waveshaper_sinh_inverse_derivative_table[8192];
#endif

/*******************************************************************************
** shaping_generate_tables()
*******************************************************************************/
short int shaping_generate_tables()
{
  int   i;

  float val;

  /* hyperbolic tangent                           */
  /* note: if tanh x is multiplied by 1/tanh(1),  */
  /* the interval [-1, 1] maps to [-1, 1]         */
  /* also, 1/tanh(1) = 1.313035285499331f         */
  for (i = 0; i < 8192; i++)
  {
    val = 1.313035285499331f * tanhf(i / 8192.0f);

    G_waveshaper_tanh_table[i] = 
      (int) ((32767 * val) + 0.5f);
  }

#if 0
  /* inverse hyperbolic sine                      */
  /* note that sinh^-1(x) = ln(x + sqrt(x^2 + 1)) */
  /* also, d/dx(sinh^-1(x)) = 1/sqrt(x^2 + 1)     */
  for (i = 0; i < 8192; i++)
  {
    val = i / 8192.0f;

    val = log(val + sqrt((val * val) + 1));

    G_waveshaper_sinh_inverse_table[i] = 
      (int) ((32767 * val) + 0.5f);
  }

  /* inverse hyperbolic sine derivative */
  for (i = 0; i < 8192; i++)
  {
    val = i / 8192.0f;

    G_waveshaper_sinh_inverse_derivative_table[i] = 
      1.0f / sqrt((val * val) + 1);
  }
#endif

#if 0
  printf("Hyperbolic Tangent Table:\n");

  for (i = 0; i < (8192 / 4); i++)
  {
    printf("%d %d %d %d\n", G_waveshaper_tanh_table[4 * i + 0], 
                            G_waveshaper_tanh_table[4 * i + 1], 
                            G_waveshaper_tanh_table[4 * i + 2], 
                            G_waveshaper_tanh_table[4 * i + 3]);
  }
#endif

#if 0
  printf("Inverse Hyperbolic Sine Table:\n");

  for (i = 0; i < (8192 / 4); i++)
  {
    printf("%d %d %d %d\n", G_waveshaper_sinh_inverse_table[4 * i + 0], 
                            G_waveshaper_sinh_inverse_table[4 * i + 1], 
                            G_waveshaper_sinh_inverse_table[4 * i + 2], 
                            G_waveshaper_sinh_inverse_table[4 * i + 3]);
  }
#endif

#if 0
  int level;

  printf("Waveshaping Table Test:\n");

  for (i = -32768; i < 32768; i++)
  {
    if (i > 32767 - 2)
      level = 32767;
    else if (i < -32767 + 2)
      level = -32767;
    else if (i >= 0)
      level = G_waveshaper_tanh_table[(i + 2) / 4];
    else
      level = -G_waveshaper_tanh_table[(-i + 2) / 4];

    printf("Table Index: %d, Shaped Value: %d\n", i, level);
  }
#endif

  return 0;
}

