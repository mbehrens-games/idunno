/*******************************************************************************
** sequence.c (sequencer)
*******************************************************************************/

#include <stdio.h>  /* testing */
#include <stdlib.h>
#include <math.h>

#include "global.h"
#include "sequence.h"
#include "synth.h"

int G_sequencer_period_table[224];

/* scale table */
/* scales: 84 (12 scales, 7 modes per scale)            */
/* tonics: 21 (C to B, each can also be sharp or flat)  */
/* midi notes: 49 (7 per octave, and 7 octaves)         */
static char S_scale_table[84][21][49];

/* semitone jump pattern for each base scale (1st mode) */
static char S_scale_semitone_jump_pattern[12][7] = 
  { {2, 2, 1, 2, 2, 2, 1},  /* ionian           */
    {2, 1, 2, 2, 2, 2, 1},  /* melodic minor    */
    {2, 1, 2, 2, 1, 3, 1},  /* harmonic minor   */
    {2, 2, 1, 2, 1, 3, 1},  /* harmonic major   */
    {1, 3, 1, 2, 1, 3, 1},  /* double harmonic  */
    {1, 2, 2, 2, 1, 3, 1},  /* neapolitan minor */
    {1, 2, 2, 2, 2, 2, 1},  /* neapolitan major */
    {3, 1, 2, 1, 2, 1, 2},  /* hungarian major  */
    {1, 3, 2, 1, 2, 1, 2},  /* romanian major   */
    {1, 3, 1, 1, 2, 3, 1},  /* persian          */
    {1, 3, 2, 2, 2, 1, 1},  /* enigmatic        */
    {1, 1, 3, 2, 1, 1, 3}   /* kanakangi        */
  };

/*******************************************************************************
** step_init()
*******************************************************************************/
short int step_init(step* st)
{
  int i;

  if (st == NULL)
    return 1;

  st->scale_name = SCALE_NAME_UNCHANGED;
  st->scale_tonic = SCALE_TONIC_UNCHANGED;

  for (i = 0; i < SEQUENCER_MAX_CHORD_NOTES; i++)
    st->chord_notes[i] = 0;

  st->staff_position = 0;
  st->staff_octave = 4;

  st->duration = 0;

  st->arp_mode = 0;
  st->arp_subdivisions = 1;
  st->arp_duration = 0;

  st->volume = 0;

  return 0;
}

/*******************************************************************************
** step_create()
*******************************************************************************/
step* step_create()
{
  step* st;

  st = malloc(sizeof(step));
  step_init(st);

  return st;
}

/*******************************************************************************
** step_deinit()
*******************************************************************************/
short int step_deinit(step* st)
{
  if (st == NULL)
    return 1;

  return 0;
}

/*******************************************************************************
** step_destroy()
*******************************************************************************/
short int step_destroy(step* st)
{
  if (st == NULL)
    return 1;

  step_deinit(st);
  free(st);

  return 0;
}

/*******************************************************************************
** measure_init()
*******************************************************************************/
short int measure_init(measure* m)
{
  int i;

  if (m == NULL)
    return 1;

  for (i = 0; i < SEQUENCER_MAX_STEPS; i++)
    step_init(&m->steps[i]);

  m->num_steps = 0;
  m->step_index = 0;

  m->length = 0;
  m->beat = 4;
  m->subdivisions = 1;

  return 0;
}

/*******************************************************************************
** measure_create()
*******************************************************************************/
measure* measure_create()
{
  measure* m;

  m = malloc(sizeof(measure));
  measure_init(m);

  return m;
}

/*******************************************************************************
** measure_deinit()
*******************************************************************************/
short int measure_deinit(measure* m)
{
  int i;

  if (m == NULL)
    return 1;

  for (i = 0; i < SEQUENCER_MAX_STEPS; i++)
    step_deinit(&m->steps[i]);

  m->num_steps = 0;
  m->step_index = 0;

  return 0;
}

/*******************************************************************************
** measure_destroy()
*******************************************************************************/
short int measure_destroy(measure* m)
{
  if (m == NULL)
    return 1;

  measure_deinit(m);
  free(m);

  return 0;
}

