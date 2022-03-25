/*******************************************************************************
** lfo.c (vibrato / tremolo)
*******************************************************************************/

#include <stdio.h>    /* testing */
#include <stdlib.h>
#include <math.h>

#include "clock.h"
#include "global.h"
#include "lfo.h"

/* vibrato / tremolo / wobble tables  */
/* types: 4 (sine, square, saw, tri)  */
/* rows: 16 depths                    */
/* index: 128 steps in the wave       */
static short int  S_vib_table[4][16][128];
static short int  S_trem_table[4][16][128];
static short int  S_wob_table[4][16][128];

/* lfo period table                               */
/* the frequency is f = 53267/(128 * period)      */
/* some of the periods match those on the ym2612  */
/* (as listed in fm2612.cpp in mame)              */

/* period 0:  864 -> 0.48 */
/* period 1:  432 -> 0.96 */
/* period 2:  216 -> 1.93 */
/* period 3:  108 -> 3.85 */
/* period 4:   77 -> 5.40 */
/* period 5:   71 -> 5.86 */
/* period 6:   67 -> 6.21 */
/* period 7:   62 -> 6.71 */
/* period 8:   52 -> 8.00 */
/* period 9:   44 -> 9.46 */
/* period 10:  32 -> 13.0 */
/* period 11:  28 -> 14.9 */
/* period 12:  24 -> 17.3 */
/* period 13:  16 -> 26.0 */
/* period 14:   8 -> 52.0 */
/* period 15:   5 -> 83.2 */
static short int  S_lfo_period_table[16] = {864, 432, 216, 108, 
                                            77,  71,  67,  62, 
                                            52,  44,  32,  28, 
                                            24,  16,  8,   5};

/*******************************************************************************
** lfo_init()
*******************************************************************************/
short int lfo_init(lfo* l)
{
  if (l == NULL)
    return 1;

  l->type     = LFO_TYPE_VIBRATO;
  l->waveform = LFO_WAVEFORM_SINE;

  l->cycles   = 0;
  l->period   = 0;
  l->padding  = 0;

  l->index    = 0;
  l->depth    = 0;

  l->lfsr     = 0x0001;

  l->level    = 0;

  return 0;
}

/*******************************************************************************
** lfo_create()
*******************************************************************************/
lfo* lfo_create()
{
  lfo* l;

  l = malloc(sizeof(lfo));
  lfo_init(l);

  return l;
}

/*******************************************************************************
** lfo_deinit()
*******************************************************************************/
short int lfo_deinit(lfo* l)
{
  if (l == NULL)
    return 1;

  return 0;
}

/*******************************************************************************
** lfo_destroy()
*******************************************************************************/
short int lfo_destroy(lfo* l)
{
  if (l == NULL)
    return 1;

  lfo_deinit(l);
  free(l);

  return 0;
}

/*******************************************************************************
** lfo_setup()
*******************************************************************************/
short int lfo_setup(lfo* l, int waveform,
                            unsigned char speed, 
                            unsigned char depth, 
                            unsigned char delay)
{
  if (l == NULL)
    return 1;

  l->lfsr = 0x0001;

  /* set waveform */
  l->waveform = waveform;

  /* set period */
  if ((speed >= 0) && (speed <= 15))
    l->period = S_lfo_period_table[speed];
  else
    l->period = S_lfo_period_table[0];

  /* set depth */
  if ((depth >= 0) && (depth <= 15))
    l->depth = depth;
  else
    l->depth = 0;

  /* set padding amount (in multiples of approximately 1/8 of a second) */
  if ((delay >= 0) && (delay <= 7))
    l->padding = (delay * 53267) / 8;
  else
    l->padding = 0;

  return 0;
}

