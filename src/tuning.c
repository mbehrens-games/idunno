/*******************************************************************************
** tuning.c (tuning systems)
*******************************************************************************/

#include <math.h>

#include "clock.h"
#include "global.h"
#include "tuning.h"

/* frequency table  */
float G_frequency_table[4096];

/* phase increment table      */
int   G_phase_increment_table[4096];

/* filter coefficient tables  */
float G_filter_omega_0_delta_t_over_2_table[4096];
float G_filter_stage_multiplier_table[4096];

/* filter resonance table */
float G_resonance_table[32];

/* multipliers from c in 12 tone equal temperament */
static float S_tuning_mult_12_et[12] = 
        { 1.0,                  /* C  */
          1.059463094359293,    /* C# */
          1.122462048309375,    /* D  */
          1.189207115002721,    /* D# */
          1.259921049894870,    /* E  */
          1.334839854170037,    /* F  */
          1.414213562373095,    /* F# */
          1.498307076876678,    /* G  */
          1.587401051968203,    /* G# */
          1.681792830507429,    /* A  */
          1.781797436280675,    /* A# */
          1.887748625363391};   /* B  */

/* multipliers from c in pythagorean tuning */
static float S_tuning_mult_pythagorean[12] = 
        { 1.0,                  /* 1:1        C  */
          1.06787109375,        /* 2187:2048  C# */
          1.125,                /* 9:8        D  */
          1.18518518518519,     /* 32:27      Eb */
          1.265625,             /* 81:64      E  */
          1.33333333333333,     /* 4:3        F  */
          1.423828125,          /* 729:512    F# */
          1.5,                  /* 3:2        G  */
          1.601806640625,       /* 6561:4096  G# */
          1.6875,               /* 27:16      A  */
          1.77777777777778,     /* 16:9       Bb */
          1.8984375};           /* 243:128    B  */

/* multipliers from c in quarter comma meantone */
static float S_tuning_mult_qc_meantone[12] = 
        { 1.0,                  /* 1:1        C  */
          1.04490672652566,     /* 5^(7/4)/16 C# */
          1.1180339887499,      /* 5^(1/2):2  D  */
          1.19627902497696,     /* 4:5^(3/4)  Eb */
          1.25,                 /* 5:4        E  */
          1.33748060995284,     /* 2:5^(1/4)  F  */
          1.39754248593737,     /* 5^(3/2):8  F# */
          1.495348781221221,    /* 5^(1/4):1  G  */
          1.5625,               /* 25:16      G# */
          1.67185076244106,     /* 5^(3/4):2  A  */
          1.78885438199984,     /* 4:5^(1/2)  Bb */
          1.86918597652653};    /* 5^(5/4):4  B  */

/* multipliers from c in just intonation */
static float S_tuning_mult_just_intonation[12] = 
        { 1.0,                  /* 1:1        C  */
          1.06666666666667,     /* 16:15      C# */
          1.125,                /* 9:8        D  */
          1.2,                  /* 6:5        Eb */
          1.25,                 /* 5:4        E  */
          1.33333333333333,     /* 4:3        F  */
          1.40625,              /* 45:32      F# */
          1.5,                  /* 3:2        G  */
          1.6,                  /* 8:5        G# */
          1.66666666666667,     /* 5:3        A  */
          1.77777777777778,     /* 16:9       Bb */
          1.875};               /* 15:8       B  */

/* multipliers from c in werckmeister iii */
static float S_tuning_mult_werckmeister_iii[12] = 
        { 1.0,                  /* 1:1              C  */
          1.05349794238683,     /* 256:243          C# */
          1.1174033085417,      /* 64*2^(1/2):81    D  */
          1.18518518518519,     /* 32:27            D# */
          1.25282724872715,     /* 256*2^(1/4):243  E  */
          1.33333333333333,     /* 4:3              F  */
          1.40466392318244,     /* 1024:729         F# */
          1.49492696045105,     /* 8^(5/4):9        G  */
          1.58024691358025,     /* 128:81           G# */
          1.6704363316362,      /* 1024*2^(1/4):729 A  */
          1.77777777777778,     /* 16:9             Bb */
          1.87924087309072};    /* 128*2^(1/4):81   B  */