/*******************************************************************************
** sequencer_init()
*******************************************************************************/
short int sequencer_init(sequencer* seq)
{
  int i;

  if (seq == NULL)
    return 1;

  for (i = 0; i < SEQUENCER_MAX_MEASURES; i++)
    measure_init(&seq->measures[i]);

  seq->num_measures = 0;
  seq->measure_index = 0;

  seq->scale_index = 0;
  seq->tonic_index = 0;

  for (i = 0; i < SEQUENCER_MAX_CHORD_NOTES; i++)
    seq->midi_notes[i] = 0;

  seq->num_midi_notes = 0;

  seq->arp_index = 0;

  seq->arp_flags = SEQUENCER_ARPEGGIATOR_FLAGS_CLEAR;

  seq->arp_period = 0;

  seq->step_cycles = 0;
  seq->arp_cycles = 0;

  seq->volume = 0;

  return 0;
}

/*******************************************************************************
** sequencer_create()
*******************************************************************************/
sequencer* sequencer_create()
{
  sequencer* seq;

  seq = malloc(sizeof(sequencer));
  sequencer_init(seq);

  return seq;
}

/*******************************************************************************
** sequencer_deinit()
*******************************************************************************/
short int sequencer_deinit(sequencer* seq)
{
  int i;

  if (seq == NULL)
    return 1;

  for (i = 0; i < SEQUENCER_MAX_MEASURES; i++)
    measure_deinit(&seq->measures[i]);

  seq->num_measures = 0;
  seq->measure_index = 0;

  return 0;
}

/*******************************************************************************
** sequencer_destroy()
*******************************************************************************/
short int sequencer_destroy(sequencer* seq)
{
  if (seq == NULL)
    return 1;

  sequencer_deinit(seq);
  free(seq);

  return 0;
}

/*******************************************************************************
** sequencer_reset()
*******************************************************************************/
short int sequencer_reset(sequencer* seq)
{
  int i;

  seq->measure_index = 0;

  seq->scale_index = 0;
  seq->tonic_index = 0;

  for (i = 0; i < SEQUENCER_MAX_CHORD_NOTES; i++)
    seq->midi_notes[i] = 0;

  seq->num_midi_notes = 0;

  seq->arp_index = 0;

  seq->arp_flags = SEQUENCER_ARPEGGIATOR_FLAGS_CLEAR;

  seq->arp_period = 0;

  seq->step_cycles = 0;
  seq->arp_cycles = 0;

  seq->volume = 0;

  return 0;
}

