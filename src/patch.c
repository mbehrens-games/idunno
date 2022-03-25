/*******************************************************************************
** patch.c (patch settings)
*******************************************************************************/

#include <stdlib.h>
#include <math.h>

#include "global.h"
#include "lfo.h"
#include "patch.h"
#include "waveform.h"

/*******************************************************************************
** patch_init()
*******************************************************************************/
short int patch_init(patch* p)
{
  int i;

  if (p == NULL)
    return 1;

  /* waveform generators */
  for (i = 0; i < PATCH_NUM_WAVES; i++)
    p->waveform[i] = OSC_WAVEFORM_SQUARE;

  p->phi = 0;
  p->sync = 0;
  p->wave_mix = 16;
  p->ring_mod = 0;

  /* oscillators */
  for (i = 0; i < PATCH_NUM_OSCS; i++)
  {
    p->detune_octave[i] = 0;
    p->detune_coarse[i] = 0;
    p->detune_fine[i]   = 0;
  }

  /* noise generator */
  p->noise_period = 0;
  p->noise_mix = 0;

  /* lowpass filter */
  p->cutoff = 0;
  p->keytrack = 0;
  p->resonance = 0;

  /* reverb */
  p->rev_delay = 0;

  for (i = 0; i < 8; i++)
    p->rev_c[i] = 0;

  p->rev_feedback = 0;
  p->rev_vol = 0;

  /* envelopes */
  for (i = 0; i < PATCH_NUM_ENVELOPES; i++)
  {
    p->ar[i] = 0;
    p->dr[i] = 0;
    p->sr[i] = 0;
    p->rr[i] = 0;
    p->tl[i] = 127;
    p->sl[i] = 0;

    p->rks[i] = 0;
    p->lks[i] = 0;
  }

  /* lfos */
  for (i = 0; i < PATCH_NUM_LFOS; i++)
  {
    p->mod_waveform[i] = LFO_WAVEFORM_SINE;
    p->mod_speed[i] = 0;
    p->mod_depth[i] = 0;
    p->mod_delay[i] = 0;
  }

  /* highpass filter, soft clipping */
  p->hpf = 0;
  p->soft_clip = 0;

  return 0;
}

/*******************************************************************************
** patch_create()
*******************************************************************************/
patch* patch_create()
{
  patch* p;

  p = malloc(sizeof(patch));
  patch_init(p);

  return p;
}

/*******************************************************************************
** patch_deinit()
*******************************************************************************/
short int patch_deinit(patch* p)
{
  if (p == NULL)
    return 1;

  return 0;
}

/*******************************************************************************
** patch_destroy()
*******************************************************************************/
short int patch_destroy(patch* p)
{
  if (p == NULL)
    return 1;

  patch_deinit(p);
  free(p);

  return 0;
}

