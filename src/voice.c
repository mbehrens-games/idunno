/*******************************************************************************
** voice.c (synth voice)
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "clock.h"
#include "envelope.h"
#include "filter.h"
#include "global.h"
#include "lfo.h"
#include "patch.h"
#include "tuning.h"
#include "voice.h"
#include "waveform.h"

/* phase offset table                                             */
/* entries are: 0, PI/4, PI/2, 3*PI/4, PI, 5*PI/4, 3*PI/2, 7*PI/4 */
static int  S_phi_table[8] = 
            { 0x00000000, 0x02000000, 0x04000000, 0x06000000,
              0x08000000, 0x0A000000, 0x0C000000, 0x0E000000};

/* noise period table - periods stored as midi note numbers     */
/* 0: A3, 1: D4,  2: A4,  3: D5,  4: A5,  5: D6,  6: A6,  7: C7 */
/* 8: F7, 9: A7, 10: C8, 11: F8, 12: A8, 13: C9, 14: E9, 15: G9 */
static char S_noise_period_table[16] = 
            {  57,  62,  69,  74,  81,  86,  93,  96, 
              101, 105, 108, 113, 117, 120, 124, 127};

/*******************************************************************************
** voice_init()
*******************************************************************************/
short int voice_init(voice* v)
{
  int i;

  if (v == NULL)
    return 1;

  /* patch */
  v->p = NULL;

  /* base pitch table indices, phases */
  for (i = 0; i < PATCH_NUM_PHASES; i++)
  {
    v->base_pitch_index[i] = 0;
    v->phase[i] = 0;
  }

  /* noise generator */
  v->lfsr = 0x0001;

  /* filter indices */
  v->base_fc_index = 0;

  /* filter */
  filter_init(&v->lowpass);

  /* envelopes */
  for (i = 0; i < PATCH_NUM_ENVELOPES; i++)
    envelope_init(&v->env[i]);

  /* lfos */
  for (i = 0; i < PATCH_NUM_LFOS; i++)
    lfo_init(&v->mod[i]);

  /* volume level */
  v->volume = 0;

  /* output level */
  v->level = 0;

  return 0;
}

/*******************************************************************************
** voice_create()
*******************************************************************************/
voice* voice_create()
{
  voice* v;

  v = malloc(sizeof(voice));
  voice_init(v);

  return v;
}

/*******************************************************************************
** voice_deinit()
*******************************************************************************/
short int voice_deinit(voice* v)
{
  int i;

  if (v == NULL)
    return 1;

  v->p = NULL;

  filter_deinit(&v->lowpass);

  for (i = 0; i < PATCH_NUM_ENVELOPES; i++)
    envelope_deinit(&v->env[i]);

  for (i = 0; i < PATCH_NUM_LFOS; i++)
    lfo_deinit(&v->mod[i]);

  return 0;
}

/*******************************************************************************
** voice_destroy()
*******************************************************************************/
short int voice_destroy(voice* v)
{
  if (v == NULL)
    return 1;

  voice_deinit(v);
  free(v);

  return 0;
}