/* multipliers from c in werckmeister iv */
static float S_tuning_mult_werckmeister_iv[12] = 
        { 1.0,                  /* 1:1                  C  */
          1.04875001176028,     /* 16384*2^(1/3):19683  C# */
          1.11992982212878,     /* 8*2^(1/3):9          D  */
          1.18518518518519,     /* 32:27                D# */
          1.25424280649339,     /* 64*4^(1/3):81        E  */
          1.33333333333333,     /* 4:3                  F  */
          1.40466392318244,     /* 1024:729             F# */
          1.49323976283837,     /* 32*2^(1/3):27        G  */
          1.57312501764042,     /* 8192*2^(1/3):6561    G# */
          1.67232374199119,     /* 256*4^(1/3):243      A  */
          1.78582618346427,     /* 9:4*2^(1/3)          Bb */
          1.87288523090992};    /* 4096:2187            B  */

/* multipliers from c in werckmeister v */
static float S_tuning_mult_werckmeister_v[12] = 
        { 1.0,                  /* 1:1            C  */
          1.05707299111353,     /* 8*2^(1/4):9    C# */
          1.125,                /* 9:8            D  */
          1.189207115002721,    /* 2^(1/4):1      D# */
          1.25707872210942,     /* 8*2^(1/2):9    E  */
          1.33785800437806,     /* 9*2^(1/4):8    F  */
          1.414213562373095,    /* 2^(1/2):1      F# */
          1.5,                  /* 3:2            G  */
          1.58024691358025,     /* 128:81         G# */
          1.681792830507429,    /* 8^(1/4):1      A  */
          1.78381067250408,     /* 3:8^(1/4)      Bb */
          1.88561808316413};    /* 4*2^(1/2):3    B  */

/* multipliers from c in werckmeister vi */
static float S_tuning_mult_werckmeister_vi[12] = 
        { 1.0,                  /* 1:1      C  */
          1.05376344086022,     /* 98:93    C# */
          1.12,                 /* 28:25    D  */
          1.18787878787879,     /* 196:165  D# */
          1.25641025641026,     /* 49:39    E  */
          1.33333333333333,     /* 4:3      F  */
          1.41007194244604,     /* 196:139  F# */
          1.49618320610687,     /* 196:131  G  */
          1.58064516129032,     /* 49:31    G# */
          1.67521367521368,     /* 196:117  A  */
          1.78181818181818,     /* 98:55    Bb */
          1.88461538461538};    /* 49:26    B  */

/* multipliers from c in renold i tuning */
static float S_tuning_mult_renold_i[12] = 
        { 1.0,                  /* 1:1            C  */
          1.06066017177982,     /* 3*2^(1/2):4    C# */
          1.125,                /* 9:8            D  */
          1.1932426932523,      /* 27*2^(1/2):32  D# */
          1.265625,             /* 81:64          E  */
          1.33333333333333,     /* 4:3            F  */
          1.414213562373095,    /* 2^(1/2):1      F# */
          1.5,                  /* 3:2            G  */
          1.59099025766973,     /* 9*2^(1/2):8    G# */
          1.6875,               /* 27:16          A  */
          1.78986403987845,     /* 81*2^(1/2):64  A# */
          1.8984375};           /* 243:128        B  */

