/*******************************************************************************
** parse.c (parsing functions)
*******************************************************************************/

#include <stdio.h>    /* testing */
#include <stdlib.h>
#include <string.h>

#include "datatree.h"
#include "global.h"
#include "parse.h"
#include "sequence.h"
#include "synth.h"
#include "token.h"
#include "tuning.h"
#include "waveform.h"

#define PARSE_EAT_TOKEN(just_eat_it)                                           \
  if (t.token == just_eat_it)                                                  \
    tokenizer_advance(&t);                                                     \
  else                                                                         \
    goto houston;

/*******************************************************************************
** parse_file_to_data_tree()
*******************************************************************************/
data_tree_node* parse_file_to_data_tree(char* filename)
{
  tokenizer         t;
  data_tree_node*   root;
  data_tree_node*   current;
  data_tree_node**  stack;
  int               stack_size;
  int               stack_top;
  int               parse_state;

  /* initialize tokenizer and open file */
  tokenizer_init(&t);

  if (tokenizer_open_file(&t, filename))
    return NULL;

  /* setup stack */
  stack = malloc(DATA_TREE_STACK_INITIAL_SIZE * sizeof(data_tree_node*));
  stack_size = DATA_TREE_STACK_INITIAL_SIZE;
  stack_top = -1;

  /* initial parsing; create root node and push onto stack */
  root = data_tree_node_create();

  PARSE_EAT_TOKEN(TOKEN_LESS_THAN)

  if ((t.token == TOKEN_IDENTIFIER) && (!strcmp(t.sb, "idunno")))
  {
    root->type = DATA_TREE_NODE_TYPE_FIELD_IDUNNO;
    current = root;
    DATA_TREE_PUSH_NODE(stack, root)
    tokenizer_advance(&t);
  }
  else
    goto cleanup;

  parse_state = PARSE_STATE_ATTRIBUTE_SUBFIELD_OR_VALUE;

  /* begin parsing subfields */
  while (stack_top >= 0)
  {
    /* attribute */
    if ((t.token == TOKEN_AT_SYMBOL) && 
        (parse_state == PARSE_STATE_ATTRIBUTE_SUBFIELD_OR_VALUE))
    {
      DATA_TREE_CREATE_NEW_NODE(stack, current)
      tokenizer_advance(&t);

      if (t.token != TOKEN_IDENTIFIER)
        goto houston;

      if (!strcmp(t.sb, "bpm"))
        current->type = DATA_TREE_NODE_TYPE_ATTRIBUTE_BPM;
      else if (!strcmp(t.sb, "export_sampling"))
        current->type = DATA_TREE_NODE_TYPE_ATTRIBUTE_EXPORT_SAMPLING;
      else if (!strcmp(t.sb, "export_bitres"))
        current->type = DATA_TREE_NODE_TYPE_ATTRIBUTE_EXPORT_BITRES;
      else if (!strcmp(t.sb, "downsampling_m"))
        current->type = DATA_TREE_NODE_TYPE_ATTRIBUTE_DOWNSAMPLING_M;
      else if (!strcmp(t.sb, "tuning_system"))
        current->type = DATA_TREE_NODE_TYPE_ATTRIBUTE_TUNING_SYSTEM;
      else if (!strcmp(t.sb, "tuning_fork"))
        current->type = DATA_TREE_NODE_TYPE_ATTRIBUTE_TUNING_FORK;
      else
        goto houston;

      tokenizer_advance(&t);

      PARSE_EAT_TOKEN(TOKEN_EQUAL_SIGN)

      if (t.token == TOKEN_NUMBER_INTEGER)
      {
        current->child = data_tree_node_create();
        current->child->type = DATA_TREE_NODE_TYPE_VALUE_INTEGER;
        current->child->value = strdup(t.sb);
      }
      else if (t.token == TOKEN_STRING)
      {
        current->child = data_tree_node_create();
        current->child->type = DATA_TREE_NODE_TYPE_VALUE_STRING;
        current->child->value = strdup(t.sb);
      }
      else
        goto houston;

      tokenizer_advance(&t);
    }
    /* subfield */
    else if ( (t.token == TOKEN_LESS_THAN) &&
              ( (parse_state == PARSE_STATE_ATTRIBUTE_SUBFIELD_OR_VALUE) ||
                (parse_state == PARSE_STATE_SUBFIELD_OR_END_OF_FIELD)))
    {
      DATA_TREE_CREATE_NEW_NODE(stack, current)
      tokenizer_advance(&t);

      if (t.token != TOKEN_IDENTIFIER)
        goto houston;

      /* top level fields */
      if (!strcmp(t.sb, "generator"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_GENERATOR;
      else if (!strcmp(t.sb, "noise"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_NOISE;
      else if (!strcmp(t.sb, "filter"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_FILTER;
      else if (!strcmp(t.sb, "reverb"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_REVERB;
      else if (!strcmp(t.sb, "amplitude_envelope"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_AMPLITUDE_ENVELOPE;
      else if (!strcmp(t.sb, "filter_envelope"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_FILTER_ENVELOPE;
      else if (!strcmp(t.sb, "vibrato"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_VIBRATO;
      else if (!strcmp(t.sb, "tremolo"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_TREMOLO;
      else if (!strcmp(t.sb, "wobble"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_WOBBLE;
      else if (!strcmp(t.sb, "hpf"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_HPF;
      else if (!strcmp(t.sb, "soft_clip"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_SOFT_CLIP;
      else if (!strcmp(t.sb, "sequencer"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_SEQUENCER;
      /* waveform generator fields */
      else if (!strcmp(t.sb, "osc_1"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_OSC_1;
      else if (!strcmp(t.sb, "osc_2"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_OSC_2;
      else if (!strcmp(t.sb, "osc_3"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_OSC_3;
      else if (!strcmp(t.sb, "phi"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_PHI;
      else if (!strcmp(t.sb, "sync"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_SYNC;
      else if (!strcmp(t.sb, "mix"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_MIX;
      else if (!strcmp(t.sb, "ring_mod"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_RING_MOD;
      /* oscillator fields */
      else if (!strcmp(t.sb, "waveform"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_WAVEFORM;
      else if (!strcmp(t.sb, "detune_octave"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_DETUNE_OCTAVE;
      else if (!strcmp(t.sb, "detune_coarse"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_DETUNE_COARSE;
      else if (!strcmp(t.sb, "detune_fine"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_DETUNE_FINE;
      /* noise generator fields */
      else if (!strcmp(t.sb, "period"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_PERIOD;
      /* filter fields */
      else if (!strcmp(t.sb, "cutoff"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_CUTOFF;
      else if (!strcmp(t.sb, "keytrack"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_KEYTRACK;
      else if (!strcmp(t.sb, "resonance"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_RESONANCE;
      /* reverb fields */
      else if (!strcmp(t.sb, "delay"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_DELAY;
      else if (!strcmp(t.sb, "c_0"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_C_0;
      else if (!strcmp(t.sb, "c_1"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_C_1;
      else if (!strcmp(t.sb, "c_2"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_C_2;
      else if (!strcmp(t.sb, "c_3"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_C_3;
      else if (!strcmp(t.sb, "c_4"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_C_4;
      else if (!strcmp(t.sb, "c_5"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_C_5;
      else if (!strcmp(t.sb, "c_6"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_C_6;
      else if (!strcmp(t.sb, "c_7"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_C_7;
      else if (!strcmp(t.sb, "feedback"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_FEEDBACK;
      else if (!strcmp(t.sb, "volume"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_VOLUME;
      /* envelope fields */
      else if (!strcmp(t.sb, "ar"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_AR;
      else if (!strcmp(t.sb, "dr"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_DR;
      else if (!strcmp(t.sb, "sr"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_SR;
      else if (!strcmp(t.sb, "rr"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_RR;
      else if (!strcmp(t.sb, "tl"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_TL;
      else if (!strcmp(t.sb, "sl"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_SL;
      else if (!strcmp(t.sb, "rks"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_RKS;
      else if (!strcmp(t.sb, "lks"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_LKS;
      /* lfo fields */
      else if (!strcmp(t.sb, "depth"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_DEPTH;
      else if (!strcmp(t.sb, "speed"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_SPEED;
      /* sequencer fields */
      else if (!strcmp(t.sb, "measure"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_MEASURE;
      /* measure fields */
      else if (!strcmp(t.sb, "step"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_STEP;
      else if (!strcmp(t.sb, "length"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_LENGTH;
      else if (!strcmp(t.sb, "beat"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_BEAT;
      else if (!strcmp(t.sb, "subdivisions"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_SUBDIVISIONS;
      /* step fields */
      else if (!strcmp(t.sb, "scale"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_SCALE;
      else if (!strcmp(t.sb, "chord"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_CHORD;
      else if (!strcmp(t.sb, "arpeggiator"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_ARPEGGIATOR;
      else if (!strcmp(t.sb, "position"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_POSITION;
      else if (!strcmp(t.sb, "octave"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_OCTAVE;
      else if (!strcmp(t.sb, "duration"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_DURATION;
      /* scale fields */
      else if (!strcmp(t.sb, "name"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_NAME;
      else if (!strcmp(t.sb, "tonic"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_TONIC;
      /* chord fields */
      else if (!strcmp(t.sb, "note_1"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_NOTE_1;
      else if (!strcmp(t.sb, "note_2"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_NOTE_2;
      else if (!strcmp(t.sb, "note_3"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_NOTE_3;
      else if (!strcmp(t.sb, "note_4"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_NOTE_4;
      else if (!strcmp(t.sb, "note_5"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_NOTE_5;
      else if (!strcmp(t.sb, "note_6"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_NOTE_6;
      /* arpeggiator fields */
      else if (!strcmp(t.sb, "mode"))
        current->type = DATA_TREE_NODE_TYPE_FIELD_MODE;
      else
        goto houston;

      DATA_TREE_PUSH_NODE(stack, current)
      tokenizer_advance(&t);
      parse_state = PARSE_STATE_ATTRIBUTE_SUBFIELD_OR_VALUE;
    }
    /* integer */
    else if ( (t.token == TOKEN_NUMBER_INTEGER) &&
              (parse_state == PARSE_STATE_ATTRIBUTE_SUBFIELD_OR_VALUE))
    {
      DATA_TREE_CREATE_NEW_NODE(stack, current)
      current->type = DATA_TREE_NODE_TYPE_VALUE_INTEGER;
      current->value = strdup(t.sb);
      tokenizer_advance(&t);
      parse_state = PARSE_STATE_END_OF_FIELD;
    }
    /* string */
    else if ( (t.token == TOKEN_STRING) &&
              (parse_state == PARSE_STATE_ATTRIBUTE_SUBFIELD_OR_VALUE))

    {
      DATA_TREE_CREATE_NEW_NODE(stack, current)
      current->type = DATA_TREE_NODE_TYPE_VALUE_STRING;
      current->value = strdup(t.sb);
      tokenizer_advance(&t);
      parse_state = PARSE_STATE_END_OF_FIELD;
    }
    /* end of field */
    else if ( (t.token == TOKEN_GREATER_THAN) &&
              ( (parse_state == PARSE_STATE_END_OF_FIELD) ||
                (parse_state == PARSE_STATE_SUBFIELD_OR_END_OF_FIELD)))
    {
      current = stack[stack_top];
      DATA_TREE_POP_NODE(stack)
      tokenizer_advance(&t);
      parse_state = PARSE_STATE_SUBFIELD_OR_END_OF_FIELD;
    }
    /* error */
    else
      goto houston;
  }

  /* read eof and cleanup */
  PARSE_EAT_TOKEN(TOKEN_EOF)

  goto cleanup;

  /* error handling */
houston:
  if (root != NULL)
  {
    data_tree_node_destroy_tree(root);
    root = NULL;
  }

  printf("Failed text file parsing on line number %d.\n", t.ln);

  /* cleanup */
cleanup:
  if (stack != NULL)
  {
    free(stack);
    stack = NULL;
  }

  tokenizer_close_file(&t);
  tokenizer_deinit(&t);

  return root;
}

/*******************************************************************************
** parse_data_tree_semantic_analysis()
*******************************************************************************/
short int parse_data_tree_semantic_analysis(int current_type, int parent_type)
{
  if ((current_type == DATA_TREE_NODE_TYPE_FIELD_IDUNNO) &&
      (parent_type != DATA_TREE_NODE_TYPE_NONE))
    return 1;
  /* top level fields */
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_GENERATOR) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_IDUNNO))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_NOISE) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_IDUNNO))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_FILTER) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_IDUNNO))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_REVERB) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_IDUNNO))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_AMPLITUDE_ENVELOPE) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_IDUNNO))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_FILTER_ENVELOPE) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_IDUNNO))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_VIBRATO) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_IDUNNO))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_TREMOLO) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_IDUNNO))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_WOBBLE) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_IDUNNO))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_HPF) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_IDUNNO))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_SOFT_CLIP) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_IDUNNO))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_SEQUENCER) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_IDUNNO))
    return 1;
  /* waveform generator fields */
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_OSC_1) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_GENERATOR))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_OSC_2) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_GENERATOR))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_OSC_3) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_GENERATOR))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_PHI) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_GENERATOR))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_SYNC) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_GENERATOR))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_MIX)       &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_GENERATOR)  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_NOISE))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_RING_MOD) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_GENERATOR))
    return 1;
  /* oscillator fields */
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_WAVEFORM)  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OSC_1)      &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OSC_2)      &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_VIBRATO)    &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_TREMOLO)    &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_WOBBLE))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_DETUNE_OCTAVE) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OSC_1)          &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OSC_2)          &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OSC_3))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_DETUNE_COARSE) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OSC_1)          &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OSC_2)          &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OSC_3))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_DETUNE_FINE) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OSC_1)        &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OSC_2)        &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OSC_3))
    return 1;
  /* noise generator fields */
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_PERIOD) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_NOISE))
    return 1;
  /* filter fields */
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_CUTOFF) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_FILTER))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_KEYTRACK) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_FILTER))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_RESONANCE) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_FILTER))
    return 1;
  /* reverb fields */
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_DELAY)   &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_REVERB)   &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_VIBRATO)  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_TREMOLO)  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_WOBBLE))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_C_0) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_REVERB))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_C_1) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_REVERB))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_C_2) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_REVERB))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_C_3) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_REVERB))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_C_4) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_REVERB))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_C_5) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_REVERB))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_C_6) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_REVERB))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_C_7) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_REVERB))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_FEEDBACK) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_REVERB))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_VOLUME)  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_REVERB)   &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_STEP))
    return 1;
  /* envelope fields */
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_AR)                &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_AMPLITUDE_ENVELOPE) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_FILTER_ENVELOPE))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_DR)                &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_AMPLITUDE_ENVELOPE) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_FILTER_ENVELOPE))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_SR)                &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_AMPLITUDE_ENVELOPE) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_FILTER_ENVELOPE))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_RR)                &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_AMPLITUDE_ENVELOPE) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_FILTER_ENVELOPE))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_TL)                &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_AMPLITUDE_ENVELOPE) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_FILTER_ENVELOPE))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_SL)                &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_AMPLITUDE_ENVELOPE) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_FILTER_ENVELOPE))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_RKS)               &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_AMPLITUDE_ENVELOPE) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_FILTER_ENVELOPE))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_LKS)               &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_AMPLITUDE_ENVELOPE) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_FILTER_ENVELOPE))
    return 1;
  /* lfo fields */
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_DEPTH)   &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_VIBRATO)  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_TREMOLO)  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_WOBBLE))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_SPEED)   &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_VIBRATO)  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_TREMOLO)  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_WOBBLE))
    return 1;
  /* sequencer fields */
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_MEASURE) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_SEQUENCER))
    return 1;
  /* measure fields */
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_STEP) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_MEASURE))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_LENGTH) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_MEASURE))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_BEAT) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_MEASURE))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_SUBDIVISIONS)  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_MEASURE)        &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_ARPEGGIATOR))
    return 1;
  /* step fields */
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_SCALE) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_STEP))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_CHORD) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_STEP))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_ARPEGGIATOR) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_STEP))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_POSITION) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_STEP))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_OCTAVE) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_STEP))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_DURATION)  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_STEP)       &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_ARPEGGIATOR))
    return 1;
  /* scale fields */
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_NAME) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_SCALE))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_TONIC) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_SCALE))
    return 1;
  /* chord fields */
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_NOTE_1) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_CHORD))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_NOTE_2) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_CHORD))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_NOTE_3) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_CHORD))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_NOTE_4) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_CHORD))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_NOTE_5) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_CHORD))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_NOTE_6) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_CHORD))
    return 1;
  /* arpeggiator fields */
  else if ( (current_type == DATA_TREE_NODE_TYPE_FIELD_MODE) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_ARPEGGIATOR))
    return 1;
  /* attributes */
  else if ( (current_type == DATA_TREE_NODE_TYPE_ATTRIBUTE_BPM) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_IDUNNO))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_ATTRIBUTE_EXPORT_SAMPLING) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_IDUNNO))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_ATTRIBUTE_EXPORT_BITRES) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_IDUNNO))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_ATTRIBUTE_DOWNSAMPLING_M) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_IDUNNO))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_ATTRIBUTE_TUNING_SYSTEM) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_IDUNNO))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_ATTRIBUTE_TUNING_FORK) &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_IDUNNO))
    return 1;
  /* values */
  else if ( (current_type == DATA_TREE_NODE_TYPE_VALUE_INTEGER)             &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_HPF)                  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_SOFT_CLIP)            &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_PHI)                  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_SYNC)                 &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_MIX)                  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_RING_MOD)             &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_DETUNE_OCTAVE)        &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_DETUNE_COARSE)        &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_DETUNE_FINE)          &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_PERIOD)               &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_KEYTRACK)             &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_RESONANCE)            &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_DELAY)                &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_C_0)                  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_C_1)                  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_C_2)                  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_C_3)                  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_C_4)                  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_C_5)                  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_C_6)                  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_C_7)                  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_FEEDBACK)             &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_VOLUME)               &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_AR)                   &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_DR)                   &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_SR)                   &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_RR)                   &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_TL)                   &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_SL)                   &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_RKS)                  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_LKS)                  &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_DEPTH)                &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_SPEED)                &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_LENGTH)               &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_BEAT)                 &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_SUBDIVISIONS)         &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_OCTAVE)               &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_DURATION)             &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_NOTE_1)               &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_NOTE_2)               &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_NOTE_3)               &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_NOTE_4)               &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_NOTE_5)               &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_NOTE_6)               &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_MODE)                 &&
            (parent_type != DATA_TREE_NODE_TYPE_ATTRIBUTE_BPM)              &&
            (parent_type != DATA_TREE_NODE_TYPE_ATTRIBUTE_EXPORT_SAMPLING)  &&
            (parent_type != DATA_TREE_NODE_TYPE_ATTRIBUTE_EXPORT_BITRES)    &&
            (parent_type != DATA_TREE_NODE_TYPE_ATTRIBUTE_DOWNSAMPLING_M))
    return 1;
  else if ( (current_type == DATA_TREE_NODE_TYPE_VALUE_STRING)            &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_WAVEFORM)           &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_CUTOFF)             &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_POSITION)           &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_NAME)               &&
            (parent_type != DATA_TREE_NODE_TYPE_FIELD_TONIC)              &&
            (parent_type != DATA_TREE_NODE_TYPE_ATTRIBUTE_TUNING_SYSTEM)  &&
            (parent_type != DATA_TREE_NODE_TYPE_ATTRIBUTE_TUNING_FORK))
    return 1;

  return 0;
}