/*******************************************************************************
** voice_key_on()
*******************************************************************************/
short int voice_key_on(voice* v, char note, char volume)
{
  int i;

  patch* p;

  if (v == NULL)
    return 1;

  /* set patch pointer */
  p = v->p;

  if (p == NULL)
    return 1;

  /* if note is out of the range A0 to C8, ignore */
  if ((note < 21) || (note > 108))
    return 0;

  /* if volume is out of range, ignore */
  if ((volume < 0) || (volume > 127))
    return 0;

  /* set oscillator base pitch indices */
  for (i = 0; i < 3; i++)
  {
    v->base_pitch_index[i] = note * 32;

    /* apply octave detune */
    if (p->detune_octave[i] == 3)
      v->base_pitch_index[i] -= 12 * 32;
    else if (p->detune_octave[i] == 1)
      v->base_pitch_index[i] += 12 * 32;

    /* apply coarse detune */
    if ((p->detune_coarse[i] >= 1) && (p->detune_coarse[i] <= 7))
      v->base_pitch_index[i] += p->detune_coarse[i] * 32;
    else if ((p->detune_coarse[i] >= 9) && (p->detune_coarse[i] <= 15))
      v->base_pitch_index[i] += (8 - p->detune_coarse[i]) * 32;

    /* apply fine detune */
    if ((p->detune_fine[i] >= 1) && (p->detune_fine[i] <= 31))
      v->base_pitch_index[i] += p->detune_fine[i];
    else if ((p->detune_fine[i] >= 33) && (p->detune_fine[i] <= 63))
      v->base_pitch_index[i] += 32 - p->detune_fine[i];

    /* bound base pitch index */
    if (v->base_pitch_index[i] < 0)
      v->base_pitch_index[i] = 0;
    else if (v->base_pitch_index[i] > 4095)
      v->base_pitch_index[i] = 4095;
  }

  /* set noise base pitch index */
  v->base_pitch_index[3] = S_noise_period_table[p->noise_period] * 32;

  /* set base filter cutoff index */
  if ((p->keytrack == 0) || (p->keytrack == 4))
    v->base_fc_index = p->cutoff * 32;
  else if (p->keytrack == 1)
    v->base_fc_index = (note * 32) + (((p->cutoff - 60) * 32) / 3);
  else if (p->keytrack == 2)
    v->base_fc_index = (note * 32) + (2 * ((p->cutoff - 60) * 32) / 3);
  else if (p->keytrack == 3)
    v->base_fc_index = (note * 32) + ((p->cutoff - 60) * 32);
  else if (p->keytrack == 5)
    v->base_fc_index = (note * 32) - (((p->cutoff - 60) * 32) / 3);
  else if (p->keytrack == 6)
    v->base_fc_index = (note * 32) - (2 * ((p->cutoff - 60) * 32) / 3);
  else if (p->keytrack == 7)
    v->base_fc_index = (note * 32) - ((p->cutoff - 60) * 32);

  if (v->base_fc_index < 0)
    v->base_fc_index = 0;
  else if (v->base_fc_index > 4095)
    v->base_fc_index = 4095;

  /* set filter coefficients */
  filter_set_indices(&v->lowpass, v->base_fc_index, p->resonance);

  /* set up envelopes */
  for (i = 0; i < PATCH_NUM_ENVELOPES; i++)
  {
    envelope_setup( &v->env[i], 
                    p->ar[i], p->dr[i], 
                    p->sr[i], p->rr[i], 
                    p->sl[i], p->tl[i], 
                    p->rks[i], p->lks[i], 
                    note);
  }

  /* setup lfos */
  v->mod[0].type = LFO_TYPE_VIBRATO;
  v->mod[1].type = LFO_TYPE_TREMOLO;
  v->mod[2].type = LFO_TYPE_WOBBLE;

  for (i = 0; i < PATCH_NUM_LFOS; i++)
  {
    lfo_setup(&v->mod[i], p->mod_waveform[i], 
                          p->mod_speed[i], 
                          p->mod_depth[i], 
                          p->mod_delay[i]);
  }

  /* reset phases */
  for (i = 0; i < PATCH_NUM_PHASES; i++)
    v->phase[i] = 0;

  /* apply phi (phase shift) */
  if ((p->phi >= 1) && (p->phi <= 7))
    v->phase[1] = S_phi_table[p->phi];

  /* reset noise lfsr */
  v->lfsr = 0x0001;

  /* reset lfo phases */
  for (i = 0; i < PATCH_NUM_LFOS; i++)
  {
    v->mod[i].cycles = -v->mod[i].padding;
    v->mod[i].index = 0;
    v->mod[i].level = 0;
    v->mod[i].lfsr = 0x0001;
  }

  /* set voice volume */
  if ((volume >= 0) && (volume <= 127))
    v->volume = volume;
  else
    v->volume = 127;

  /* trigger envelopes */
  for (i = 0; i < PATCH_NUM_ENVELOPES; i++)
    envelope_change_state(&v->env[i], ENVELOPE_STATE_ATTACK);

  return 0;
}

/*******************************************************************************
** voice_key_off()
*******************************************************************************/
short int voice_key_off(voice* v)
{
  int i;

  if (v == NULL)
    return 1;

  /* release envelopes */
  for (i = 0; i < PATCH_NUM_ENVELOPES; i++)
    envelope_change_state(&v->env[i], ENVELOPE_STATE_RELEASE);

  return 0;
}

