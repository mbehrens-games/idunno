/*******************************************************************************
** global.h (global variables)
*******************************************************************************/

#ifndef GLOBAL_H
#define GLOBAL_H

#include "sequence.h"
#include "synth.h"

#define PI      3.14159265358979323846
#define TWO_PI  6.28318530717958647693

extern sequencer  G_sequencer;
extern synth      G_synth;

extern int        G_bpm;

extern int        G_export_sampling;
extern int        G_export_period;
extern int        G_export_bitres;

extern int        G_downsampling_m;
extern int        G_downsampling_bound;

extern int        G_tuning_system;
extern int        G_tuning_fork;

/* function declarations */
short int globals_init();
short int globals_deinit();

#endif