/*******************************************************************************
** parse_data_tree_lookup_midi_note()
*******************************************************************************/
char parse_data_tree_lookup_midi_note(char* name)
{
  char note;

  if (name == NULL)
    return -1;

  /* make sure this string is the proper size */
  if ((name[0] == '\0') || (name[1] == '\0') || 
      (name[2] == '\0') || (name[3] != '\0'))
  {
    printf("Invalid note specified.\n");
    return -1;
  }

  /* lookup octave */
  if (name[2] == '0')
    note = 12;
  else if (name[2] == '1')
    note = 24;
  else if (name[2] == '2')
    note = 36;
  else if (name[2] == '3')
    note = 48;
  else if (name[2] == '4')
    note = 60;
  else if (name[2] == '5')
    note = 72;
  else if (name[2] == '6')
    note = 84;
  else if (name[2] == '7')
    note = 96;
  else if (name[2] == '8')
    note = 108;
  else if (name[2] == '9')
    note = 120;
  else
  {
    printf("Invalid note specified.\n");
    return -1;
  }

  /* lookup note */
  if ((name[0] == 'c') && (name[1] == '_'))
    note += 0;
  else if ((name[0] == 'c') && (name[1] == '#'))
    note += 1;
  else if ((name[0] == 'd') && (name[1] == '_'))
    note += 2;
  else if ((name[0] == 'd') && (name[1] == '#'))
    note += 3;
  else if ((name[0] == 'e') && (name[1] == '_'))
    note += 4;
  else if ((name[0] == 'f') && (name[1] == '_'))
    note += 5;
  else if ((name[0] == 'f') && (name[1] == '#'))
    note += 6;
  else if ((name[0] == 'g') && (name[1] == '_'))
    note += 7;
  else if (((name[0] == 'g') && (name[1] == '#')) && (note != 120))
    note += 8;
  else if (((name[0] == 'a') && (name[1] == '_')) && (note != 120))
    note += 9;
  else if (((name[0] == 'a') && (name[1] == '#')) && (note != 120))
    note += 10;
  else if (((name[0] == 'b') && (name[1] == '_')) && (note != 120))
    note += 11;
  else
  {
    printf("Invalid note specified.\n");
    return -1;
  }

  return note;
}

