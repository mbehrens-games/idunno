/*******************************************************************************
** IDUNN-O! - Michael Behrens 2020
*******************************************************************************/

/*******************************************************************************
** main.c
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "clock.h"
#include "datatree.h"
#include "downsamp.h"
#include "export.h"
#include "global.h"
#include "parse.h"
#include "sequence.h"
#include "shaping.h"
#include "synth.h"
#include "tuning.h"
#include "synth.h"
#include "waveform.h"

/*******************************************************************************
** main()
*******************************************************************************/
int main(int argc, char *argv[])
{
  int   i;

  char* name;
  char  input_filename[256];
  char  output_filename[256];

  data_tree_node* root;

  int sample_index;
  int time_elapsed;

  float export_length;

  short int* sample_buffer;
  short int* export_buffer;

  int sample_buffer_size;
  int export_buffer_size;

  /* initialization */
  i = 0;

  name = NULL;
  root = NULL;

  sample_index = 0;

  sample_buffer = NULL;
  export_buffer = NULL;

  /* read command line arguments */
  i = 1;

  while (i < argc)
  {
    /* name */
    if (!strcmp(argv[i], "-n"))
    {
      i++;
      if (i >= argc)
      {
        printf("Insufficient number of arguments. ");
        printf("Expected name. Exiting...\n");
        return 0;
      }

      name = strdup(argv[i]);
      i++;
    }
    else
    {
      printf("Unknown command line argument %s. Exiting...\n", argv[i]);
      return 0;
    }
  }

  /* make sure name is defined */
  if (name == NULL)
  {
    printf("Name not defined. Exiting...\n");
    return 0;
  }

  /* determine input and output filenames */
  strncpy(input_filename, name, 252);
  strncat(input_filename, ".txt", 4);

  strncpy(output_filename, name, 252);
  strncat(output_filename, ".wav", 4);

  /* setup */
  globals_init();
  export_init();

  /* read input file */
  root = parse_file_to_data_tree(input_filename);

  if (root == NULL)
  {
    printf("Data tree not created from input file. Exiting...\n");
    goto cleanup;
  }

  parse_data_tree_to_globals(root);
  data_tree_node_destroy_tree(root);
  root = NULL;

  /* initialize tables */
  tuning_generate_tables();
  shaping_generate_tables();
  lfo_generate_tables();
  waveform_generate_tables();
  sequencer_generate_tables();
  downsamp_compute_sinc_filter();

  /* determine buffer sizes */
  export_length = sequencer_calculate_length(&G_sequencer);

  sample_buffer_size = (int) (export_length * GENESIS_PER_OP_FM_CLOCK);
  export_buffer_size = (int) (export_length * G_export_sampling);

  /* allocate buffers */
  sample_buffer = malloc(sizeof(short int) * sample_buffer_size);

  if (sample_buffer == NULL)
    goto cleanup;

  export_buffer = malloc(sizeof(short int) * export_buffer_size);

  if (export_buffer == NULL)
    goto cleanup;

  /* setup synth, reset sequencer */
  synth_setup(&G_synth);
  sequencer_reset(&G_sequencer);

  /* sound generation start */
  sample_index = 0;
  time_elapsed = 0;

  sequencer_activate_step(&G_sequencer, &G_synth);

  while (sample_index < sample_buffer_size)
  {
    /* update sequencer */
    if (time_elapsed >= G_sequencer_period_table[G_bpm - 32])
    {
      sequencer_ahead_one_tick(&G_sequencer, &G_synth);
      time_elapsed -= G_sequencer_period_table[G_bpm - 32];
    }

    /* update voice */
    synth_update(&G_synth);

    /* add sample to buffer */
    if (G_synth.level > 32767)
      sample_buffer[sample_index] = 32767;
    else if (G_synth.level < -32767)
      sample_buffer[sample_index] = -32767;
    else
      sample_buffer[sample_index] = (short int) G_synth.level;

    sample_index += 1;

    /* update time elapsed */
    time_elapsed += GENESIS_DELTA_T_NANOSECONDS;
  }

  /* downsample */
  if (G_export_sampling != 53267)
  {
    downsamp_apply_filter(sample_buffer, sample_buffer_size);

    downsamp_perform_downsample(sample_buffer, sample_buffer_size, 
                                export_buffer, export_buffer_size);
  }
  else
  {
    memcpy(export_buffer, sample_buffer, export_buffer_size);
  }

  /* open output file */
  if (export_open_file(output_filename))
  {
    fprintf(stdout, "Output file not opened. Exiting...\n");
    goto cleanup;
  }

  /* write to file */
  export_write_header(export_buffer_size);
  export_write_block(export_buffer, export_buffer_size);

  /* close output file */
  export_close_file();

  /* cleanup */
cleanup:
  if (sample_buffer != NULL)
  {
    free(sample_buffer);
    sample_buffer = NULL;
  }

  if (export_buffer != NULL)
  {
    free(export_buffer);
    export_buffer = NULL;
  }

  export_deinit();
  globals_deinit();

  return 0;
}