/*******************************************************************************
** lfo_update()
*******************************************************************************/
short int lfo_update(lfo* l)
{
  if (l == NULL)
    return 1;

  /* increment cycles */
  l->cycles++;

  /* check if currently in initial delay state */
  if (l->cycles < 0)
  {
    l->level = 0;
    return 0;
  }

  /* update table index */
  if (l->cycles >= l->period)
  {
    /* noise waveform: use lfsr to determine table index */
    if (l->waveform == LFO_WAVEFORM_NOISE)
    {
      /* 15-bit lfsr, taps on 1 and 2 (nes) */
      if ((l->lfsr & 0x0001) ^ ((l->lfsr & 0x0002) >> 1))
        l->lfsr = ((l->lfsr >> 1) & 0x3FFF) | 0x4000;
      else
        l->lfsr = (l->lfsr >> 1) & 0x3FFF;

      /* determine index in square wave table */
      if (l->lfsr & 0x0001)
        l->index = 64;
      else
        l->index = 0;
    }
    /* other waveforms: increment table index */
    else
      l->index = (l->index + 1) % 128;

    l->cycles -= l->period;
  }

  /* update level */
  if (l->type == LFO_TYPE_VIBRATO)
  {
    if (l->waveform == LFO_WAVEFORM_SINE)
      l->level = S_vib_table[0][l->depth][l->index];
    else if (l->waveform == LFO_WAVEFORM_SQUARE)
      l->level = S_vib_table[1][l->depth][l->index];
    else if (l->waveform == LFO_WAVEFORM_TRIANGLE)
      l->level = S_vib_table[3][l->depth][l->index];
    else if (l->waveform == LFO_WAVEFORM_SAW_UP)
      l->level = S_vib_table[2][l->depth][l->index];
    else if (l->waveform == LFO_WAVEFORM_SAW_DOWN)
      l->level = S_vib_table[2][l->depth][127 - l->index];
    else if (l->waveform == LFO_WAVEFORM_NOISE)
      l->level = S_vib_table[1][l->depth][l->index];
    else
      l->level = 0;
  }
  else if (l->type == LFO_TYPE_TREMOLO)
  {
    if (l->waveform == LFO_WAVEFORM_SINE)
      l->level = S_trem_table[0][l->depth][l->index];
    else if (l->waveform == LFO_WAVEFORM_SQUARE)
      l->level = S_trem_table[1][l->depth][l->index];
    else if (l->waveform == LFO_WAVEFORM_TRIANGLE)
      l->level = S_trem_table[3][l->depth][l->index];
    else if (l->waveform == LFO_WAVEFORM_SAW_UP)
      l->level = S_trem_table[2][l->depth][l->index];
    else if (l->waveform == LFO_WAVEFORM_SAW_DOWN)
      l->level = S_trem_table[2][l->depth][127 - l->index];
    else if (l->waveform == LFO_WAVEFORM_NOISE)
      l->level = S_trem_table[1][l->depth][l->index];
    else
      l->level = 0;
  }
  else if (l->type == LFO_TYPE_WOBBLE)
  {
    if (l->waveform == LFO_WAVEFORM_SINE)
      l->level = S_wob_table[0][l->depth][l->index];
    else if (l->waveform == LFO_WAVEFORM_SQUARE)
      l->level = S_wob_table[1][l->depth][l->index];
    else if (l->waveform == LFO_WAVEFORM_TRIANGLE)
      l->level = S_wob_table[3][l->depth][l->index];
    else if (l->waveform == LFO_WAVEFORM_SAW_UP)
      l->level = S_wob_table[2][l->depth][l->index];
    else if (l->waveform == LFO_WAVEFORM_SAW_DOWN)
      l->level = S_wob_table[2][l->depth][127 - l->index];
    else if (l->waveform == LFO_WAVEFORM_NOISE)
      l->level = S_wob_table[1][l->depth][l->index];
    else
      l->level = 0;
  }

  return 0;
}

