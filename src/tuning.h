/*******************************************************************************
** tuning.h (tuning systems)
*******************************************************************************/

#ifndef TUNING_H
#define TUNING_H

enum
{
  TUNING_SYSTEM_12_ET,
  TUNING_SYSTEM_PYTHAGOREAN,
  TUNING_SYSTEM_QC_MEANTONE,
  TUNING_SYSTEM_JUST,
  TUNING_SYSTEM_WERCKMEISTER_III,
  TUNING_SYSTEM_WERCKMEISTER_IV,
  TUNING_SYSTEM_WERCKMEISTER_V,
  TUNING_SYSTEM_WERCKMEISTER_VI,
  TUNING_SYSTEM_RENOLD_I
};

enum
{
  TUNING_FORK_A440,
  TUNING_FORK_A432,
  TUNING_FORK_C256,
  TUNING_FORK_AMIGA
};

extern float  G_frequency_table[];

extern int    G_phase_increment_table[];

extern float  G_filter_omega_0_delta_t_over_2_table[];
extern float  G_filter_stage_multiplier_table[];

extern float  G_resonance_table[];

/* function declarations */
short int tuning_generate_tables();

#endif
