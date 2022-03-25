/*******************************************************************************
** lfo.h (vibrato / tremolo)
*******************************************************************************/

#ifndef LFO_H
#define LFO_H

enum
{
  LFO_TYPE_VIBRATO,
  LFO_TYPE_TREMOLO,
  LFO_TYPE_WOBBLE
};

enum
{
  LFO_WAVEFORM_SINE,
  LFO_WAVEFORM_SQUARE,
  LFO_WAVEFORM_TRIANGLE,
  LFO_WAVEFORM_SAW_UP,
  LFO_WAVEFORM_SAW_DOWN,
  LFO_WAVEFORM_NOISE
};

typedef struct lfo
{
  /* type, waveform */
  int           type;
  int           waveform;

  /* cycle counter, period, initial delay */
  int           cycles;
  int           period;
  int           padding;

  /* table index, depth */
  unsigned char index;
  unsigned char depth;

  /* noise generator */
  unsigned int  lfsr;

  /* level */
  short int     level;
} lfo;

/* function declarations */
short int lfo_init(lfo* l);
lfo*      lfo_create();
short int lfo_deinit(lfo* l);
short int lfo_destroy(lfo* l);

short int lfo_setup(lfo* l, int waveform,
                            unsigned char speed, 
                            unsigned char depth, 
                            unsigned char delay);
short int lfo_update(lfo* l);

short int lfo_generate_tables();

#endif