/*******************************************************************************
** lfo_generate_tables()
*******************************************************************************/
short int lfo_generate_tables()
{
  int i;
  int k;

  /* vibrato tables */

  /* depth  0: amplitude is   0 */
  /* depth  1: amplitude is   2 */
  /* depth  2: amplitude is   4 */
  /* depth  3: amplitude is   6 */
  /* depth  4: amplitude is   8 */
  /* depth  5: amplitude is  10 */
  /* depth  6: amplitude is  12 */
  /* depth  7: amplitude is  16 */
  /* depth  8: amplitude is  20 */
  /* depth  9: amplitude is  24 */
  /* depth 10: amplitude is  28 */
  /* depth 11: amplitude is  32 */
  /* depth 12: amplitude is  40 */
  /* depth 13: amplitude is  48 */
  /* depth 14: amplitude is  56 */
  /* depth 15: amplitude is  64 */

  /* generate vibrato waves at depth 15 */
  for (k = 0; k < 128; k++)
  {
    /* sine */
    S_vib_table[0][15][k] = (short int) ((sin(TWO_PI * k / 128.0f) * 64) + 0.5f);
  }

  for (k = 0; k < 64; k++)
  {
    /* square */
    S_vib_table[1][15][k] = 64;
    S_vib_table[1][15][k + 64] = -64;

    /* saw */
    S_vib_table[2][15][k] = k;
    S_vib_table[2][15][k + 64] = k - 64;
  }

  for (k = 0; k < 32; k++)
  {
    /* triangle */
    S_vib_table[3][15][k] = 2 * k;
    S_vib_table[3][15][k + 32] = 64 - (2 * k);
    S_vib_table[3][15][k + 64] = -(2 * k);
    S_vib_table[3][15][k + 96] = -(64 - (2 * k));
  }

  /* generate waves at other depths */
  for (i = 0; i < 4; i++)
  {
    for (k = 0; k < 128; k++)
    {
      S_vib_table[i][0][k] = 0;

      S_vib_table[i][1][k] = S_vib_table[i][15][k] / 32;
      S_vib_table[i][2][k] = S_vib_table[i][15][k] / 16;
      S_vib_table[i][3][k] = (3 * S_vib_table[i][15][k]) / 32;
      S_vib_table[i][4][k] = S_vib_table[i][15][k] / 8;
      S_vib_table[i][5][k] = (5 * S_vib_table[i][15][k]) / 32;
      S_vib_table[i][6][k] = (3 * S_vib_table[i][15][k]) / 16;
      S_vib_table[i][7][k] = S_vib_table[i][15][k] / 4;
      S_vib_table[i][8][k] = (5 * S_vib_table[i][15][k]) / 16;
      S_vib_table[i][9][k] = (3 * S_vib_table[i][15][k]) / 8;
      S_vib_table[i][10][k] = (7 * S_vib_table[i][15][k]) / 16;
      S_vib_table[i][11][k] = S_vib_table[i][15][k] / 2;
      S_vib_table[i][12][k] = (5 * S_vib_table[i][15][k]) / 8;
      S_vib_table[i][13][k] = (3 * S_vib_table[i][15][k]) / 4;
      S_vib_table[i][14][k] = (7 * S_vib_table[i][15][k]) / 8;
    }
  }

  /* tremolo tables */

  /* depth  0: amplitude is   0 */
  /* depth  1: amplitude is  16 */
  /* depth  2: amplitude is  24 */
  /* depth  3: amplitude is  32 */
  /* depth  4: amplitude is  40 */
  /* depth  5: amplitude is  48 */
  /* depth  6: amplitude is  56 */
  /* depth  7: amplitude is  64 */
  /* depth  8: amplitude is  72 */
  /* depth  9: amplitude is  80 */
  /* depth 10: amplitude is  88 */
  /* depth 11: amplitude is  96 */
  /* depth 12: amplitude is 104 */
  /* depth 13: amplitude is 112 */
  /* depth 14: amplitude is 120 */
  /* depth 15: amplitude is 128 */

  /* generate waves at depth 15 */
  for (k = 0; k < 64; k++)
  {
    /* square */
    S_trem_table[1][15][k] = 0;
    S_trem_table[1][15][k + 64] = 128;

    /* triangle */
    S_trem_table[3][15][k] = 2 * k;
    S_trem_table[3][15][k + 64] = 128 - (2 * k);
  }

  for (k = 0; k < 128; k++)
  {
    /* sine */
    S_trem_table[0][15][k] = (short int) (((1.0f - cos(TWO_PI * k / 128.0f)) * 64) + 0.5f);

    /* saw */
    S_trem_table[2][15][k] = k;
  }

  /* generate waves at other depths */
  for (i = 0; i < 4; i++)
  {
    for (k = 0; k < 128; k++)
    {
      S_trem_table[i][0][k] = 0;

      S_trem_table[i][1][k] = S_trem_table[i][15][k] / 8;
      S_trem_table[i][2][k] = (3 * S_trem_table[i][15][k]) / 16;
      S_trem_table[i][3][k] = S_trem_table[i][15][k] / 4;
      S_trem_table[i][4][k] = (5 * S_trem_table[i][15][k]) / 16;
      S_trem_table[i][5][k] = (3 * S_trem_table[i][15][k]) / 8;
      S_trem_table[i][6][k] = (7 * S_trem_table[i][15][k]) / 16;
      S_trem_table[i][7][k] = S_trem_table[i][15][k] / 2;
      S_trem_table[i][8][k] = (9 * S_trem_table[i][15][k]) / 16;
      S_trem_table[i][9][k] = (5 * S_trem_table[i][15][k]) / 8;
      S_trem_table[i][10][k] = (11 * S_trem_table[i][15][k]) / 16;
      S_trem_table[i][11][k] = (3 * S_trem_table[i][15][k]) / 4;
      S_trem_table[i][12][k] = (13 * S_trem_table[i][15][k]) / 16;
      S_trem_table[i][13][k] = (7 * S_trem_table[i][15][k]) / 8;
      S_trem_table[i][14][k] = (15 * S_trem_table[i][15][k]) / 16;
    }
  }

  /* wobble tables */

  /* depth  0: amplitude is   0 */
  /* depth  1: amplitude is  32 */
  /* depth  2: amplitude is  48 */
  /* depth  3: amplitude is  64 */
  /* depth  4: amplitude is  80 */
  /* depth  5: amplitude is  96 */
  /* depth  6: amplitude is 112 */
  /* depth  7: amplitude is 128 */
  /* depth  8: amplitude is 144 */
  /* depth  9: amplitude is 160 */
  /* depth 10: amplitude is 176 */
  /* depth 11: amplitude is 192 */
  /* depth 12: amplitude is 208 */
  /* depth 13: amplitude is 224 */
  /* depth 14: amplitude is 240 */
  /* depth 15: amplitude is 256 */

  /* generate wobble waves at depth 15 */
  for (k = 0; k < 128; k++)
  {
    /* sine */
    S_wob_table[0][15][k] = (short int) ((sin(TWO_PI * k / 128.0f) * 256) + 0.5f);
  }

  for (k = 0; k < 64; k++)
  {
    /* square */
    S_wob_table[1][15][k] = 256;
    S_wob_table[1][15][k + 64] = -256;

    /* saw */
    S_wob_table[2][15][k] = 4 * k;
    S_wob_table[2][15][k + 64] = (4 * k) - 256;
  }

  for (k = 0; k < 32; k++)
  {
    /* triangle */
    S_wob_table[3][15][k] = 8 * k;
    S_wob_table[3][15][k + 32] = 256 - (8 * k);
    S_wob_table[3][15][k + 64] = -(8 * k);
    S_wob_table[3][15][k + 96] = -(256 - (8 * k));
  }

  /* generate waves at other depths */
  for (i = 0; i < 4; i++)
  {
    for (k = 0; k < 128; k++)
    {
      S_wob_table[i][0][k] = 0;

      S_wob_table[i][1][k] = S_wob_table[i][15][k] / 8;
      S_wob_table[i][2][k] = (3 * S_wob_table[i][15][k]) / 16;
      S_wob_table[i][3][k] = S_wob_table[i][15][k] / 4;
      S_wob_table[i][4][k] = (5 * S_wob_table[i][15][k]) / 16;
      S_wob_table[i][5][k] = (3 * S_wob_table[i][15][k]) / 8;
      S_wob_table[i][6][k] = (7 * S_wob_table[i][15][k]) / 16;
      S_wob_table[i][7][k] = S_wob_table[i][15][k] / 2;
      S_wob_table[i][8][k] = (9 * S_wob_table[i][15][k]) / 16;
      S_wob_table[i][9][k] = (5 * S_wob_table[i][15][k]) / 8;
      S_wob_table[i][10][k] = (11 * S_wob_table[i][15][k]) / 16;
      S_wob_table[i][11][k] = (3 * S_wob_table[i][15][k]) / 4;
      S_wob_table[i][12][k] = (13 * S_wob_table[i][15][k]) / 16;
      S_wob_table[i][13][k] = (7 * S_wob_table[i][15][k]) / 8;
      S_wob_table[i][14][k] = (15 * S_wob_table[i][15][k]) / 16;
    }
  }

  /* testing: print tables */
#if 0
  printf("Vibrato Sine Table (Depth 15):\n");

  for (j = 0; j < 128; j++)
    printf("  %d\n", S_vib_table[0][15][j]);
#endif

#if 0
  printf("Vibrato Square Table (Depth 15):\n");

  for (j = 0; j < 128; j++)
    printf("  %d\n", S_vib_table[1][15][j]);
#endif

#if 0
  printf("Vibrato Triangle Table (Depth 15):\n");

  for (j = 0; j < 128; j++)
    printf("  %d\n", S_vib_table[3][15][j]);
#endif

#if 0
  printf("Vibrato Saw Table (Depth 15):\n");

  for (j = 0; j < 128; j++)
    printf("  %d\n", S_vib_table[2][15][j]);
#endif

#if 0
  printf("Tremolo Sine Table (Depth 15):\n");

  for (j = 0; j < 128; j++)
    printf("  %d\n", S_trem_table[0][15][j]);
#endif

#if 0
  printf("Tremolo Square Table (Depth 15):\n");

  for (j = 0; j < 128; j++)
    printf("  %d\n", S_trem_table[1][15][j]);
#endif

#if 0
  printf("Tremolo Triangle Table (Depth 15):\n");

  for (j = 0; j < 128; j++)
    printf("  %d\n", S_trem_table[3][15][j]);
#endif

#if 0
  printf("Tremolo Saw Table (Depth 15):\n");

  for (j = 0; j < 128; j++)
    printf("  %d\n", S_trem_table[2][15][j]);
#endif

#if 0
  printf("Wobble Sine Table (Depth 15):\n");

  for (j = 0; j < 128; j++)
    printf("  %d\n", S_wob_table[0][15][j]);
#endif

#if 0
  printf("Wobble Square Table (Depth 15):\n");

  for (j = 0; j < 128; j++)
    printf("  %d\n", S_wob_table[1][15][j]);
#endif

#if 0
  printf("Wobble Triangle Table (Depth 15):\n");

  for (j = 0; j < 128; j++)
    printf("  %d\n", S_wob_table[3][15][j]);
#endif

#if 0
  printf("Wobble Saw Table (Depth 15):\n");

  for (j = 0; j < 128; j++)
    printf("  %d\n", S_wob_table[2][15][j]);
#endif

  return 0;
}
