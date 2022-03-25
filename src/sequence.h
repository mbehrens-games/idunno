/*******************************************************************************
** sequence.h (sequencer)
*******************************************************************************/

#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "synth.h"

enum
{
  SCALE_NAME_UNCHANGED = 0,
  /* major scale and its modes */
  SCALE_NAME_IONIAN,
  SCALE_NAME_DORIAN,
  SCALE_NAME_PHRYGIAN,
  SCALE_NAME_LYDIAN,
  SCALE_NAME_MIXOLYDIAN,
  SCALE_NAME_AEOLIAN,
  SCALE_NAME_LOCRIAN,
  /* ascending melodic minor scale and its modes */
  SCALE_NAME_MELODIC_MINOR,
  SCALE_NAME_MELODIC_MINOR_2ND_MODE,
  SCALE_NAME_MELODIC_MINOR_3RD_MODE,
  SCALE_NAME_MELODIC_MINOR_4TH_MODE,
  SCALE_NAME_MELODIC_MINOR_5TH_MODE,
  SCALE_NAME_MELODIC_MINOR_6TH_MODE,
  SCALE_NAME_MELODIC_MINOR_7TH_MODE,
  /* harmonic minor scale and its modes */
  SCALE_NAME_HARMONIC_MINOR,
  SCALE_NAME_HARMONIC_MINOR_2ND_MODE,
  SCALE_NAME_HARMONIC_MINOR_3RD_MODE,
  SCALE_NAME_HARMONIC_MINOR_4TH_MODE,
  SCALE_NAME_HARMONIC_MINOR_5TH_MODE,
  SCALE_NAME_HARMONIC_MINOR_6TH_MODE,
  SCALE_NAME_HARMONIC_MINOR_7TH_MODE,
  /* harmonic major scale and its modes */
  SCALE_NAME_HARMONIC_MAJOR,
  SCALE_NAME_HARMONIC_MAJOR_2ND_MODE,
  SCALE_NAME_HARMONIC_MAJOR_3RD_MODE,
  SCALE_NAME_HARMONIC_MAJOR_4TH_MODE,
  SCALE_NAME_HARMONIC_MAJOR_5TH_MODE,
  SCALE_NAME_HARMONIC_MAJOR_6TH_MODE,
  SCALE_NAME_HARMONIC_MAJOR_7TH_MODE,
  /* double harmonic scale and its modes */
  SCALE_NAME_DOUBLE_HARMONIC,
  SCALE_NAME_DOUBLE_HARMONIC_2ND_MODE,
  SCALE_NAME_DOUBLE_HARMONIC_3RD_MODE,
  SCALE_NAME_DOUBLE_HARMONIC_4TH_MODE,
  SCALE_NAME_DOUBLE_HARMONIC_5TH_MODE,
  SCALE_NAME_DOUBLE_HARMONIC_6TH_MODE,
  SCALE_NAME_DOUBLE_HARMONIC_7TH_MODE,
  /* neapolitan minor scale and its modes */
  SCALE_NAME_NEAPOLITAN_MINOR,
  SCALE_NAME_NEAPOLITAN_MINOR_2ND_MODE,
  SCALE_NAME_NEAPOLITAN_MINOR_3RD_MODE,
  SCALE_NAME_NEAPOLITAN_MINOR_4TH_MODE,
  SCALE_NAME_NEAPOLITAN_MINOR_5TH_MODE,
  SCALE_NAME_NEAPOLITAN_MINOR_6TH_MODE,
  SCALE_NAME_NEAPOLITAN_MINOR_7TH_MODE,
  /* neapolitan major scale and its modes */
  SCALE_NAME_NEAPOLITAN_MAJOR,
  SCALE_NAME_NEAPOLITAN_MAJOR_2ND_MODE,
  SCALE_NAME_NEAPOLITAN_MAJOR_3RD_MODE,
  SCALE_NAME_NEAPOLITAN_MAJOR_4TH_MODE,
  SCALE_NAME_NEAPOLITAN_MAJOR_5TH_MODE,
  SCALE_NAME_NEAPOLITAN_MAJOR_6TH_MODE,
  SCALE_NAME_NEAPOLITAN_MAJOR_7TH_MODE,
  /* hungarian major scale and its modes */
  SCALE_NAME_HUNGARIAN_MAJOR,
  SCALE_NAME_HUNGARIAN_MAJOR_2ND_MODE,
  SCALE_NAME_HUNGARIAN_MAJOR_3RD_MODE,
  SCALE_NAME_HUNGARIAN_MAJOR_4TH_MODE,
  SCALE_NAME_HUNGARIAN_MAJOR_5TH_MODE,
  SCALE_NAME_HUNGARIAN_MAJOR_6TH_MODE,
  SCALE_NAME_HUNGARIAN_MAJOR_7TH_MODE,
  /* romanian major scale and its modes */
  SCALE_NAME_ROMANIAN_MAJOR,
  SCALE_NAME_ROMANIAN_MAJOR_2ND_MODE,
  SCALE_NAME_ROMANIAN_MAJOR_3RD_MODE,
  SCALE_NAME_ROMANIAN_MAJOR_4TH_MODE,
  SCALE_NAME_ROMANIAN_MAJOR_5TH_MODE,
  SCALE_NAME_ROMANIAN_MAJOR_6TH_MODE,
  SCALE_NAME_ROMANIAN_MAJOR_7TH_MODE,
  /* persian scale and its modes */
  SCALE_NAME_PERSIAN,
  SCALE_NAME_PERSIAN_2ND_MODE,
  SCALE_NAME_PERSIAN_3RD_MODE,
  SCALE_NAME_PERSIAN_4TH_MODE,
  SCALE_NAME_PERSIAN_5TH_MODE,
  SCALE_NAME_PERSIAN_6TH_MODE,
  SCALE_NAME_PERSIAN_7TH_MODE,
  /* enigmatic scale and its modes */
  SCALE_NAME_ENIGMATIC,
  SCALE_NAME_ENIGMATIC_2ND_MODE,
  SCALE_NAME_ENIGMATIC_3RD_MODE,
  SCALE_NAME_ENIGMATIC_4TH_MODE,
  SCALE_NAME_ENIGMATIC_5TH_MODE,
  SCALE_NAME_ENIGMATIC_6TH_MODE,
  SCALE_NAME_ENIGMATIC_7TH_MODE,
  /* kanakangi scale and its modes */
  SCALE_NAME_KANAKANGI,
  SCALE_NAME_KANAKANGI_2ND_MODE,
  SCALE_NAME_KANAKANGI_3RD_MODE,
  SCALE_NAME_KANAKANGI_4TH_MODE,
  SCALE_NAME_KANAKANGI_5TH_MODE,
  SCALE_NAME_KANAKANGI_6TH_MODE,
  SCALE_NAME_KANAKANGI_7TH_MODE,
  SCALE_NAME_UPPER_BOUND
};

