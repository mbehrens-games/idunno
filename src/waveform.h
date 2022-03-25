/*******************************************************************************
** waveform.h (oscillator wavetables)
*******************************************************************************/

#ifndef WAVEFORM_H
#define WAVEFORM_H

#define DB_STEP 0.01171875

enum
{
  OSC_WAVEFORM_SQUARE,
  OSC_WAVEFORM_SAW,
  OSC_WAVEFORM_TRIANGLE,
  OSC_WAVEFORM_PULSE_1_8,
  OSC_WAVEFORM_PULSE_1_4,
  OSC_WAVEFORM_PULSE_3_8,
  OSC_WAVEFORM_PULSE_1_16,
  OSC_WAVEFORM_PULSE_3_16,
  OSC_WAVEFORM_PULSE_5_16,
  OSC_WAVEFORM_PULSE_7_16
};

/* function declarations */
short int waveform_generate_tables();

short int waveform_wave_lookup( int type, int phase, 
                                int wave_mix, int noise_mix, int env_index);
short int waveform_ringmod_lookup(int type_1, int phase_1, 
                                  int type_2, int phase_2, 
                                  int wave_mix, int noise_mix, int env_index);
short int waveform_noise_lookup(int lfsr, int noise_mix, int env_index);

#endif