/*******************************************************************************
** parse_data_tree_load_integer()
*******************************************************************************/
short int parse_data_tree_load_integer(int val, int parent_type, int grand_type)
{
  int       num;

  patch*    p;
  measure*  m;
  step*     st;

  p = &G_synth.p;

  m = &G_sequencer.measures[G_sequencer.num_measures - 1];
  st = &m->steps[m->num_steps - 1];

  /* highpass filter */
  if (parent_type == DATA_TREE_NODE_TYPE_FIELD_HPF)
  {
    if ((val >= 0) && (val <= 7))
      p->hpf = (unsigned char) val;
    else
    {
      printf("Invalid Highpass Filter specified. Defaulting to 0.\n");
      p->hpf = 0;
    }
  }
  /* soft clip */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_SOFT_CLIP)
  {
    if ((val >= 0) && (val <= 1))
      p->soft_clip = val;
    else
    {
      printf("Invalid Soft Clip specified. Defaulting to 0.\n");
      p->soft_clip = 0;
    }
  }
  /* phi */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_PHI)
  {
    if ((val >= 0) && (val <= 7))
      p->phi = (unsigned char) val;
    else
    {
      printf("Invalid Phi specified. Defaulting to 0.\n");
      p->phi = 0;
    }
  }
  /* sync */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_SYNC)
  {
    if ((val >= 0) && (val <= 3))
      p->sync = val;
    else
    {
      printf("Invalid Sync specified. Defaulting to 0.\n");
      p->sync = 0;
    }
  }
  /* mix */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_MIX)
  {
    if (grand_type == DATA_TREE_NODE_TYPE_FIELD_GENERATOR)
    {
      if ((val >= 0) && (val <= 31))
        p->wave_mix = val;
      else
      {
        printf("Invalid Wave Mix specified. Defaulting to 16.\n");
        p->wave_mix = 16;
      }
    }
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_NOISE)
    {
      if ((val >= 0) && (val <= 31))
        p->noise_mix = val;
      else
      {
        printf("Invalid Noise Mix specified. Defaulting to 16.\n");
        p->noise_mix = 16;
      }
    }
  }
  /* ring mod */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_RING_MOD)
  {
    if ((val >= 0) && (val <= 1))
      p->ring_mod = val;
    else
    {
      printf("Invalid Ring Mod specified. Defaulting to 0.\n");
      p->ring_mod = 0;
    }
  }
  /* detune octave */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_DETUNE_OCTAVE)
  {
    /* determine oscillator */
    if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OSC_1)
      num = 0;
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OSC_2)
      num = 1;
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OSC_3)
      num = 2;
    else
      num = 0;

    /* set octave */
    if ((val >= 0) && (val <= 3))
      p->detune_octave[num] = val;
    else
    {
      printf("Invalid Detune Octave specified. Defaulting to 0.\n");
      p->detune_octave[num] = 0;
    }
  }
  /* detune coarse (semitones) */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_DETUNE_COARSE)
  {
    /* determine oscillator */
    if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OSC_1)
      num = 0;
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OSC_2)
      num = 1;
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OSC_3)
      num = 2;
    else
      num = 0;

    /* set detune coarse */
    if ((val >= 0) && (val <= 15))
      p->detune_coarse[num] = val;
    else
    {
      printf("Invalid Detune Coarse specified. Defaulting to 0.\n");
      p->detune_coarse[num] = 0;
    }
  }
  /* detune fine (cents) */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_DETUNE_FINE)
  {
    /* determine oscillator */
    if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OSC_1)
      num = 0;
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OSC_2)
      num = 1;
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OSC_3)
      num = 2;
    else
      num = 0;

    /* set detune fine */
    if ((val >= 0) && (val <= 63))
      p->detune_fine[num] = val;
    else
    {
      printf("Invalid Detune Fine specified. Defaulting to 0.\n");
      p->detune_fine[num] = 0;
    }
  }
  /* noise period */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_PERIOD)
  {
    if ((val >= 0) && (val <= 15))
      p->noise_period = val;
    else
    {
      printf("Invalid Noise Period specified. Defaulting to 0.\n");
      p->noise_period = 0;
    }
  }
  /* keytrack */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_KEYTRACK)
  {
    if ((val >= 0) && (val <= 7))
      p->keytrack = val;
    else
    {
      printf("Invalid Keytrack specified. Defaulting to 0.\n");
      p->keytrack = 0;
    }
  }
  /* resonance */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_RESONANCE)
  {
    if ((val >= 0) && (val <= 31))
      p->resonance = val;
    else
    {
      printf("Invalid Resonance specified. Defaulting to 0.\n");
      p->resonance = 0;
    }
  }
  /* delay */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_DELAY)
  {
    /* reverb delay */
    if (grand_type == DATA_TREE_NODE_TYPE_FIELD_REVERB)
    {
      if ((val >= 0) && (val <= 63))
        p->rev_delay = val;
      else
      {
        printf("Invalid Reverb Delay specified. Defaulting to 0.\n");
        p->rev_delay = 0;
      }
    }
    /* lfo delay */
    else
    {
      /* determine modulator */
      if (grand_type == DATA_TREE_NODE_TYPE_FIELD_VIBRATO)
        num = 0;
      else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_TREMOLO)
        num = 1;
      else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_WOBBLE)
        num = 2;
      else
        num = 0;

      /* set delay */
      if ((val >= 0) && (val <= 7))
        p->mod_delay[num] = val;
      else
      {
        printf("Invalid LFO Delay specified. Defaulting to 0.\n");
        p->mod_delay[num] = 0;
      }
    }
  }
  /* coefficient 0 */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_C_0)
  {
    if ((val >= -128) && (val <= 127))
      p->rev_c[0] = val;
    else
    {
      printf("Invalid Reverb C0 specified. Defaulting to 0.\n");
      p->rev_c[0] = 0;
    }
  }
  /* coefficient 1 */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_C_1)
  {
    if ((val >= -128) && (val <= 127))
      p->rev_c[1] = val;
    else
    {
      printf("Invalid Reverb C1 specified. Defaulting to 0.\n");
      p->rev_c[1] = 0;
    }
  }
  /* coefficient 2 */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_C_2)
  {
    if ((val >= -128) && (val <= 127))
      p->rev_c[2] = val;
    else
    {
      printf("Invalid Reverb C2 specified. Defaulting to 0.\n");
      p->rev_c[2] = 0;
    }
  }
  /* coefficient 3 */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_C_3)
  {
    if ((val >= -128) && (val <= 127))
      p->rev_c[3] = val;
    else
    {
      printf("Invalid Reverb C3 specified. Defaulting to 0.\n");
      p->rev_c[3] = 0;
    }
  }
  /* coefficient 4 */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_C_4)
  {
    if ((val >= -128) && (val <= 127))
      p->rev_c[4] = val;
    else
    {
      printf("Invalid Reverb C4 specified. Defaulting to 0.\n");
      p->rev_c[4] = 0;
    }
  }
  /* coefficient 5 */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_C_5)
  {
    if ((val >= -128) && (val <= 127))
      p->rev_c[5] = val;
    else
    {
      printf("Invalid Reverb C5 specified. Defaulting to 0.\n");
      p->rev_c[5] = 0;
    }
  }
  /* coefficient 6 */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_C_6)
  {
    if ((val >= -128) && (val <= 127))
      p->rev_c[6] = val;
    else
    {
      printf("Invalid Reverb C6 specified. Defaulting to 0.\n");
      p->rev_c[6] = 0;
    }
  }
  /* coefficient 7 */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_C_7)
  {
    if ((val >= -128) && (val <= 127))
      p->rev_c[7] = val;
    else
    {
      printf("Invalid Reverb C7 specified. Defaulting to 0.\n");
      p->rev_c[7] = 0;
    }
  }
  /* feedback */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_FEEDBACK)
  {
    if ((val >= -128) && (val <= 127))
      p->rev_feedback = val;
    else
    {
      printf("Invalid Reverb Feedback specified. Defaulting to 0.\n");
      p->rev_feedback = 0;
    }
  }
  /* volume */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_VOLUME)
  {
    /* reverb volume */
    if (grand_type == DATA_TREE_NODE_TYPE_FIELD_REVERB)
    {
      if ((val >= 0) && (val <= 127))
        p->rev_vol = val;
      else
      {
        printf("Invalid Reverb Volume specified. Defaulting to 0.\n");
        p->rev_vol = 0;
      }
    }
    /* step volume */
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_STEP)
    {
      if ((val >= 0) && (val <= 127))
        st->volume = val;
      else
      {
        printf("Invalid Sequencer Step Volume specified. Defaulting to 0.\n");
        st->volume = 0;
      }
    }
  }
  /* ar */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_AR)
  {
    /* determine envelope */
    if (grand_type == DATA_TREE_NODE_TYPE_FIELD_AMPLITUDE_ENVELOPE)
      num = 0;
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_FILTER_ENVELOPE)
      num = 1;
    else
      num = 0;

    /* set ar */
    if ((val >= 0) && (val <= 31))
      p->ar[num] = (unsigned char) val;
    else
    {
      printf("Invalid AR specified. Defaulting to 0.\n");
      p->ar[num] = 0;
    }
  }
  /* dr */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_DR)
  {
    /* determine envelope */
    if (grand_type == DATA_TREE_NODE_TYPE_FIELD_AMPLITUDE_ENVELOPE)
      num = 0;
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_FILTER_ENVELOPE)
      num = 1;
    else
      num = 0;

    /* set dr */
    if ((val >= 0) && (val <= 31))
      p->dr[num] = (unsigned char) val;
    else
    {
      printf("Invalid DR specified. Defaulting to 0.\n");
      p->dr[num] = 0;
    }
  }
  /* sr */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_SR)
  {
    /* determine envelope */
    if (grand_type == DATA_TREE_NODE_TYPE_FIELD_AMPLITUDE_ENVELOPE)
      num = 0;
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_FILTER_ENVELOPE)
      num = 1;
    else
      num = 0;

    /* set sr */
    if ((val >= 0) && (val <= 31))
      p->sr[num] = (unsigned char) val;
    else
    {
      printf("Invalid SR specified. Defaulting to 0.\n");
      p->sr[num] = 0;
    }
  }
  /* rr */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_RR)
  {
    /* determine envelope */
    if (grand_type == DATA_TREE_NODE_TYPE_FIELD_AMPLITUDE_ENVELOPE)
      num = 0;
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_FILTER_ENVELOPE)
      num = 1;
    else
      num = 0;

    /* set rr */
    if ((val >= 0) && (val <= 15))
      p->rr[num] = (unsigned char) val;
    else
    {
      printf("Invalid RR specified. Defaulting to 0.\n");
      p->rr[num] = 0;
    }
  }
  /* tl */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_TL)
  {
    /* determine envelope */
    if (grand_type == DATA_TREE_NODE_TYPE_FIELD_AMPLITUDE_ENVELOPE)
      num = 0;
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_FILTER_ENVELOPE)
      num = 1;
    else
      num = 0;

    /* set tl */
    if ((val >= 0) && (val <= 127))
      p->tl[num] = (unsigned char) val;
    else
    {
      printf("Invalid TL specified. Defaulting to 0.\n");
      p->tl[num] = 0;
    }
  }
  /* sl */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_SL)
  {
    /* determine envelope */
    if (grand_type == DATA_TREE_NODE_TYPE_FIELD_AMPLITUDE_ENVELOPE)
      num = 0;
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_FILTER_ENVELOPE)
      num = 1;
    else
      num = 0;

    /* set sl */
    if ((val >= 0) && (val <= 15))
      p->sl[num] = (unsigned char) val;
    else
    {
      printf("Invalid SL specified. Defaulting to 0.\n");
      p->sl[num] = 0;
    }
  }
  /* rks */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_RKS)
  {
    /* determine envelope */
    if (grand_type == DATA_TREE_NODE_TYPE_FIELD_AMPLITUDE_ENVELOPE)
      num = 0;
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_FILTER_ENVELOPE)
      num = 1;
    else
      num = 0;

    /* set rks */
    if ((val >= 0) && (val <= 3))
      p->rks[num] = val;
    else
    {
      printf("Invalid RKS specified. Defaulting to 0.\n");
      p->rks[num] = 0;
    }
  }
  /* lks */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_LKS)
  {
    /* determine envelope */
    if (grand_type == DATA_TREE_NODE_TYPE_FIELD_AMPLITUDE_ENVELOPE)
      num = 0;
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_FILTER_ENVELOPE)
      num = 1;
    else
      num = 0;

    /* set lks */
    if ((val >= 0) && (val <= 3))
      p->lks[num] = val;
    else
    {
      printf("Invalid LKS specified. Defaulting to 0.\n");
      p->lks[num] = 0;
    }
  }
  /* depth */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_DEPTH)
  {
    /* determine modulator */
    if (grand_type == DATA_TREE_NODE_TYPE_FIELD_VIBRATO)
      num = 0;
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_TREMOLO)
      num = 1;
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_WOBBLE)
      num = 2;
    else
      num = 0;

    /* set depth */
    if ((val >= 0) && (val <= 15))
      p->mod_depth[num] = val;
    else
    {
      printf("Invalid LFO Depth specified. Defaulting to 0.\n");
      p->mod_depth[num] = 0;
    }
  }
  /* speed */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_SPEED)
  {
    /* determine modulator */
    if (grand_type == DATA_TREE_NODE_TYPE_FIELD_VIBRATO)
      num = 0;
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_TREMOLO)
      num = 1;
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_WOBBLE)
      num = 2;
    else
      num = 0;

    /* set speed */
    if ((val >= 0) && (val <= 15))
      p->mod_speed[num] = val;
    else
    {
      printf("Invalid LFO Speed specified. Defaulting to 0.\n");
      p->mod_speed[num] = 0;
    }
  }
  /* measure length */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_LENGTH)
  {
    if ((val >= 1) && (val <= 16))
      m->length = val;
    else
    {
      printf("Invalid Measure Length specified. Defaulting to 4.\n");
      m->length = 4;
    }
  }
  /* beat */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_BEAT)
  {
    if ((val == 2) || (val == 4) || (val == 8)|| (val == 16))
      m->beat = val;
    else
    {
      printf("Invalid Beat specified. Defaulting to 4 (Quarter Note).\n");
      m->beat = 4;
    }
  }
  /* subdivisions */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_SUBDIVISIONS)
  {
    if (grand_type == DATA_TREE_NODE_TYPE_FIELD_MEASURE)
    {
      if ((val == 1)  || (val == 2)   || (val == 3)   || (val == 4) || 
          (val == 5)  || (val == 6)   || (val == 7)   || (val == 8) || 
          (val == 10) || (val == 12)  || (val == 14)  || (val == 16))
      {
        m->subdivisions = val;
      }
      else
      {
        printf("Invalid Measure Subdivisions specified. Defaulting to 1.\n");
        m->subdivisions = 1;
      }
    }
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_ARPEGGIATOR)
    {
      if ((val == 1)  || (val == 2)   || (val == 3)   || (val == 4) || 
          (val == 5)  || (val == 6)   || (val == 7)   || (val == 8) || 
          (val == 10) || (val == 12)  || (val == 14)  || (val == 16))
      {
        st->arp_subdivisions = val;
      }
      else
      {
        printf("Invalid Arpeggiator Subdivisions specified. Defaulting to 1.\n");
        st->arp_subdivisions = 1;
      }
    }
  }
  /* staff octave */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_OCTAVE)
  {
    if ((val >= 2) && (val <= 6))
      st->staff_octave = val;
    else
    {
      printf("Invalid Staff Octave specified. Defaulting to 4.\n");
      st->staff_octave = 4;
    }
  }
  /* duration */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_DURATION)
  {
    if (grand_type == DATA_TREE_NODE_TYPE_FIELD_STEP)
    {
      if ((val >= 1) && (val <= 16))
        st->duration = val;
      else
      {
        printf("Invalid Step Duration specified. Defaulting to 1.\n");
        st->duration = 1;
      }
    }
    else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_ARPEGGIATOR)
    {
      if ((val >= 1) && (val <= 16))
        st->arp_duration = val;
      else
      {
        printf("Invalid Arpeggiator Step Duration specified. Defaulting to 1.\n");
        st->arp_duration = 1;
      }
    }
  }
  /* note 1 */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_NOTE_1)
  {
    if ((val >= -7) && (val <= 8))
      st->chord_notes[0] = val;
    else
    {
      printf("Invalid Chord Note 1 specified. Defaulting to 0 (none).\n");
      st->chord_notes[0] = 0;
    }
  }
  /* note 2 */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_NOTE_2)
  {
    if ((val >= -7) && (val <= 8))
      st->chord_notes[1] = val;
    else
    {
      printf("Invalid Chord Note 2 specified. Defaulting to 0 (none).\n");
      st->chord_notes[1] = 0;
    }
  }
  /* note 3 */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_NOTE_3)
  {
    if ((val >= -7) && (val <= 8))
      st->chord_notes[2] = val;
    else
    {
      printf("Invalid Chord Note 3 specified. Defaulting to 0 (none).\n");
      st->chord_notes[2] = 0;
    }
  }
  /* note 4 */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_NOTE_4)
  {
    if ((val >= -7) && (val <= 8))
      st->chord_notes[3] = val;
    else
    {
      printf("Invalid Chord Note 4 specified. Defaulting to 0 (none).\n");
      st->chord_notes[3] = 0;
    }
  }
  /* note 5 */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_NOTE_5)
  {
    if ((val >= -7) && (val <= 8))
      st->chord_notes[4] = val;
    else
    {
      printf("Invalid Chord Note 5 specified. Defaulting to 0 (none).\n");
      st->chord_notes[4] = 0;
    }
  }
  /* note 6 */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_NOTE_6)
  {
    if ((val >= -7) && (val <= 8))
      st->chord_notes[5] = val;
    else
    {
      printf("Invalid Chord Note 6 specified. Defaulting to 0 (none).\n");
      st->chord_notes[5] = 0;
    }
  }
  /* mode */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_MODE)
  {
    if ((val >= 0) && (val <= 16))
      st->arp_mode = val;
    else
    {
      printf("Invalid Arpeggiator Mode specified. Defaulting to 0.\n");
      st->arp_mode = 0;
    }
  }
  /* bpm */
  else if (parent_type == DATA_TREE_NODE_TYPE_ATTRIBUTE_BPM)
  {
    if ((val >= 32) && (val <= 255))
      G_bpm = val;
    else
    {
      printf("Invalid BPM specified. Defaulting to 120.\n");
      G_bpm = 120;
    }
  }
  /* export sampling rate */
  else if (parent_type == DATA_TREE_NODE_TYPE_ATTRIBUTE_EXPORT_SAMPLING)
  {
    if (val == 8363)
    {
      G_export_sampling = 8363;
      G_export_period = 119574;
    }
    else if (val == 16726)
    {
      G_export_sampling = 16726;
      G_export_period = 59787;
    }
    else if (val == 22050)
    {
      G_export_sampling = 22050;
      G_export_period = 45351;
    }
    else if (val == 44100)
    {
      G_export_sampling = 44100;
      G_export_period = 22676;
    }
    else if (val == 53267)
    {
      G_export_sampling = 53267;
      G_export_period = 18773;
    }
    else
    {
      printf("Invalid export sampling rate specified. Defaulting to 44100 hz.\n");
      G_export_sampling = 44100;
      G_export_period = 22676;
    }
  }
  /* export bit resolution */
  else if (parent_type == DATA_TREE_NODE_TYPE_ATTRIBUTE_EXPORT_BITRES)
  {
    if ((val == 8) || (val == 16))
      G_export_bitres = val;
    else
    {
      printf("Invalid export bitres specified. Defaulting to 16 bit.\n");
      G_export_bitres = 16;
    }
  }
  /* downsampling m */
  else if (parent_type == DATA_TREE_NODE_TYPE_ATTRIBUTE_DOWNSAMPLING_M)
  {
    if ((val == 64) || (val == 128) || (val == 256) || (val == 512))
    {
      G_downsampling_m = val;
      G_downsampling_bound = (G_downsampling_m / 2) + 1;
    }
    else
    {
      printf("Invalid downsampling M specified. Defaulting to 128.\n");
      G_downsampling_m = 128;
      G_downsampling_bound = (G_downsampling_m / 2) + 1;
    }
  }

  return 0;
}

