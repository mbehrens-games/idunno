/*******************************************************************************
** synth.h (individual synth)
*******************************************************************************/

#ifndef SYNTH_H
#define SYNTH_H

#include "filter.h"
#include "patch.h"
#include "reverb.h"
#include "voice.h"

#define SYNTH_MAX_VOICES 6

typedef struct synth
{
  /* patch */
  patch   p;

  /* voices */
  voice   v[SYNTH_MAX_VOICES];

  /* highpass filter */
  filter  highpass;

  /* reverb */
  reverb  r;

  /* output level */
  int     level;
} synth;

/* function declarations */
short int   synth_init(synth* s);
synth*      synth_create();
short int   synth_deinit(synth* s);
short int   synth_destroy(synth* s);

short int   synth_setup(synth* s);
short int   synth_key_on(synth* s, int voice_num, char note, char volume);
short int   synth_key_off(synth* s, int voice_num);
short int   synth_update(synth* s);

#endif