/*******************************************************************************
** sequencer_activate_step()
*******************************************************************************/
short int sequencer_activate_step(sequencer* seq, synth* syn)
{
  int       i;
  int       j;

  measure*  m;
  step*     st;

  int       root_position;
  int       note_offset;

  char      swap_note;
  int       swap_index;

  if (seq == NULL)
    return 1;

  if (syn == NULL)
    return 1;

  /* set current measure */
  if ((seq->measure_index >= 0) && (seq->measure_index < seq->num_measures))
    m = &seq->measures[seq->measure_index];
  else
    return 1;

  /* set current step */
  if ((m->step_index >= 0) && (m->step_index < m->num_steps))
    st = &m->steps[m->step_index];
  else
    return 1;

  /* set scale index */
  if ((st->scale_name > SCALE_NAME_UNCHANGED) && (st->scale_name < SCALE_NAME_UPPER_BOUND))
    seq->scale_index = st->scale_name - 1;

  /* set scale tonic */
  if ((st->scale_tonic > SCALE_TONIC_UNCHANGED) && (st->scale_tonic < SCALE_TONIC_UPPER_BOUND))
    seq->tonic_index = st->scale_tonic - 1;

  /* reset midi notes */
  for (i = 0; i < SEQUENCER_MAX_CHORD_NOTES; i++)
    seq->midi_notes[i] = 0;

  seq->num_midi_notes = 0;

  /* set midi notes based on staff position & octave */
  if ((st->staff_position >= 1) && (st->staff_position <= 7)  &&
      (st->staff_octave >= 2)   && (st->staff_octave <= 6))
  {
    root_position = (7 * (st->staff_octave - 1)) + (st->staff_position - 1);

    note_offset = 0;

    for (i = 0; i < SEQUENCER_MAX_CHORD_NOTES; i++)
    {
      if (st->chord_notes[i] == 0)
        continue;
      else if (st->chord_notes[i] == 1)
        note_offset = 0;
      else if (st->chord_notes[i] == 2)
        note_offset = 1;
      else if (st->chord_notes[i] == 3)
        note_offset = 2;
      else if (st->chord_notes[i] == 4)
        note_offset = 3;
      else if (st->chord_notes[i] == 5)
        note_offset = 4;
      else if (st->chord_notes[i] == 6)
        note_offset = 5;
      else if (st->chord_notes[i] == 7)
        note_offset = 6;
      else if (st->chord_notes[i] == 8)
        note_offset = 7;
      else if (st->chord_notes[i] == -1)
        note_offset = -7;
      else if (st->chord_notes[i] == -2)
        note_offset = -6;
      else if (st->chord_notes[i] == -3)
        note_offset = -5;
      else if (st->chord_notes[i] == -4)
        note_offset = -4;
      else if (st->chord_notes[i] == -5)
        note_offset = -3;
      else if (st->chord_notes[i] == -6)
        note_offset = -2;
      else if (st->chord_notes[i] == -7)
        note_offset = -1;
      else
        continue;

      seq->midi_notes[seq->num_midi_notes] = 
        S_scale_table[seq->scale_index][seq->tonic_index][root_position + note_offset];

      seq->num_midi_notes += 1;
    }
  }

  /* sort midi notes */
  for (i = 0; i < seq->num_midi_notes - 1; i++)
  {
    swap_index = i;

    for (j = i + 1; j < seq->num_midi_notes; j++)
    {
      if (seq->midi_notes[j] < seq->midi_notes[swap_index])
        swap_index = j;
    }

    if (swap_index != i)
    {
      swap_note = seq->midi_notes[swap_index];
      seq->midi_notes[swap_index] = seq->midi_notes[i];
      seq->midi_notes[i] = swap_note;
    }
  }

  /* set arpeggiator mode */
  seq->arp_flags = SEQUENCER_ARPEGGIATOR_FLAGS_CLEAR;

  /* mode 0: arpeggiator off */
  if (st->arp_mode == 0)
  {
    seq->arp_flags &= ~SEQUENCER_ARPEGGIATOR_FLAG_ON;
  }
  /* mode 1: rolled chord up */
  else if (st->arp_mode == 1)
  {
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_ON;
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_ROLLED;
  }
  /* mode 2: rolled chord down */
  else if (st->arp_mode == 2)
  {
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_ON;
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_ROLLED;
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_DIRECTION;
  }
  /* mode 3: grace notes up */
  else if (st->arp_mode == 3)
  {
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_ON;
  }
  /* mode 4: grace notes down */
  else if (st->arp_mode == 4)
  {
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_ON;
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_DIRECTION;
  }
  /* mode 5: arpeggio up */
  else if (st->arp_mode == 5)
  {
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_ON;
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_REPEAT;
  }
  /* mode 6: arpeggio down */
  else if (st->arp_mode == 6)
  {
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_ON;
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_DIRECTION;
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_REPEAT;
  }
  /* mode 7: arpeggio up & down (ping pong) */
  else if (st->arp_mode == 7)
  {
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_ON;
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_REFLECT;
  }
  /* mode 8: arpeggio up & down (double edge) */
  else if (st->arp_mode == 8)
  {
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_ON;
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_REFLECT;
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_REPEAT;
  }
  /* mode 9: arpeggio down & up (ping pong) */
  else if (st->arp_mode == 9)
  {
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_ON;
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_DIRECTION;
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_REFLECT;
  }
  /* mode 10: arpeggio down & up (double edge) */
  else if (st->arp_mode == 10)
  {
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_ON;
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_DIRECTION;
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_REFLECT;
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_REPEAT;
  }
  /* mode 11: rolled arpeggio up */
  else if (st->arp_mode == 11)
  {
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_ON;
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_ROLLED;
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_REPEAT;
  }
  /* mode 12: rolled arpeggio down */
  else if (st->arp_mode == 12)
  {
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_ON;
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_ROLLED;
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_DIRECTION;
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_REPEAT;
  }
  /* mode 13: rolled arpeggio up & down (ping pong) */
  else if (st->arp_mode == 13)
  {
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_ON;
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_ROLLED;
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_REFLECT;
  }
  /* mode 14: rolled arpeggio up & down (double edge) */
  else if (st->arp_mode == 14)
  {
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_ON;
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_ROLLED;
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_REFLECT;
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_REPEAT;
  }
  /* mode 15: rolled arpeggio down & up (ping pong) */
  else if (st->arp_mode == 15)
  {
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_ON;
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_ROLLED;
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_DIRECTION;
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_REFLECT;
  }
  /* mode 16: rolled arpeggio down & up (double edge) */
  else if (st->arp_mode == 16)
  {
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_ON;
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_ROLLED;
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_DIRECTION;
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_REFLECT;
    seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_REPEAT;
  }
  else
  {
    seq->arp_flags &= ~SEQUENCER_ARPEGGIATOR_FLAG_ON;
  }

  /* make sure arpeggiator is off if there are no   */
  /* chord notes, or if this is a unison interval.  */
  if (seq->num_midi_notes <= 1)
    seq->arp_flags &= ~SEQUENCER_ARPEGGIATOR_FLAG_ON;

  /* set step cycles counter */
  if (m->beat == 2)
    seq->step_cycles = SEQUENCER_TICKS_PER_QUARTER_NOTE * 2;
  else if (m->beat == 4)
    seq->step_cycles = SEQUENCER_TICKS_PER_QUARTER_NOTE;
  else if (m->beat == 8)
    seq->step_cycles = SEQUENCER_TICKS_PER_QUARTER_NOTE / 2;
  else if (m->beat == 16)
    seq->step_cycles = SEQUENCER_TICKS_PER_QUARTER_NOTE / 4;
  else
    seq->step_cycles = SEQUENCER_TICKS_PER_QUARTER_NOTE;

  if ((m->subdivisions == 2)  || (m->subdivisions == 3)   || 
      (m->subdivisions == 4)  || (m->subdivisions == 5)   || 
      (m->subdivisions == 6)  || (m->subdivisions == 7)   || 
      (m->subdivisions == 8)  || (m->subdivisions == 10)  || 
      (m->subdivisions == 12) || (m->subdivisions == 14)  || 
      (m->subdivisions == 16))
  {
    seq->step_cycles /= m->subdivisions;
  }

  if (st->duration > 1)
    seq->step_cycles *= st->duration;

  /* set arpeggio cycles counter */
  if (m->beat == 2)
    seq->arp_cycles = SEQUENCER_TICKS_PER_QUARTER_NOTE * 2;
  else if (m->beat == 4)
    seq->arp_cycles = SEQUENCER_TICKS_PER_QUARTER_NOTE;
  else if (m->beat == 8)
    seq->arp_cycles = SEQUENCER_TICKS_PER_QUARTER_NOTE / 2;
  else if (m->beat == 16)
    seq->arp_cycles = SEQUENCER_TICKS_PER_QUARTER_NOTE / 4;
  else
    seq->arp_cycles = SEQUENCER_TICKS_PER_QUARTER_NOTE;

  if ((st->arp_subdivisions == 2)   || (st->arp_subdivisions == 3)  || 
      (st->arp_subdivisions == 4)   || (st->arp_subdivisions == 5)  || 
      (st->arp_subdivisions == 6)   || (st->arp_subdivisions == 7)  || 
      (st->arp_subdivisions == 8)   || (st->arp_subdivisions == 10) || 
      (st->arp_subdivisions == 12)  || (st->arp_subdivisions == 14) || 
      (st->arp_subdivisions == 16))
  {
    seq->arp_cycles /= st->arp_subdivisions;
  }

  if (st->arp_duration > 1)
    seq->arp_cycles *= st->arp_duration;

  /* set arpeggio period */
  seq->arp_period = seq->arp_cycles;

  /* set volume */
  seq->volume = st->volume;

  /* key off on all voices */
  for (i = 0; i < SYNTH_MAX_VOICES; i++)
    synth_key_off(syn, i);

  /* play initial note(s) */
  if (seq->arp_flags & SEQUENCER_ARPEGGIATOR_FLAG_ON)
  {
    if (seq->arp_flags & SEQUENCER_ARPEGGIATOR_FLAG_DIRECTION)
      seq->arp_index = seq->num_midi_notes - 1;
    else
      seq->arp_index = 0;

    if ((seq->arp_index < 0) || (seq->arp_index >= seq->num_midi_notes))
    {
      seq->arp_flags &= ~SEQUENCER_ARPEGGIATOR_FLAG_ON;
      return 0;
    }

    synth_key_on(syn, seq->arp_index, 
                      seq->midi_notes[seq->arp_index], 
                      seq->volume);
  }
  else
  {
    for (i = 0; i < seq->num_midi_notes; i++)
      synth_key_on(syn, i, seq->midi_notes[i], seq->volume);
  }

  return 0;
}