enum
{
  SCALE_TONIC_UNCHANGED = 0,
  SCALE_TONIC_C,
  SCALE_TONIC_C_SHARP,
  SCALE_TONIC_C_FLAT,
  SCALE_TONIC_D,
  SCALE_TONIC_D_SHARP,
  SCALE_TONIC_D_FLAT,
  SCALE_TONIC_E,
  SCALE_TONIC_E_SHARP,
  SCALE_TONIC_E_FLAT,
  SCALE_TONIC_F,
  SCALE_TONIC_F_SHARP,
  SCALE_TONIC_F_FLAT,
  SCALE_TONIC_G,
  SCALE_TONIC_G_SHARP,
  SCALE_TONIC_G_FLAT,
  SCALE_TONIC_A,
  SCALE_TONIC_A_SHARP,
  SCALE_TONIC_A_FLAT,
  SCALE_TONIC_B,
  SCALE_TONIC_B_SHARP,
  SCALE_TONIC_B_FLAT,
  SCALE_TONIC_UPPER_BOUND
};

#define SEQUENCER_MAX_MEASURES      16
#define SEQUENCER_MAX_STEPS         256

#define SEQUENCER_MAX_CHORD_NOTES   6

#define SEQUENCER_TICKS_PER_QUARTER_NOTE      6720  /* 4 * 16 * 3 * 5 * 7 */

#define SEQUENCER_ARPEGGIATOR_FLAGS_CLEAR     0x00

#define SEQUENCER_ARPEGGIATOR_FLAG_ROLLED     0x01
#define SEQUENCER_ARPEGGIATOR_FLAG_DIRECTION  0x02
#define SEQUENCER_ARPEGGIATOR_FLAG_REFLECT    0x04
#define SEQUENCER_ARPEGGIATOR_FLAG_REPEAT     0x08

#define SEQUENCER_ARPEGGIATOR_FLAG_ON         0x10

typedef struct step
{
  int   scale_name;
  int   scale_tonic;

  char  chord_notes[SEQUENCER_MAX_CHORD_NOTES];

  char  staff_position;
  char  staff_octave;

  char  duration;

  char  arp_mode;
  char  arp_subdivisions;
  char  arp_duration;

  char  volume;
} step;

typedef struct measure
{
  step  steps[SEQUENCER_MAX_STEPS];
  int   num_steps;
  int   step_index;

  char  length;
  char  beat;
  char  subdivisions;
} measure;

typedef struct sequencer
{
  measure measures[SEQUENCER_MAX_MEASURES];
  int     num_measures;
  int     measure_index;

  int     scale_index;
  int     tonic_index;

  char    midi_notes[SEQUENCER_MAX_CHORD_NOTES];
  int     num_midi_notes;

  int     arp_index;

  char    arp_flags;

  int     arp_period;

  int     step_cycles;
  int     arp_cycles;

  char    volume;
} sequencer;

extern int  G_sequencer_period_table[];

/* function declarations */
short int   sequencer_init(sequencer* seq);
sequencer*  sequencer_create();
short int   sequencer_deinit(sequencer* seq);
short int   sequencer_destroy(sequencer* seq);

short int   sequencer_reset(sequencer* seq);

short int   sequencer_activate_step(sequencer* seq, synth* syn);
short int   sequencer_ahead_one_tick(sequencer* seq, synth* syn);

float       sequencer_calculate_length(sequencer* seq);

short int   sequencer_generate_tables();

#endif
