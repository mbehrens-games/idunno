/*******************************************************************************
** waveform.c (oscillator wavetables)
*******************************************************************************/

#include <stdio.h>
#include <math.h>

#include "waveform.h"

static short int S_db_to_linear[8192];

static short int S_wavetable_saw[1024];
static short int S_wavetable_tri[512];

static short int S_wave_mix_linear[33];

static short int S_geometric_saw[1024];
static short int S_geometric_tri[512];

/*******************************************************************************
** waveform_generate_tables()
*******************************************************************************/
short int waveform_generate_tables()
{
  int     i;
  double  val;

  /* ym2612 - 10 bit envelope (shifted to 12 bit), 12 bit sine, 13 bit sum    */
  /* 10 bit db: 24, 12, 6, 3, 1.5, 0.75, 0.375, 0.1875, 0.09375, 0.046875     */
  /* 12 bit db: adds on 0.0234375, 0.01171875 in back                         */
  /* 13 bit db: adds on 48 in front                                           */

  /* db to linear scale conversion */
  S_db_to_linear[0] = 32767;

  for (i = 1; i < 8191; i++)
  {
    S_db_to_linear[i] = 
      (short int) ((32767.0f * exp(-log(10) * (DB_STEP / 10) * i)) + 0.5f);
  }

  S_db_to_linear[8191] = 0;

  /* wavetable (sawtooth) */
  S_wavetable_saw[0] = 4095;
  S_wavetable_saw[512] = 0;

  for (i = 1; i < 512; i++)
  {
    val = i / 512.0f;
    S_wavetable_saw[i] = (short int) ((10 * (log(1 / val) / log(10)) / DB_STEP) + 0.5f);
    S_wavetable_saw[1024 - i] = S_wavetable_saw[i];
  }

  /* wavetable (triangle) */
  S_wavetable_tri[0] = 4095;
  S_wavetable_tri[256] = 0;

  for (i = 1; i < 256; i++)
  {
    val = i / 256.0f;
    S_wavetable_tri[i] = (short int) ((10 * (log(1 / val) / log(10)) / DB_STEP) + 0.5f);
    S_wavetable_tri[512 - i] = S_wavetable_tri[i];
  }

  /* wave mix (linear weights) */
  S_wave_mix_linear[0] = 4096;

  for (i = 1; i < 32; i++)
  {
    val = i / 32.0f;
    S_wave_mix_linear[i] = (short int) ((10 * (log(1 / val) / log(10)) / DB_STEP) + 0.5f);
  }

  S_wave_mix_linear[32] = 0;

  /* the geometric tables are used to perform wave mixing with ring mod       */
  /* each sample in the original wavetable is raised to the 1/32 power        */
  /* note that the two waveforms are mixed with the expression (x^w1)(y^w2),  */
  /* where w1 and w2 are weights in the form 0/32, 1/32, ..., 32/32.          */

  /* wavetable (sawtooth) - geometric */
  S_geometric_saw[0] = 4095;
  S_geometric_saw[512] = 0;

  for (i = 1; i < 512; i++)
  {
    val = i / 512.0f;
    val = pow(val, 0.03125f);

    S_geometric_saw[i] = (short int) ((10 * (log(1 / val) / log(10)) / DB_STEP) + 0.5f);
    S_geometric_saw[1024 - i] = S_geometric_saw[i];
  }

  /* wavetable (triangle) - geometric */
  S_geometric_tri[0] = 4095;
  S_geometric_tri[256] = 0;

  for (i = 1; i < 256; i++)
  {
    val = i / 256.0f;
    val = pow(val, 0.03125f);

    S_geometric_tri[i] = (short int) ((10 * (log(1 / val) / log(10)) / DB_STEP) + 0.5f);
    S_geometric_tri[512 - i] = S_geometric_tri[i];
  }

  return 0;
}

/*******************************************************************************
** waveform_wave_lookup()
*******************************************************************************/
short int waveform_wave_lookup( int type, int phase, 
                                int wave_mix, int noise_mix, int env_index)
{
  int       att_index;

  int       wave_index;
  char      negative;

  int       final_index;

  short int level;

  /* bound phase */
  phase &= 0x3FF;

  /* determine mix attenuation index */
  if ((wave_mix >= 0) && (wave_mix <= 32))
    att_index = S_wave_mix_linear[wave_mix];
  else
    att_index = 0;

  if ((noise_mix >= 0) && (noise_mix <= 32))
    att_index += S_wave_mix_linear[noise_mix];

  /* determine wave index & negative flag */
  if (type == OSC_WAVEFORM_SQUARE)
  {
    wave_index = 0;

    if (phase < 512)
      negative = 0;
    else
      negative = 1;
  }
  else if (type == OSC_WAVEFORM_SAW)
  {
    wave_index = S_wavetable_saw[phase];

    if (phase < 512)
      negative = 0;
    else
      negative = 1;
  }
  else if (type == OSC_WAVEFORM_TRIANGLE)
  {
    if (phase < 512)
    {
      wave_index = S_wavetable_tri[phase];
      negative = 0;
    }
    else
    {
      wave_index = S_wavetable_tri[phase - 512];
      negative = 1;
    }
  }
  else if (type == OSC_WAVEFORM_PULSE_1_8)
  {
    wave_index = 0;

    if (phase < 128)
      negative = 0;
    else
      negative = 1;
  }
  else if (type == OSC_WAVEFORM_PULSE_1_4)
  {
    wave_index = 0;

    if (phase < 256)
      negative = 0;
    else
      negative = 1;
  }
  else if (type == OSC_WAVEFORM_PULSE_3_8)
  {
    wave_index = 0;

    if (phase < 384)
      negative = 0;
    else
      negative = 1;
  }
  else if (type == OSC_WAVEFORM_PULSE_1_16)
  {
    wave_index = 0;

    if (phase < 64)
      negative = 0;
    else
      negative = 1;
  }
  else if (type == OSC_WAVEFORM_PULSE_3_16)
  {
    wave_index = 0;

    if (phase < 192)
      negative = 0;
    else
      negative = 1;
  }
  else if (type == OSC_WAVEFORM_PULSE_5_16)
  {
    wave_index = 0;

    if (phase < 320)
      negative = 0;
    else
      negative = 1;
  }
  else if (type == OSC_WAVEFORM_PULSE_7_16)
  {
    wave_index = 0;

    if (phase < 448)
      negative = 0;
    else
      negative = 1;
  }
  else
  {
    wave_index = 0;
    negative = 0;
  }

  /* determine attenuated & enveloped wave value */
  final_index = wave_index + att_index + env_index;

  if (final_index < 0)
    final_index = 0;
  else if (final_index > 8191)
    final_index = 8191;

  if (negative == 1)
    level = -S_db_to_linear[final_index];
  else
    level = S_db_to_linear[final_index];

  return level;
}