/*******************************************************************************
** sequencer_ahead_one_tick()
*******************************************************************************/
short int sequencer_ahead_one_tick(sequencer* seq, synth* syn)
{
  int       i;

  measure*  m;

  if (seq == NULL)
    return 1;

  if (syn == NULL)
    return 1;

  /* if all measures played, return */
  if ((seq->measure_index < 0) || (seq->measure_index >= seq->num_measures))
    return 0;

  /* decrement cycles */
  if (seq->step_cycles > 0)
    seq->step_cycles -= 1;

  if ((seq->arp_flags & SEQUENCER_ARPEGGIATOR_FLAG_ON) && (seq->arp_cycles > 0))
    seq->arp_cycles -= 1;

  /* advance sequence if necessary */
  if (seq->step_cycles == 0)
  {
    /* set current measure */
    m = &seq->measures[seq->measure_index];

    /* increment current step */
    m->step_index += 1;

    /* see if this measure is completed */
    if (m->step_index >= m->num_steps)
    {
      seq->measure_index += 1;

      if ((seq->measure_index < 0) || (seq->measure_index >= seq->num_measures))
        return 0;

      m = &seq->measures[seq->measure_index];

      m->step_index = 0;
    }

    /* process this step */
    sequencer_activate_step(seq, syn);
  }

  /* advance arpeggiator if necessary */
  if ((seq->arp_flags & SEQUENCER_ARPEGGIATOR_FLAG_ON) && (seq->arp_cycles == 0))
  {
    /* downward direction */
    if (seq->arp_flags & SEQUENCER_ARPEGGIATOR_FLAG_DIRECTION)
    {
      seq->arp_index -= 1;

      if (seq->arp_index < 0)
      {
        /* double edge reflection */
        if ((seq->arp_flags & SEQUENCER_ARPEGGIATOR_FLAG_REPEAT) &&
            (seq->arp_flags & SEQUENCER_ARPEGGIATOR_FLAG_REFLECT))
        {
          seq->arp_flags &= ~SEQUENCER_ARPEGGIATOR_FLAG_DIRECTION;
          seq->arp_index = 0;
        }
        /* repeat */
        else if (seq->arp_flags & SEQUENCER_ARPEGGIATOR_FLAG_REPEAT)
        {
          seq->arp_index = seq->num_midi_notes - 1;
        }
        /* ping pong reflection */
        else if (seq->arp_flags & SEQUENCER_ARPEGGIATOR_FLAG_REFLECT)
        {
          seq->arp_flags &= ~SEQUENCER_ARPEGGIATOR_FLAG_DIRECTION;
          seq->arp_index = 1;
        }
        /* single cycle */
        else
        {
          seq->arp_flags &= ~SEQUENCER_ARPEGGIATOR_FLAG_ON;
        }
      }
    }
    /* upward direction */
    else
    {
      seq->arp_index += 1;

      if (seq->arp_index >= seq->num_midi_notes)
      {
        /* double edge reflection */
        if ((seq->arp_flags & SEQUENCER_ARPEGGIATOR_FLAG_REPEAT) &&
            (seq->arp_flags & SEQUENCER_ARPEGGIATOR_FLAG_REFLECT))
        {
          seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_DIRECTION;
          seq->arp_index = seq->num_midi_notes - 1;
        }
        /* repeat */
        else if (seq->arp_flags & SEQUENCER_ARPEGGIATOR_FLAG_REPEAT)
        {
          seq->arp_index = 0;
        }
        /* ping pong reflection */
        else if (seq->arp_flags & SEQUENCER_ARPEGGIATOR_FLAG_REFLECT)
        {
          seq->arp_flags |= SEQUENCER_ARPEGGIATOR_FLAG_DIRECTION;
          seq->arp_index = seq->num_midi_notes - 2;
        }
        /* single cycle */
        else
        {
          seq->arp_flags &= ~SEQUENCER_ARPEGGIATOR_FLAG_ON;
        }
      }
    }

    seq->arp_cycles = seq->arp_period;

    /* if arpeggiator is on, play note */
    if (seq->arp_flags & SEQUENCER_ARPEGGIATOR_FLAG_ON)
    {
      if (seq->arp_flags & SEQUENCER_ARPEGGIATOR_FLAG_ROLLED)
      {
        synth_key_on(syn, seq->arp_index, 
                          seq->midi_notes[seq->arp_index], 
                          seq->volume);
      }
      else
      {
        for (i = 0; i < SYNTH_MAX_VOICES; i++)
          synth_key_off(syn, i);

        synth_key_on(syn, seq->arp_index, 
                          seq->midi_notes[seq->arp_index], 
                          seq->volume);
      }
    }
  }

  return 0;
}

