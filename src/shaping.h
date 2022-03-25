/*******************************************************************************
** shaping.h (waveshaping)
*******************************************************************************/

#ifndef SHAPING_H
#define SHAPING_H

extern int    G_waveshaper_tanh_table[];

#if 0
extern int    G_waveshaper_sinh_inverse_table[];
extern float  G_waveshaper_sinh_inverse_derivative_table[];
#endif

/* function declarations */
short int shaping_generate_tables();

#endif