/*******************************************************************************
** waveform_ringmod_lookup()
*******************************************************************************/
short int waveform_ringmod_lookup(int type_1, int phase_1, 
                                  int type_2, int phase_2, 
                                  int wave_mix, int noise_mix, int env_index)
{
  int       i;

  int       type[2];
  int       phase[2];

  int       att_index;

  int       wave_index[2];
  char      negative;

  int       final_index;

  short int level;

  /* copy type and phase of each wave to local arrays */
  type[0] = type_1;
  type[1] = type_2;

  phase[0] = phase_1;
  phase[1] = phase_2;

  /* determine mix attenuation index */
  if ((noise_mix >= 0) && (noise_mix <= 32))
    att_index = S_wave_mix_linear[noise_mix];
  else
    att_index = 0;

  /* determine wave indices & negative flag */
  negative = 0;

  for (i = 0; i < 2; i++)
  {
    if (type[i] == OSC_WAVEFORM_SQUARE)
    {
      wave_index[i] = 0;

      if (phase[i] < 512)
        negative += 0;
      else
        negative += 1;
    }
    else if (type[i] == OSC_WAVEFORM_SAW)
    {
      wave_index[i] = S_geometric_saw[phase[i]];

      if (phase[i] < 512)
        negative += 0;
      else
        negative += 1;
    }
    else if (type[i] == OSC_WAVEFORM_TRIANGLE)
    {
      if (phase[i] < 512)
      {
        wave_index[i] = S_geometric_tri[phase[i]];
        negative += 0;
      }
      else
      {
        wave_index[i] = S_geometric_tri[phase[i] - 512];
        negative += 1;
      }
    }
    else if (type[i] == OSC_WAVEFORM_PULSE_1_8)
    {
      wave_index[i] = 0;

      if (phase[i] < 128)
        negative += 0;
      else
        negative += 1;
    }
    else if (type[i] == OSC_WAVEFORM_PULSE_1_4)
    {
      wave_index[i] = 0;

      if (phase[i] < 256)
        negative += 0;
      else
        negative += 1;
    }
    else if (type[i] == OSC_WAVEFORM_PULSE_3_8)
    {
      wave_index[i] = 0;

      if (phase[i] < 384)
        negative += 0;
      else
        negative += 1;
    }
    else if (type[i] == OSC_WAVEFORM_PULSE_1_16)
    {
      wave_index[i] = 0;

      if (phase[i] < 64)
        negative += 0;
      else
        negative += 1;
    }
    else if (type[i] == OSC_WAVEFORM_PULSE_3_16)
    {
      wave_index[i] = 0;

      if (phase[i] < 192)
        negative += 0;
      else
        negative += 1;
    }
    else if (type[i] == OSC_WAVEFORM_PULSE_5_16)
    {
      wave_index[i] = 0;

      if (phase[i] < 320)
        negative += 0;
      else
        negative += 1;
    }
    else if (type[i] == OSC_WAVEFORM_PULSE_7_16)
    {
      wave_index[i] = 0;

      if (phase[i] < 448)
        negative += 0;
      else
        negative += 1;
    }
    else
    {
      wave_index[i] = 0;
      negative += 0;
    }
  }

  /* determine attenuated & enveloped value */
  if ((wave_mix >= 0) && (wave_mix <= 32))
  {
    final_index = ((32 - wave_mix) * wave_index[0]) + 
                  (wave_mix * wave_index[1]) + att_index + env_index;
  }
  else
  {
    final_index = 32 * wave_index[0] + att_index + env_index;
  }

  if (final_index < 0)
    final_index = 0;
  else if (final_index > 8191)
    final_index = 8191;

  if (negative == 1)
    level = -S_db_to_linear[final_index];
  else
    level = S_db_to_linear[final_index];

  return level;
}

/*******************************************************************************
** waveform_noise_lookup()
*******************************************************************************/
short int waveform_noise_lookup(int lfsr, int noise_mix, int env_index)
{
  int       att_index;

  int       final_index;

  short int level;

  /* determine mix attenuation index */
  if ((noise_mix >= 0) && (noise_mix <= 32))
    att_index = S_wave_mix_linear[noise_mix];
  else
    att_index = 0;

  /* determine attenuated & enveloped wave value */
  final_index = att_index + env_index;

  if (final_index < 0)
    final_index = 0;
  else if (final_index > 8191)
    final_index = 8191;

  if (lfsr & 0x0001)
    level = -S_db_to_linear[final_index];
  else
    level = S_db_to_linear[final_index];

  return level;
}