/*******************************************************************************
** sequencer_calculate_length()
*******************************************************************************/
float sequencer_calculate_length(sequencer* seq)
{
  int       i;
  int       j;

  measure*  m;
  step*     st;

  int       total_ticks;
  float     delta_t;

  int       beat_ticks;
  int       measure_ticks;
  int       step_ticks;

  /* compute total number of ticks */
  total_ticks = 0;

  for (i = 0; i < seq->num_measures; i++)
  {
    m = &seq->measures[i];

    /* compute ticks in each beat */
    if (m->beat == 2)
      beat_ticks = SEQUENCER_TICKS_PER_QUARTER_NOTE * 2;
    else if (m->beat == 4)
      beat_ticks = SEQUENCER_TICKS_PER_QUARTER_NOTE;
    else if (m->beat == 8)
      beat_ticks = SEQUENCER_TICKS_PER_QUARTER_NOTE / 2;
    else if (m->beat == 16)
      beat_ticks = SEQUENCER_TICKS_PER_QUARTER_NOTE / 4;
    else
      beat_ticks = SEQUENCER_TICKS_PER_QUARTER_NOTE;

    /* compute ticks in this measure */
    measure_ticks = 0;

    for (j = 0; j < m->num_steps; j++)
    {
      st = &m->steps[j];

      step_ticks = beat_ticks;

      if ((m->subdivisions == 2)  || (m->subdivisions == 3)   || 
          (m->subdivisions == 4)  || (m->subdivisions == 5)   || 
          (m->subdivisions == 6)  || (m->subdivisions == 7)   || 
          (m->subdivisions == 8)  || (m->subdivisions == 10)  || 
          (m->subdivisions == 12) || (m->subdivisions == 14)  || 
          (m->subdivisions == 16))
      {
        step_ticks /= m->subdivisions;
      }

      if (st->duration > 1)
        step_ticks *= st->duration;

      measure_ticks += step_ticks;
    }

    /* check if the number of ticks in the measure matches the time signature */
    if (measure_ticks != beat_ticks * m->length)
    {
      printf("Warning: Size of Measure %d does not match the time signature.\n", i);
    }

    /* add measure ticks to total ticks */
    total_ticks += measure_ticks;
  }

  /* compute tick period (in seconds) */
  delta_t = 60.0f / (G_bpm * SEQUENCER_TICKS_PER_QUARTER_NOTE);

  /* return length of sequence (in seconds) */
  return (total_ticks * delta_t);
}