/*******************************************************************************
** parse_data_tree_load_string()
*******************************************************************************/
short int parse_data_tree_load_string(char* name, 
                                      int parent_type, int grand_type)
{
  int       num;

  patch*    p;
  measure*  m;
  step*     st;

  if (name == NULL)
    return 1;

  p = &G_synth.p;

  m = &G_sequencer.measures[G_sequencer.num_measures - 1];
  st = &m->steps[m->num_steps - 1];

  /* waveform */
  if (parent_type == DATA_TREE_NODE_TYPE_FIELD_WAVEFORM)
  {
    /* oscillator waveform */
    if ((grand_type == DATA_TREE_NODE_TYPE_FIELD_OSC_1) ||
        (grand_type == DATA_TREE_NODE_TYPE_FIELD_OSC_2))
    {
      /* determine oscillator */
      if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OSC_1)
        num = 0;
      else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_OSC_2)
        num = 1;
      else
        num = 0;

      /* set waveform */
      if (!strcmp(name, "square"))
        p->waveform[num] = OSC_WAVEFORM_SQUARE;
      else if (!strcmp(name, "saw"))
        p->waveform[num] = OSC_WAVEFORM_SAW;
      else if (!strcmp(name, "triangle"))
        p->waveform[num] = OSC_WAVEFORM_TRIANGLE;
      else if (!strcmp(name, "pulse_1_8"))
        p->waveform[num] = OSC_WAVEFORM_PULSE_1_8;
      else if (!strcmp(name, "pulse_1_4"))
        p->waveform[num] = OSC_WAVEFORM_PULSE_1_4;
      else if (!strcmp(name, "pulse_3_8"))
        p->waveform[num] = OSC_WAVEFORM_PULSE_3_8;
      else if (!strcmp(name, "pulse_1_16"))
        p->waveform[num] = OSC_WAVEFORM_PULSE_1_16;
      else if (!strcmp(name, "pulse_3_16"))
        p->waveform[num] = OSC_WAVEFORM_PULSE_3_16;
      else if (!strcmp(name, "pulse_5_16"))
        p->waveform[num] = OSC_WAVEFORM_PULSE_5_16;
      else if (!strcmp(name, "pulse_7_16"))
        p->waveform[num] = OSC_WAVEFORM_PULSE_7_16;
      else
      {
        printf("Invalid Oscillator Waveform specified. Defaulting to Square.\n");
        p->waveform[num] = OSC_WAVEFORM_SQUARE;
      }
    }
    /* lfo waveform */
    else if ( (grand_type == DATA_TREE_NODE_TYPE_FIELD_VIBRATO) ||
              (grand_type == DATA_TREE_NODE_TYPE_FIELD_TREMOLO) ||
              (grand_type == DATA_TREE_NODE_TYPE_FIELD_WOBBLE))
    {
      /* determine modulator */
      if (grand_type == DATA_TREE_NODE_TYPE_FIELD_VIBRATO)
        num = 0;
      else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_TREMOLO)
        num = 1;
      else if (grand_type == DATA_TREE_NODE_TYPE_FIELD_WOBBLE)
        num = 2;
      else
        num = 0;

      /* set waveform */
      if (!strcmp(name, "sine"))
        p->mod_waveform[num] = LFO_WAVEFORM_SINE;
      else if (!strcmp(name, "square"))
        p->mod_waveform[num] = LFO_WAVEFORM_SQUARE;
      else if (!strcmp(name, "triangle"))
        p->mod_waveform[num] = LFO_WAVEFORM_TRIANGLE;
      else if (!strcmp(name, "saw_up"))
        p->mod_waveform[num] = LFO_WAVEFORM_SAW_UP;
      else if (!strcmp(name, "saw_down"))
        p->mod_waveform[num] = LFO_WAVEFORM_SAW_DOWN;
      else if (!strcmp(name, "noise"))
        p->mod_waveform[num] = LFO_WAVEFORM_NOISE;
      else
      {
        printf("Invalid LFO Waveform specified. Defaulting to Sine.\n");
        p->mod_waveform[num] = LFO_WAVEFORM_SINE;
      }
    }
  }
  /* cutoff */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_CUTOFF)
  {
    p->cutoff = parse_data_tree_lookup_midi_note(name);

    if (p->cutoff == -1)
    {
      printf("Invalid Filter Cutoff specified. Defaulting to G9.\n");
      p->cutoff = 127;
    }
  }
  /* staff position */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_POSITION)
  {
    if (!strcmp(name, "c"))
      st->staff_position = 1;
    else if (!strcmp(name, "d"))
      st->staff_position = 2;
    else if (!strcmp(name, "e"))
      st->staff_position = 3;
    else if (!strcmp(name, "f"))
      st->staff_position = 4;
    else if (!strcmp(name, "g"))
      st->staff_position = 5;
    else if (!strcmp(name, "a"))
      st->staff_position = 6;
    else if (!strcmp(name, "b"))
      st->staff_position = 7;
    else if (!strcmp(name, "rest"))
      st->staff_position = 0;
    else
    {
      printf("Invalid Staff Position specified. Defaulting to Rest.\n");
      st->staff_position = 0;
    }
  }
  /* name */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_NAME)
  {
    /* scale name */
    if ((!strcmp(name, "ionian")) || (!strcmp(name, "major")))
      st->scale_name = SCALE_NAME_IONIAN;
    else if (!strcmp(name, "dorian"))
      st->scale_name = SCALE_NAME_DORIAN;
    else if (!strcmp(name, "phrygian"))
      st->scale_name = SCALE_NAME_PHRYGIAN;
    else if (!strcmp(name, "lydian"))
      st->scale_name = SCALE_NAME_LYDIAN;
    else if (!strcmp(name, "mixolydian"))
      st->scale_name = SCALE_NAME_MIXOLYDIAN;
    else if ((!strcmp(name, "aeolian")) || (!strcmp(name, "minor")))
      st->scale_name = SCALE_NAME_AEOLIAN;
    else if (!strcmp(name, "locrian"))
      st->scale_name = SCALE_NAME_LOCRIAN;
    else if (!strcmp(name, "melodic_minor"))
      st->scale_name = SCALE_NAME_MELODIC_MINOR;
    else if (!strcmp(name, "melodic_minor_2nd_mode"))
      st->scale_name = SCALE_NAME_MELODIC_MINOR_2ND_MODE;
    else if ((!strcmp(name, "melodic_minor_3rd_mode")) || (!strcmp(name, "lydian_augmented")))
      st->scale_name = SCALE_NAME_MELODIC_MINOR_3RD_MODE;
    else if ((!strcmp(name, "melodic_minor_4th_mode")) || (!strcmp(name, "lydian_dominant")))
      st->scale_name = SCALE_NAME_MELODIC_MINOR_4TH_MODE;
    else if ((!strcmp(name, "melodic_minor_5th_mode")) || (!strcmp(name, "major_minor")))
      st->scale_name = SCALE_NAME_MELODIC_MINOR_5TH_MODE;
    else if ((!strcmp(name, "melodic_minor_6th_mode")) || (!strcmp(name, "half_diminished")))
      st->scale_name = SCALE_NAME_MELODIC_MINOR_6TH_MODE;
    else if (!strcmp(name, "melodic_minor_7th_mode"))
      st->scale_name = SCALE_NAME_MELODIC_MINOR_7TH_MODE;
    else if (!strcmp(name, "harmonic_minor"))
      st->scale_name = SCALE_NAME_HARMONIC_MINOR;
    else if (!strcmp(name, "harmonic_minor_2nd_mode"))
      st->scale_name = SCALE_NAME_HARMONIC_MINOR_2ND_MODE;
    else if ((!strcmp(name, "harmonic_minor_3rd_mode")) || (!strcmp(name, "major_augmented")))
      st->scale_name = SCALE_NAME_HARMONIC_MINOR_3RD_MODE;
    else if ((!strcmp(name, "harmonic_minor_4th_mode")) || (!strcmp(name, "lydian_diminished")))
      st->scale_name = SCALE_NAME_HARMONIC_MINOR_4TH_MODE;
    else if ((!strcmp(name, "harmonic_minor_5th_mode")) || (!strcmp(name, "phrygian_dominant")))
      st->scale_name = SCALE_NAME_HARMONIC_MINOR_5TH_MODE;
    else if (!strcmp(name, "harmonic_minor_6th_mode"))
      st->scale_name = SCALE_NAME_HARMONIC_MINOR_6TH_MODE;
    else if (!strcmp(name, "harmonic_minor_7th_mode"))
      st->scale_name = SCALE_NAME_HARMONIC_MINOR_7TH_MODE;
    else if (!strcmp(name, "harmonic_major"))
      st->scale_name = SCALE_NAME_HARMONIC_MAJOR;
    else if (!strcmp(name, "harmonic_major_2nd_mode"))
      st->scale_name = SCALE_NAME_HARMONIC_MAJOR_2ND_MODE;
    else if (!strcmp(name, "harmonic_major_3rd_mode"))
      st->scale_name = SCALE_NAME_HARMONIC_MAJOR_3RD_MODE;
    else if (!strcmp(name, "harmonic_major_4th_mode"))
      st->scale_name = SCALE_NAME_HARMONIC_MAJOR_4TH_MODE;
    else if (!strcmp(name, "harmonic_major_5th_mode"))
      st->scale_name = SCALE_NAME_HARMONIC_MAJOR_5TH_MODE;
    else if (!strcmp(name, "harmonic_major_6th_mode"))
      st->scale_name = SCALE_NAME_HARMONIC_MAJOR_6TH_MODE;
    else if (!strcmp(name, "harmonic_major_7th_mode"))
      st->scale_name = SCALE_NAME_HARMONIC_MAJOR_7TH_MODE;
    else if (!strcmp(name, "double_harmonic"))
      st->scale_name = SCALE_NAME_DOUBLE_HARMONIC;
    else if (!strcmp(name, "double_harmonic_2nd_mode"))
      st->scale_name = SCALE_NAME_DOUBLE_HARMONIC_2ND_MODE;
    else if (!strcmp(name, "double_harmonic_3rd_mode"))
      st->scale_name = SCALE_NAME_DOUBLE_HARMONIC_3RD_MODE;
    else if ((!strcmp(name, "double_harmonic_4th_mode")) || (!strcmp(name, "double_harmonic_minor")))
      st->scale_name = SCALE_NAME_DOUBLE_HARMONIC_4TH_MODE;
    else if ((!strcmp(name, "double_harmonic_5th_mode")) || (!strcmp(name, "oriental")))
      st->scale_name = SCALE_NAME_DOUBLE_HARMONIC_5TH_MODE;
    else if (!strcmp(name, "double_harmonic_6th_mode"))
      st->scale_name = SCALE_NAME_DOUBLE_HARMONIC_6TH_MODE;
    else if (!strcmp(name, "double_harmonic_7th_mode"))
      st->scale_name = SCALE_NAME_DOUBLE_HARMONIC_7TH_MODE;
    else if (!strcmp(name, "neapolitan_minor"))
      st->scale_name = SCALE_NAME_NEAPOLITAN_MINOR;
    else if (!strcmp(name, "neapolitan_minor_2nd_mode"))
      st->scale_name = SCALE_NAME_NEAPOLITAN_MINOR_2ND_MODE;
    else if (!strcmp(name, "neapolitan_minor_3rd_mode"))
      st->scale_name = SCALE_NAME_NEAPOLITAN_MINOR_3RD_MODE;
    else if ((!strcmp(name, "neapolitan_minor_4th_mode")) || (!strcmp(name, "ukrainian_dorian")))
      st->scale_name = SCALE_NAME_NEAPOLITAN_MINOR_4TH_MODE;
    else if ((!strcmp(name, "neapolitan_minor_5th_mode")) || (!strcmp(name, "locrian_dominant")))
      st->scale_name = SCALE_NAME_NEAPOLITAN_MINOR_5TH_MODE;
    else if (!strcmp(name, "neapolitan_minor_6th_mode"))
      st->scale_name = SCALE_NAME_NEAPOLITAN_MINOR_6TH_MODE;
    else if (!strcmp(name, "neapolitan_minor_7th_mode"))
      st->scale_name = SCALE_NAME_NEAPOLITAN_MINOR_7TH_MODE;
    else if (!strcmp(name, "neapolitan_major"))
      st->scale_name = SCALE_NAME_NEAPOLITAN_MAJOR;
    else if ((!strcmp(name, "neapolitan_major_2nd_mode")) || (!strcmp(name, "leading_whole_tone")))
      st->scale_name = SCALE_NAME_NEAPOLITAN_MAJOR_2ND_MODE;
    else if (!strcmp(name, "neapolitan_major_3rd_mode"))
      st->scale_name = SCALE_NAME_NEAPOLITAN_MAJOR_3RD_MODE;
    else if ((!strcmp(name, "neapolitan_major_4th_mode")) || (!strcmp(name, "lydian_minor")))
      st->scale_name = SCALE_NAME_NEAPOLITAN_MAJOR_4TH_MODE;
    else if ((!strcmp(name, "neapolitan_major_5th_mode")) || (!strcmp(name, "major_locrian")))
      st->scale_name = SCALE_NAME_NEAPOLITAN_MAJOR_5TH_MODE;
    else if (!strcmp(name, "neapolitan_major_6th_mode"))
      st->scale_name = SCALE_NAME_NEAPOLITAN_MAJOR_6TH_MODE;
    else if (!strcmp(name, "neapolitan_major_7th_mode"))
      st->scale_name = SCALE_NAME_NEAPOLITAN_MAJOR_7TH_MODE;
    else if (!strcmp(name, "hungarian_major"))
      st->scale_name = SCALE_NAME_HUNGARIAN_MAJOR;
    else if (!strcmp(name, "hungarian_major_2nd_mode"))
      st->scale_name = SCALE_NAME_HUNGARIAN_MAJOR_2ND_MODE;
    else if (!strcmp(name, "hungarian_major_3rd_mode"))
      st->scale_name = SCALE_NAME_HUNGARIAN_MAJOR_3RD_MODE;
    else if (!strcmp(name, "hungarian_major_4th_mode"))
      st->scale_name = SCALE_NAME_HUNGARIAN_MAJOR_4TH_MODE;
    else if (!strcmp(name, "hungarian_major_5th_mode"))
      st->scale_name = SCALE_NAME_HUNGARIAN_MAJOR_5TH_MODE;
    else if (!strcmp(name, "hungarian_major_6th_mode"))
      st->scale_name = SCALE_NAME_HUNGARIAN_MAJOR_6TH_MODE;
    else if (!strcmp(name, "hungarian_major_7th_mode"))
      st->scale_name = SCALE_NAME_HUNGARIAN_MAJOR_7TH_MODE;
    else if (!strcmp(name, "romanian_major"))
      st->scale_name = SCALE_NAME_ROMANIAN_MAJOR;
    else if (!strcmp(name, "romanian_major_2nd_mode"))
      st->scale_name = SCALE_NAME_ROMANIAN_MAJOR_2ND_MODE;
    else if (!strcmp(name, "romanian_major_3rd_mode"))
      st->scale_name = SCALE_NAME_ROMANIAN_MAJOR_3RD_MODE;
    else if (!strcmp(name, "romanian_major_4th_mode"))
      st->scale_name = SCALE_NAME_ROMANIAN_MAJOR_4TH_MODE;
    else if (!strcmp(name, "romanian_major_5th_mode"))
      st->scale_name = SCALE_NAME_ROMANIAN_MAJOR_5TH_MODE;
    else if (!strcmp(name, "romanian_major_6th_mode"))
      st->scale_name = SCALE_NAME_ROMANIAN_MAJOR_6TH_MODE;
    else if (!strcmp(name, "romanian_major_7th_mode"))
      st->scale_name = SCALE_NAME_ROMANIAN_MAJOR_7TH_MODE;
    else if (!strcmp(name, "persian"))
      st->scale_name = SCALE_NAME_PERSIAN;
    else if (!strcmp(name, "persian_2nd_mode"))
      st->scale_name = SCALE_NAME_PERSIAN_2ND_MODE;
    else if (!strcmp(name, "persian_3rd_mode"))
      st->scale_name = SCALE_NAME_PERSIAN_3RD_MODE;
    else if (!strcmp(name, "persian_4th_mode"))
      st->scale_name = SCALE_NAME_PERSIAN_4TH_MODE;
    else if (!strcmp(name, "persian_5th_mode"))
      st->scale_name = SCALE_NAME_PERSIAN_5TH_MODE;
    else if (!strcmp(name, "persian_6th_mode"))
      st->scale_name = SCALE_NAME_PERSIAN_6TH_MODE;
    else if (!strcmp(name, "persian_7th_mode"))
      st->scale_name = SCALE_NAME_PERSIAN_7TH_MODE;
    else if (!strcmp(name, "enigmatic"))
      st->scale_name = SCALE_NAME_ENIGMATIC;
    else if (!strcmp(name, "enigmatic_2nd_mode"))
      st->scale_name = SCALE_NAME_ENIGMATIC_2ND_MODE;
    else if (!strcmp(name, "enigmatic_3rd_mode"))
      st->scale_name = SCALE_NAME_ENIGMATIC_3RD_MODE;
    else if (!strcmp(name, "enigmatic_4th_mode"))
      st->scale_name = SCALE_NAME_ENIGMATIC_4TH_MODE;
    else if (!strcmp(name, "enigmatic_5th_mode"))
      st->scale_name = SCALE_NAME_ENIGMATIC_5TH_MODE;
    else if (!strcmp(name, "enigmatic_6th_mode"))
      st->scale_name = SCALE_NAME_ENIGMATIC_6TH_MODE;
    else if (!strcmp(name, "enigmatic_7th_mode"))
      st->scale_name = SCALE_NAME_ENIGMATIC_7TH_MODE;
    else if (!strcmp(name, "kanakangi"))
      st->scale_name = SCALE_NAME_KANAKANGI;
    else if (!strcmp(name, "kanakangi_2nd_mode"))
      st->scale_name = SCALE_NAME_KANAKANGI_2ND_MODE;
    else if (!strcmp(name, "kanakangi_3rd_mode"))
      st->scale_name = SCALE_NAME_KANAKANGI_3RD_MODE;
    else if (!strcmp(name, "kanakangi_4th_mode"))
      st->scale_name = SCALE_NAME_KANAKANGI_4TH_MODE;
    else if (!strcmp(name, "kanakangi_5th_mode"))
      st->scale_name = SCALE_NAME_KANAKANGI_5TH_MODE;
    else if (!strcmp(name, "kanakangi_6th_mode"))
      st->scale_name = SCALE_NAME_KANAKANGI_6TH_MODE;
    else if (!strcmp(name, "kanakangi_7th_mode"))
      st->scale_name = SCALE_NAME_KANAKANGI_7TH_MODE;
    else
    {
      printf("Invalid Scale Name specified. Defaulting to Ionian (Major).\n");
      st->scale_name = SCALE_NAME_IONIAN;
    }
  }
  /* tonic */
  else if (parent_type == DATA_TREE_NODE_TYPE_FIELD_TONIC)
  {
    if (!strcmp(name, "c"))
      st->scale_tonic = SCALE_TONIC_C;
    else if (!strcmp(name, "c_sharp"))
      st->scale_tonic = SCALE_TONIC_C_SHARP;
    else if (!strcmp(name, "c_flat"))
      st->scale_tonic = SCALE_TONIC_C_FLAT;
    else if (!strcmp(name, "d"))
      st->scale_tonic = SCALE_TONIC_D;
    else if (!strcmp(name, "d_sharp"))
      st->scale_tonic = SCALE_TONIC_D_SHARP;
    else if (!strcmp(name, "d_flat"))
      st->scale_tonic = SCALE_TONIC_D_FLAT;
    else if (!strcmp(name, "e"))
      st->scale_tonic = SCALE_TONIC_E;
    else if (!strcmp(name, "e_sharp"))
      st->scale_tonic = SCALE_TONIC_E_SHARP;
    else if (!strcmp(name, "e_flat"))
      st->scale_tonic = SCALE_TONIC_E_FLAT;
    else if (!strcmp(name, "f"))
      st->scale_tonic = SCALE_TONIC_F;
    else if (!strcmp(name, "f_sharp"))
      st->scale_tonic = SCALE_TONIC_F_SHARP;
    else if (!strcmp(name, "f_flat"))
      st->scale_tonic = SCALE_TONIC_F_FLAT;
    else if (!strcmp(name, "g"))
      st->scale_tonic = SCALE_TONIC_G;
    else if (!strcmp(name, "g_sharp"))
      st->scale_tonic = SCALE_TONIC_G_SHARP;
    else if (!strcmp(name, "g_flat"))
      st->scale_tonic = SCALE_TONIC_G_FLAT;
    else if (!strcmp(name, "a"))
      st->scale_tonic = SCALE_TONIC_A;
    else if (!strcmp(name, "a_sharp"))
      st->scale_tonic = SCALE_TONIC_A_SHARP;
    else if (!strcmp(name, "a_flat"))
      st->scale_tonic = SCALE_TONIC_A_FLAT;
    else if (!strcmp(name, "b"))
      st->scale_tonic = SCALE_TONIC_B;
    else if (!strcmp(name, "b_sharp"))
      st->scale_tonic = SCALE_TONIC_B_SHARP;
    else if (!strcmp(name, "b_flat"))
      st->scale_tonic = SCALE_TONIC_B_FLAT;
    else
    {
      printf("Invalid Scale Tonic specified. Defaulting to C.\n");
      st->scale_tonic = SCALE_TONIC_C;
    }
  }
  /* tuning system */
  else if (parent_type == DATA_TREE_NODE_TYPE_ATTRIBUTE_TUNING_SYSTEM)
  {
    if (!strcmp(name, "equal_temperament"))
      G_tuning_system = TUNING_SYSTEM_12_ET;
    else if (!strcmp(name, "pythagorean"))
      G_tuning_system = TUNING_SYSTEM_PYTHAGOREAN;
    else if (!strcmp(name, "quarter_comma_meantone"))
      G_tuning_system = TUNING_SYSTEM_QC_MEANTONE;
    else if (!strcmp(name, "just_intonation"))
      G_tuning_system = TUNING_SYSTEM_JUST;
    else if (!strcmp(name, "werckmeister_iii"))
      G_tuning_system = TUNING_SYSTEM_WERCKMEISTER_III;
    else if (!strcmp(name, "werckmeister_iv"))
      G_tuning_system = TUNING_SYSTEM_WERCKMEISTER_IV;
    else if (!strcmp(name, "werckmeister_v"))
      G_tuning_system = TUNING_SYSTEM_WERCKMEISTER_V;
    else if (!strcmp(name, "werckmeister_vi"))
      G_tuning_system = TUNING_SYSTEM_WERCKMEISTER_VI;
    else if (!strcmp(name, "renold_i"))
      G_tuning_system = TUNING_SYSTEM_RENOLD_I;
    else
    {
      printf("Invalid tuning system specified. Defaulting to Equal Temperament.\n");
      G_tuning_system = TUNING_SYSTEM_12_ET;
    }
  }
  /* tuning fork */
  else if (parent_type == DATA_TREE_NODE_TYPE_ATTRIBUTE_TUNING_FORK)
  {
    if (!strcmp(name, "a440"))
      G_tuning_fork = TUNING_FORK_A440;
    else if (!strcmp(name, "a432"))
      G_tuning_fork = TUNING_FORK_A432;
    else if (!strcmp(name, "c256"))
      G_tuning_fork = TUNING_FORK_C256;
    else if (!strcmp(name, "amiga"))
      G_tuning_fork = TUNING_FORK_AMIGA;
    else
    {
      printf("Invalid tuning fork specified. Defaulting to A440.\n");
      G_tuning_fork = TUNING_FORK_A440;
    }
  }

  return 0;
}

