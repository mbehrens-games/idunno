/*******************************************************************************
** datatree.h (data tree node)
*******************************************************************************/

#ifndef DATA_TREE_H
#define DATA_TREE_H

#define DATA_TREE_STACK_INITIAL_SIZE  10

#define DATA_TREE_PUSH_NODE(stack, node)                                       \
  if (stack##_top >= stack##_size - 1)                                         \
  {                                                                            \
    stack = realloc(stack,                                                     \
                    (stack##_size + DATA_TREE_STACK_INITIAL_SIZE) *            \
                    sizeof(data_tree_node*));                                  \
  }                                                                            \
                                                                               \
  stack[++stack##_top] = node;

#define DATA_TREE_POP_NODE(stack)                                              \
  if (stack##_top >= 0)                                                        \
    stack##_top--;

#define DATA_TREE_CREATE_NEW_NODE(stack, current)                              \
  if (current == stack[stack##_top])                                           \
  {                                                                            \
    current->child = data_tree_node_create();                                  \
    current = current->child;                                                  \
  }                                                                            \
  else                                                                         \
  {                                                                            \
    current->sibling = data_tree_node_create();                                \
    current = current->sibling;                                                \
  }

enum
{
  DATA_TREE_NODE_TYPE_NONE,
  /* root fields */
  DATA_TREE_NODE_TYPE_FIELD_IDUNNO,
  /* top level fields */
  DATA_TREE_NODE_TYPE_FIELD_GENERATOR,
  DATA_TREE_NODE_TYPE_FIELD_NOISE,
  DATA_TREE_NODE_TYPE_FIELD_FILTER,
  DATA_TREE_NODE_TYPE_FIELD_REVERB,
  DATA_TREE_NODE_TYPE_FIELD_AMPLITUDE_ENVELOPE,
  DATA_TREE_NODE_TYPE_FIELD_FILTER_ENVELOPE,
  DATA_TREE_NODE_TYPE_FIELD_VIBRATO,
  DATA_TREE_NODE_TYPE_FIELD_TREMOLO,
  DATA_TREE_NODE_TYPE_FIELD_WOBBLE,
  DATA_TREE_NODE_TYPE_FIELD_HPF,
  DATA_TREE_NODE_TYPE_FIELD_SOFT_CLIP,
  DATA_TREE_NODE_TYPE_FIELD_SEQUENCER,
  /* waveform generator fields */
  DATA_TREE_NODE_TYPE_FIELD_OSC_1,
  DATA_TREE_NODE_TYPE_FIELD_OSC_2,
  DATA_TREE_NODE_TYPE_FIELD_OSC_3,
  DATA_TREE_NODE_TYPE_FIELD_PHI,
  DATA_TREE_NODE_TYPE_FIELD_SYNC,
  DATA_TREE_NODE_TYPE_FIELD_MIX,
  DATA_TREE_NODE_TYPE_FIELD_RING_MOD,
  /* oscillator fields */
  DATA_TREE_NODE_TYPE_FIELD_WAVEFORM,
  DATA_TREE_NODE_TYPE_FIELD_DETUNE_OCTAVE,
  DATA_TREE_NODE_TYPE_FIELD_DETUNE_COARSE,
  DATA_TREE_NODE_TYPE_FIELD_DETUNE_FINE,
  /* noise generator fields */
  DATA_TREE_NODE_TYPE_FIELD_PERIOD,
  /* filter fields */
  DATA_TREE_NODE_TYPE_FIELD_CUTOFF,
  DATA_TREE_NODE_TYPE_FIELD_KEYTRACK,
  DATA_TREE_NODE_TYPE_FIELD_RESONANCE,
  /* reverb fields */
  DATA_TREE_NODE_TYPE_FIELD_DELAY,
  DATA_TREE_NODE_TYPE_FIELD_C_0,
  DATA_TREE_NODE_TYPE_FIELD_C_1,
  DATA_TREE_NODE_TYPE_FIELD_C_2,
  DATA_TREE_NODE_TYPE_FIELD_C_3,
  DATA_TREE_NODE_TYPE_FIELD_C_4,
  DATA_TREE_NODE_TYPE_FIELD_C_5,
  DATA_TREE_NODE_TYPE_FIELD_C_6,
  DATA_TREE_NODE_TYPE_FIELD_C_7,
  DATA_TREE_NODE_TYPE_FIELD_FEEDBACK,
  DATA_TREE_NODE_TYPE_FIELD_VOLUME,
  /* envelope fields */
  DATA_TREE_NODE_TYPE_FIELD_AR,
  DATA_TREE_NODE_TYPE_FIELD_DR,
  DATA_TREE_NODE_TYPE_FIELD_SR,
  DATA_TREE_NODE_TYPE_FIELD_RR,
  DATA_TREE_NODE_TYPE_FIELD_TL,
  DATA_TREE_NODE_TYPE_FIELD_SL,
  DATA_TREE_NODE_TYPE_FIELD_RKS,
  DATA_TREE_NODE_TYPE_FIELD_LKS,
  /* lfo fields */
  DATA_TREE_NODE_TYPE_FIELD_DEPTH,
  DATA_TREE_NODE_TYPE_FIELD_SPEED,
  /* sequencer fields */
  DATA_TREE_NODE_TYPE_FIELD_MEASURE,
  /* measure fields */
  DATA_TREE_NODE_TYPE_FIELD_STEP,
  DATA_TREE_NODE_TYPE_FIELD_LENGTH,
  DATA_TREE_NODE_TYPE_FIELD_BEAT,
  DATA_TREE_NODE_TYPE_FIELD_SUBDIVISIONS,
  /* step fields */
  DATA_TREE_NODE_TYPE_FIELD_SCALE,
  DATA_TREE_NODE_TYPE_FIELD_CHORD,
  DATA_TREE_NODE_TYPE_FIELD_ARPEGGIATOR,
  DATA_TREE_NODE_TYPE_FIELD_POSITION,
  DATA_TREE_NODE_TYPE_FIELD_OCTAVE,
  DATA_TREE_NODE_TYPE_FIELD_DURATION,
  /* scale fields */
  DATA_TREE_NODE_TYPE_FIELD_NAME,
  DATA_TREE_NODE_TYPE_FIELD_TONIC,
  /* chord fields */
  DATA_TREE_NODE_TYPE_FIELD_NOTE_1,
  DATA_TREE_NODE_TYPE_FIELD_NOTE_2,
  DATA_TREE_NODE_TYPE_FIELD_NOTE_3,
  DATA_TREE_NODE_TYPE_FIELD_NOTE_4,
  DATA_TREE_NODE_TYPE_FIELD_NOTE_5,
  DATA_TREE_NODE_TYPE_FIELD_NOTE_6,
  /* arpeggiator fields */
  DATA_TREE_NODE_TYPE_FIELD_MODE,
  /* attributes */
  DATA_TREE_NODE_TYPE_ATTRIBUTE_BPM,
  DATA_TREE_NODE_TYPE_ATTRIBUTE_EXPORT_SAMPLING,
  DATA_TREE_NODE_TYPE_ATTRIBUTE_EXPORT_BITRES,
  DATA_TREE_NODE_TYPE_ATTRIBUTE_DOWNSAMPLING_M,
  DATA_TREE_NODE_TYPE_ATTRIBUTE_TUNING_SYSTEM,
  DATA_TREE_NODE_TYPE_ATTRIBUTE_TUNING_FORK,
  /* values */
  DATA_TREE_NODE_TYPE_VALUE_INTEGER,
  DATA_TREE_NODE_TYPE_VALUE_FLOAT,
  DATA_TREE_NODE_TYPE_VALUE_STRING
};

typedef struct data_tree_node
{
  int   type;
  char* value;

  struct data_tree_node*  child;
  struct data_tree_node*  sibling;
} data_tree_node;

/* function declarations */
short int       data_tree_node_init(data_tree_node* node);
data_tree_node* data_tree_node_create();
short int       data_tree_node_deinit(data_tree_node* node);
short int       data_tree_node_destroy(data_tree_node* node);
short int       data_tree_node_destroy_tree(data_tree_node* node);

#endif