/*******************************************************************************
** voice_update()
*******************************************************************************/
short int voice_update(voice* v)
{
  int       i;

  patch*    p;

  short int env_index[PATCH_NUM_ENVELOPES];

  int       pitch_offset;
  int       current_pitch_index;

  int       fc_offset;
  int       current_fc_index;

  int       level;

  if (v == NULL)
    return 1;

  /* set patch pointer */
  p = v->p;

  if (p == NULL)
    return 1;

  /* update lfos */
  for (i = 0; i < PATCH_NUM_LFOS; i++)
    lfo_update(&v->mod[i]);

  /* update amplitude envelope */
  envelope_update(&v->env[0]);

  env_index[0] = v->env[0].attenuation;
  env_index[0] += v->env[0].total_bound;
  env_index[0] += v->mod[1].level;

  if (env_index[0] > 1023)
    env_index[0] = 1023;

  env_index[0] = env_index[0] << 2;

  /* update filter envelope */
  envelope_update(&v->env[1]);

  env_index[1] = v->env[1].attenuation;
  env_index[1] += v->env[1].total_bound;

  if (env_index[1] > 1023)
    env_index[1] = 1023;

  /* compute pitch offset (vibrato) */
  pitch_offset = v->mod[0].level;

  /* update wave generators */
  current_pitch_index = v->base_pitch_index[0] + pitch_offset;

  if (current_pitch_index < 0)
    v->phase[0] += G_phase_increment_table[0];
  else if (current_pitch_index > 4095)
    v->phase[0] += G_phase_increment_table[4095];
  else
    v->phase[0] += G_phase_increment_table[current_pitch_index];

  v->phase[0] &= 0xFFFFFFF;

  current_pitch_index = v->base_pitch_index[1] + pitch_offset;

  if (current_pitch_index < 0)
    v->phase[1] += G_phase_increment_table[0];
  else if (current_pitch_index > 4095)
    v->phase[1] += G_phase_increment_table[4095];
  else
    v->phase[1] += G_phase_increment_table[current_pitch_index];

  v->phase[1] &= 0xFFFFFFF;

  /* update sync generator */
  current_pitch_index = v->base_pitch_index[2] + pitch_offset;

  if (current_pitch_index < 0)
    v->phase[2] += G_phase_increment_table[0];
  else if (current_pitch_index > 4095)
    v->phase[2] += G_phase_increment_table[4095];
  else
    v->phase[2] += G_phase_increment_table[current_pitch_index];

  if (v->phase[2] > 0xFFFFFFF)
  {
    v->phase[2] &= 0xFFFFFFF;

    if ((p->sync == 1) || (p->sync == 3))
      v->phase[0] = v->phase[2];

    if ((p->sync == 2) || (p->sync == 3))
    {
      if ((p->phi >= 1) && (p->phi <= 7))
        v->phase[1] = v->phase[2] + S_phi_table[p->phi];
      else
        v->phase[1] = v->phase[2];

      v->phase[1] &= 0xFFFFFFF;
    }
  }

  /* update noise generator (nes) */
  /* 15-bit lfsr, taps on 1 and 2 */
  current_pitch_index = v->base_pitch_index[3];

  if (current_pitch_index < 0)
    v->phase[3] += G_phase_increment_table[0];
  else if (current_pitch_index > 4095)
    v->phase[3] += G_phase_increment_table[4095];
  else
    v->phase[3] += G_phase_increment_table[current_pitch_index];

  if (v->phase[3] > 0xFFFFFFF)
  {
    if ((v->lfsr & 0x0001) ^ ((v->lfsr & 0x0002) >> 1))
      v->lfsr = ((v->lfsr >> 1) & 0x3FFF) | 0x4000;
    else
      v->lfsr = (v->lfsr >> 1) & 0x3FFF;

    v->phase[3] &= 0xFFFFFFF;
  }

  /* compute level */
  level = 0;

  /* additive wave mixing */
  if (p->ring_mod == 0)
  {
    level += 
      waveform_wave_lookup( p->waveform[0], (v->phase[0] >> 18), 
                            (32 - p->wave_mix), (32 - p->noise_mix), 
                            env_index[0]);

    level += 
      waveform_wave_lookup( p->waveform[1], (v->phase[1] >> 18), 
                            p->wave_mix, (32 - p->noise_mix), 
                            env_index[0]);
  }
  /* multiplicative wave mixing */
  else if (p->ring_mod == 1)
  {
    level +=
      waveform_ringmod_lookup(p->waveform[0], (v->phase[0] >> 18), 
                              p->waveform[1], (v->phase[1] >> 18), 
                              p->wave_mix, (32 - p->noise_mix), 
                              env_index[0]);
  }

  /* mix in noise */
  level +=
    waveform_noise_lookup(v->lfsr, p->noise_mix, env_index[0]);

  /* determine current filter cutoff frequency                    */
  /* filter envelope scaled so that its max value is 19 semitones */
  /* thus, at C8, the max value reaches the highest midi note G9  */
  fc_offset = (19 * (1023 - env_index[1])) / 32;
  fc_offset += v->mod[2].level;

  current_fc_index = v->base_fc_index + fc_offset;

  if (current_fc_index < 0)
    current_fc_index = 0;
  else if (current_fc_index > 4095)
    current_fc_index = 4095;

  if (current_fc_index != v->lowpass.fc_index)
    filter_set_indices(&v->lowpass, current_fc_index, p->resonance);

  /* apply lowpass filter */
  filter_update_lowpass(&v->lowpass, level);

  level = v->lowpass.level;

  /* apply volume */
  level = (level * v->volume) / 128;

  /* set voice level */
  v->level = level;

  return 0;
}

