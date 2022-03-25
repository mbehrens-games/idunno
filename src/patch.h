/*******************************************************************************
** patch.h (patch settings)
*******************************************************************************/

#ifndef PATCH_H
#define PATCH_H

#define PATCH_NUM_WAVES     2 /* waves                */
#define PATCH_NUM_OSCS      3 /* waves + sync         */
#define PATCH_NUM_PHASES    4 /* waves + sync + noise */

#define PATCH_NUM_ENVELOPES 2
#define PATCH_NUM_LFOS      3

typedef struct patch
{
  /* waveform generators */
  int             waveform[PATCH_NUM_WAVES];
  unsigned char   phi;
  char            sync;
  char            wave_mix;
  char            ring_mod;

  /* oscillators */
  char            detune_octave[PATCH_NUM_OSCS];
  char            detune_coarse[PATCH_NUM_OSCS];
  char            detune_fine[PATCH_NUM_OSCS];

  /* noise generator */
  unsigned char   noise_period;
  char            noise_mix;

  /* lowpass filter */
  char            cutoff;
  char            keytrack;
  char            resonance;

  /* reverb */
  char            rev_delay;
  char            rev_c[8];
  char            rev_feedback;
  char            rev_vol;

  /* envelopes */
  unsigned char   ar[PATCH_NUM_ENVELOPES]; /* 0-31  */
  unsigned char   dr[PATCH_NUM_ENVELOPES]; /* 0-31  */
  unsigned char   sr[PATCH_NUM_ENVELOPES]; /* 0-31  */
  unsigned char   rr[PATCH_NUM_ENVELOPES]; /* 0-15  */
  unsigned char   tl[PATCH_NUM_ENVELOPES]; /* 0-127 */
  unsigned char   sl[PATCH_NUM_ENVELOPES]; /* 0-15  */

  int             rks[PATCH_NUM_ENVELOPES];
  int             lks[PATCH_NUM_ENVELOPES];

  /* lfos */
  int             mod_waveform[PATCH_NUM_LFOS];
  unsigned char   mod_speed[PATCH_NUM_LFOS];
  unsigned char   mod_depth[PATCH_NUM_LFOS];
  unsigned char   mod_delay[PATCH_NUM_LFOS];

  /* highpass filter, soft clipping */
  char            hpf;
  char            soft_clip;
} patch;

/* function declarations */
short int patch_init(patch* p);
patch*    patch_create();
short int patch_deinit(patch* p);
short int patch_destroy(patch* p);

#endif
