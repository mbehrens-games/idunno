/*******************************************************************************
** reverb.h (snes-style reverb)
*******************************************************************************/

#ifndef REVERB_H
#define REVERB_H

typedef struct reverb
{
  char  c[8];
  char  feedback;

  int   ring_buffer[512 * 64];
  int   write_index;
  int   read_index;

  int   x[8];
  int   y[2];

  char  volume;

  int   level;
} reverb;

/* function declarations */
short int   reverb_init(reverb* r);
reverb*     reverb_create();
short int   reverb_deinit(reverb* r);
short int   reverb_destroy(reverb* r);

short int   reverb_setup(reverb* r, char delay, 
                                    char* c, 
                                    char feedback, 
                                    char vol);
short int   reverb_update(reverb* r, int input);

#endif