/*******************************************************************************
** tuning_generate_tables()
*******************************************************************************/
short int tuning_generate_tables()
{
  int     i;
  int     j;

  float*  mult_table;

  float   cents;
  float   delta;

  float   val;

  /* determine multiplier table */
  if (G_tuning_system == TUNING_SYSTEM_12_ET)
    mult_table = S_tuning_mult_12_et;
  else if (G_tuning_system == TUNING_SYSTEM_PYTHAGOREAN)
    mult_table = S_tuning_mult_pythagorean;
  else if (G_tuning_system == TUNING_SYSTEM_QC_MEANTONE)
    mult_table = S_tuning_mult_qc_meantone;
  else if (G_tuning_system == TUNING_SYSTEM_JUST)
    mult_table = S_tuning_mult_just_intonation;
  else if (G_tuning_system == TUNING_SYSTEM_WERCKMEISTER_III)
    mult_table = S_tuning_mult_werckmeister_iii;
  else if (G_tuning_system == TUNING_SYSTEM_WERCKMEISTER_IV)
    mult_table = S_tuning_mult_werckmeister_iv;
  else if (G_tuning_system == TUNING_SYSTEM_WERCKMEISTER_V)
    mult_table = S_tuning_mult_werckmeister_v;
  else if (G_tuning_system == TUNING_SYSTEM_WERCKMEISTER_VI)
    mult_table = S_tuning_mult_werckmeister_vi;
  else if (G_tuning_system == TUNING_SYSTEM_RENOLD_I)
    mult_table = S_tuning_mult_renold_i;
  else
    mult_table = S_tuning_mult_12_et;

  /* compute frequency at tuning fork */
  if (G_tuning_fork == TUNING_FORK_C256)
    G_frequency_table[60 * 32] = 256;
  else if (G_tuning_fork == TUNING_FORK_A440)
    G_frequency_table[69 * 32] = 440;
  else if (G_tuning_fork == TUNING_FORK_A432)
    G_frequency_table[69 * 32] = 432;
  else if (G_tuning_fork == TUNING_FORK_AMIGA)
    G_frequency_table[60 * 32] = 261.34375; /* 8363 / 32 */
  else
    return 1;

  /* compute frequencies for notes in octave 4 based on tuning system */
  if ((G_tuning_fork == TUNING_FORK_C256) ||
      (G_tuning_fork == TUNING_FORK_AMIGA))
  {
    for (i = 1; i < 12; i++)
    {
      G_frequency_table[(60 + i) * 32] = 
        G_frequency_table[60 * 32] * mult_table[i];
    }
  }
  else if ( (G_tuning_fork == TUNING_FORK_A440) ||
            (G_tuning_fork == TUNING_FORK_A432))
  {
    for (i = 0; i < 12; i++)
    {
      if (i == 9)
        continue;

      G_frequency_table[(60 + i) * 32] = 
        G_frequency_table[69 * 32] * mult_table[i] / mult_table[9];
    }
  }

  /* compute frequencies at all notes */
  for (i = 0; i < 12; i++)
  {
    G_frequency_table[( 0 + i) * 32] = G_frequency_table[(60 + i) * 32] / 32;
    G_frequency_table[(12 + i) * 32] = G_frequency_table[(60 + i) * 32] / 16;
    G_frequency_table[(24 + i) * 32] = G_frequency_table[(60 + i) * 32] / 8;
    G_frequency_table[(36 + i) * 32] = G_frequency_table[(60 + i) * 32] / 4;
    G_frequency_table[(48 + i) * 32] = G_frequency_table[(60 + i) * 32] / 2;

    G_frequency_table[( 72 + i) * 32] = G_frequency_table[(60 + i) * 32] * 2;
    G_frequency_table[( 84 + i) * 32] = G_frequency_table[(60 + i) * 32] * 4;
    G_frequency_table[( 96 + i) * 32] = G_frequency_table[(60 + i) * 32] * 8;
    G_frequency_table[(108 + i) * 32] = G_frequency_table[(60 + i) * 32] * 16;

    if (120 + i < 128)
      G_frequency_table[(120 + i) * 32] = G_frequency_table[(60 + i) * 32] * 32;
  }

  /* compute frequencies between notes */
  for (i = 0; i < 12; i++)
  {
    cents = 1200 * log(mult_table[i]) / log(2);

    if (i != 11)
      delta = (1200 * (log(mult_table[i + 1]) - log(mult_table[i])) / log(2)) / 32.0f;
    else
      delta = (1200 * (log(2) - log(mult_table[i])) / log(2)) / 32.0f;

    for (j = 1; j < 32; j++)
    {
      G_frequency_table[32 * (60 + i) + j] = 
        G_frequency_table[32 * 60] * exp(log(2) * ((cents + delta * j) / 1200.0f));

      G_frequency_table[32 * ( 0 + i) + j] = G_frequency_table[32 * (60 + i) + j] / 32.0f;
      G_frequency_table[32 * (12 + i) + j] = G_frequency_table[32 * (60 + i) + j] / 16.0f;
      G_frequency_table[32 * (24 + i) + j] = G_frequency_table[32 * (60 + i) + j] / 8.0f;
      G_frequency_table[32 * (36 + i) + j] = G_frequency_table[32 * (60 + i) + j] / 4.0f;
      G_frequency_table[32 * (48 + i) + j] = G_frequency_table[32 * (60 + i) + j] / 2.0f;

      G_frequency_table[32 * (72 + i) + j] = G_frequency_table[32 * (60 + i) + j] * 2.0f;
      G_frequency_table[32 * (84 + i) + j] = G_frequency_table[32 * (60 + i) + j] * 4.0f;
      G_frequency_table[32 * (96 + i) + j] = G_frequency_table[32 * (60 + i) + j] * 8.0f;
      G_frequency_table[32 * (108 + i) + j] = G_frequency_table[32 * (60 + i) + j] * 16.0f;

      if (120 + i < 128)
        G_frequency_table[32 * (120 + i) + j] = G_frequency_table[32 * (60 + i) + j] * 32.0f;
    }
  }

  /* compute resonance table */

  /* see Vadim Zavalishin's "The Art of VA Filter Design"               */
  /* The values in the table are for K, which is in the interval [0, 2) */
  /* In Section 4.2, p. 103, there is a formula for R (the resonance    */
  /* parameter for the SVF) based on the resonance peak height A:       */
  /*   R = sqrt((1 - sqrt(1 - A^-2))/2)                                 */
  /* Here, we are computing K values for the TSK filter. As shown in    */
  /* Section 5.8, p. 152, we have 2R = 2 - K.                           */
  /* So, plugging the first equation into the second, we obtain:        */
  /*   K = 2 - sqrt(2[1 - sqrt(1 - A^-2)])                              */
  for (i = 0; i < 32; i++)
  {
    val = exp(log(10) * i / 32.0f);

    G_resonance_table[i] = 2 - sqrt(2 - (2 * sqrt(1 - (1 / (val * val)))));
  }

  /* generate other tables */
  for (i = 0; i < 4096; i++)
  {
    G_phase_increment_table[i] = 
      (int) ((G_frequency_table[i] * GENESIS_1HZ_PHASE_INCREMENT) + 0.5);

    /* see Vadim Zavalishin's "The Art of VA Filter Design" for equations */

    /* compute omega_0 */
    val = TWO_PI * G_frequency_table[i];

    /* pre-warping (section 3.8, p. 62)                               */
    /* (1/2) * new_omega_0 * delta_T = tan((1/2) * omega_0 * delta_T) */
    val = tanf(0.5f * val * GENESIS_DELTA_T_SECONDS);

    G_filter_omega_0_delta_t_over_2_table[i] = val;

    /* 1st order stage multiplier calculation (section 3.10, p. 76-77)              */
    /* multiplier = ((1/2) * omega_0 * delta_T) / [1 + ((1/2) * omega_0 * delta_T)] */
    G_filter_stage_multiplier_table[i] = val / (1.0f + val);
  }

#if 0
  printf("Frequency Table:\n");

  for (i = 0; i < (4096 / 4); i++)
  {
    printf("%f %f %f %f\n", G_frequency_table[4 * i + 0], 
                            G_frequency_table[4 * i + 1], 
                            G_frequency_table[4 * i + 2], 
                            G_frequency_table[4 * i + 3]);
  }
#endif

#if 0
  printf("Phase Increment Table:\n");

  for (i = 0; i < (4096 / 4); i++)
  {
    printf("%d %d %d %d\n", G_phase_increment_table[4 * i + 0], 
                            G_phase_increment_table[4 * i + 1], 
                            G_phase_increment_table[4 * i + 2], 
                            G_phase_increment_table[4 * i + 3]);
  }
#endif

#if 0
  printf("omega_0 * delta_t Table:\n");

  for (i = 0; i < (4096 / 4); i++)
  {
    printf("%f %f %f %f\n", G_filter_omega_0_delta_t_over_2_table[4 * i + 0], 
                            G_filter_omega_0_delta_t_over_2_table[4 * i + 1], 
                            G_filter_omega_0_delta_t_over_2_table[4 * i + 2], 
                            G_filter_omega_0_delta_t_over_2_table[4 * i + 3]);
  }
#endif

  return 0;
}

