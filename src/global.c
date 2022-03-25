/*******************************************************************************
** global.c (global variables)
*******************************************************************************/

#include <stdio.h>
#include <math.h>

#include "clock.h"
#include "global.h"
#include "sequence.h"
#include "synth.h"
#include "tuning.h"

/* global variables */
sequencer G_sequencer;
synth     G_synth;

int       G_bpm;

int       G_export_sampling;
int       G_export_period;
int       G_export_bitres;

int       G_downsampling_m;
int       G_downsampling_bound;

int       G_tuning_system;
int       G_tuning_fork;

/*******************************************************************************
** globals_init()
*******************************************************************************/
short int globals_init()
{
  /* initialize variables */
  sequencer_init(&G_sequencer);
  synth_init(&G_synth);

  G_bpm = 120;

  G_export_sampling = 44100;
  G_export_period = 22676;
  G_export_bitres = 16;

  G_downsampling_m = 128;
  G_downsampling_bound = (G_downsampling_m / 2) + 1;

  G_tuning_fork = TUNING_SYSTEM_12_ET;
  G_tuning_fork = TUNING_FORK_A440;

  return 0;
}

/*******************************************************************************
** globals_deinit()
*******************************************************************************/
short int globals_deinit()
{
  sequencer_deinit(&G_sequencer);
  synth_deinit(&G_synth);

  return 0;
}