/*******************************************************************************
** sequencer_generate_tables()
*******************************************************************************/
short int sequencer_generate_tables()
{
  int   i;
  int   j;
  int   k;
  float val;

  /* sequencer period table */
  for (i = 0; i < 224; i++)
  {
    val = 60.0f / ((i + 32) * SEQUENCER_TICKS_PER_QUARTER_NOTE);

    G_sequencer_period_table[i] = (int) ((val * 1000000000) + 0.5f);
  }

  /* scale table */

  /* set tonic notes in octave 4 */
  for (i = 0; i < 84; i++)
  {
    S_scale_table[i][0][21]   = 60; /* C  */
    S_scale_table[i][1][21]   = 61; /* C# */
    S_scale_table[i][2][21]   = 59; /* Cb */

    S_scale_table[i][3][22]   = 62; /* D  */
    S_scale_table[i][4][22]   = 63; /* D# */
    S_scale_table[i][5][22]   = 61; /* Db */

    S_scale_table[i][6][23]   = 64; /* E  */
    S_scale_table[i][7][23]   = 65; /* E# */
    S_scale_table[i][8][23]   = 63; /* Eb */

    S_scale_table[i][9][24]   = 65; /* F  */
    S_scale_table[i][10][24]  = 66; /* F# */
    S_scale_table[i][11][24]  = 64; /* Fb */

    S_scale_table[i][12][25]  = 67; /* G  */
    S_scale_table[i][13][25]  = 68; /* G# */
    S_scale_table[i][14][25]  = 66; /* Gb */

    S_scale_table[i][15][26]  = 69; /* A  */
    S_scale_table[i][16][26]  = 70; /* A# */
    S_scale_table[i][17][26]  = 68; /* Ab */

    S_scale_table[i][18][27]  = 71; /* B  */
    S_scale_table[i][19][27]  = 72; /* B# */
    S_scale_table[i][20][27]  = 70; /* Bb */
  }

  /* generate one octave of each scale from the tonic */
  for (i = 0; i < 84; i++)
  {
    for (j = 0; j < 21; j++)
    {
      for (k = 0; k < 7; k++)
      {
        S_scale_table[i][j][k + (j / 3) + 21 + 1] = 
          S_scale_table[i][j][k + (j / 3) + 21] + S_scale_semitone_jump_pattern[i / 7][(k + i) % 7];
      }
    }
  }

  /* compute rest of each scale in the other octaves */
  for (i = 0; i < 84; i++)
  {
    for (j = 0; j < 21; j++)
    {
      for (k = 0; k < 7; k++)
      {
        if (-7 + (j / 3) + k >= 0)
          S_scale_table[i][j][-7 + (j / 3) + k] = S_scale_table[i][j][21 + (j / 3) + k] - 48;

        S_scale_table[i][j][ 0 + (j / 3) + k] = S_scale_table[i][j][21 + (j / 3) + k] - 36;
        S_scale_table[i][j][ 7 + (j / 3) + k] = S_scale_table[i][j][21 + (j / 3) + k] - 24;
        S_scale_table[i][j][14 + (j / 3) + k] = S_scale_table[i][j][21 + (j / 3) + k] - 12;

        S_scale_table[i][j][28 + (j / 3) + k] = S_scale_table[i][j][21 + (j / 3) + k] + 12;
        S_scale_table[i][j][35 + (j / 3) + k] = S_scale_table[i][j][21 + (j / 3) + k] + 24;

        if (42 + (j / 3) + k <= 49)
          S_scale_table[i][j][42 + (j / 3) + k] = S_scale_table[i][j][21 + (j / 3) + k] + 36;
      }
    }
  }

  return 0;
}