/*******************************************************************************
** parse_data_tree_to_globals()
*******************************************************************************/
short int parse_data_tree_to_globals(data_tree_node* root)
{
  data_tree_node*   current;
  int               current_type;
  int               parent_type;
  int               grand_type;
  int               great_type;
  data_tree_node**  stack;
  int               stack_size;
  int               stack_top;

  if (root == NULL)
    return 1;

  /* setup stack */
  stack = malloc(DATA_TREE_STACK_INITIAL_SIZE * sizeof(data_tree_node*));
  stack_size = DATA_TREE_STACK_INITIAL_SIZE;
  stack_top = -1;

  /* verify root node */
  if ((root->type != DATA_TREE_NODE_TYPE_FIELD_IDUNNO)  ||
      (root->sibling != NULL)                           ||
      (root->child == NULL))
  {
    printf("Invalid root node.\n");
    goto houston;
  }

  /* push root node onto stack and begin traversing the subtree */
  DATA_TREE_PUSH_NODE(stack, root)
  current = root->child;

  while(stack_top >= 0)
  {
    current_type = current->type;
    parent_type = stack[stack_top]->type;

    if (stack_top == 0)
      grand_type = DATA_TREE_NODE_TYPE_NONE;
    else
      grand_type = stack[stack_top - 1]->type;

    if (stack_top <= 1)
      great_type = DATA_TREE_NODE_TYPE_NONE;
    else
      great_type = stack[stack_top - 2]->type;

    /* semantic analysis */
    if (parse_data_tree_semantic_analysis(current_type, parent_type))
    {
      printf("Semantic analysis failed.\n");
      goto houston;
    }

    /* process this node */
    if (current_type == DATA_TREE_NODE_TYPE_FIELD_MEASURE)
    {
      G_sequencer.num_measures += 1;

      if (G_sequencer.num_measures > SEQUENCER_MAX_MEASURES)
      {
        printf("Too many sequencer measures defined.\n");
        goto houston;
      }
    }
    else if (current_type == DATA_TREE_NODE_TYPE_FIELD_STEP)
    {
      G_sequencer.measures[G_sequencer.num_measures - 1].num_steps += 1;

      if (G_sequencer.measures[G_sequencer.num_measures - 1].num_steps > SEQUENCER_MAX_STEPS)
      {
        printf("Too many pattern steps defined.\n");
        goto houston;
      }
    }
    else if (current_type == DATA_TREE_NODE_TYPE_VALUE_INTEGER)
    {
      parse_data_tree_load_integer( strtol(current->value, NULL, 10),
                                    parent_type, grand_type);
    }
    else if (current_type == DATA_TREE_NODE_TYPE_VALUE_STRING)
    {
      parse_data_tree_load_string(current->value, parent_type, grand_type);
    }

    /* go to next node */
    if (current->child != NULL)
    {
      DATA_TREE_PUSH_NODE(stack, current)
      current = current->child;
    }
    else if (current->sibling != NULL)
    {
      current = current->sibling;
    }
    else
    {
      current = stack[stack_top];
      DATA_TREE_POP_NODE(stack)

      while ((stack_top >= 0) && (current->sibling == NULL))
      {
        current = stack[stack_top];
        DATA_TREE_POP_NODE(stack)
      }

      if (stack_top == -1)
        current = NULL;
      else
        current = current->sibling;
    }
  }

  goto cleanup;

  /* error handling */
houston:
  synth_deinit(&G_synth);
  sequencer_deinit(&G_sequencer);

  /* cleanup */
cleanup:
  if (stack != NULL)
  {
    free(stack);
    stack = NULL;
  }

  return 0;
}

