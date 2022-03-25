/*******************************************************************************
** voice.h (synth voice)
*******************************************************************************/

#ifndef VOICE_H
#define VOICE_H

#include "envelope.h"
#include "filter.h"
#include "lfo.h"
#include "patch.h"

typedef struct voice
{
  /* patch */
  patch*        p;

  /* base pitch table indices, phases */
  int           base_pitch_index[PATCH_NUM_PHASES];
  int           phase[PATCH_NUM_PHASES];

  /* noise generator */
  unsigned int  lfsr;

  /* filter indices */
  int           base_fc_index;

  /* filter */
  filter        lowpass;

  /* envelopes */
  envelope      env[PATCH_NUM_ENVELOPES];

  /* lfos */
  lfo           mod[PATCH_NUM_LFOS];

  /* volume level */
  char          volume;

  /* output level */
  int           level;
} voice;

/* function declarations */
short int   voice_init(voice* v);
voice*      voice_create();
short int   voice_deinit(voice* v);
short int   voice_destroy(voice* v);

short int   voice_key_on(voice* v, char note, char volume);
short int   voice_key_off(voice* v);
short int   voice_update(voice* v);

#endif
