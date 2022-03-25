/*******************************************************************************
** reverb.h (snes-style reverb)
*******************************************************************************/

#include <stdlib.h>

#include "reverb.h"

/*******************************************************************************
** reverb_init()
*******************************************************************************/
short int reverb_init(reverb* r)
{
  int i;

  if (r == NULL)
    return 1;

  for (i = 0; i < 8; i++)
    r->c[i] = 0;

  r->feedback = 0;

  for (i = 0; i < 512 * 64; i++)
    r->ring_buffer[i] = 0;

  r->write_index  = 0;
  r->read_index   = 0;

  for (i = 0; i < 8; i++)
    r->x[i] = 0;

  for (i = 0; i < 2; i++)
    r->y[i] = 0;

  r->volume = 127;

  r->level = 0;

  return 0;
}

/*******************************************************************************
** reverb_create()
*******************************************************************************/
reverb* reverb_create()
{
  reverb* r;

  r = malloc(sizeof(reverb));
  reverb_init(r);

  return r;
}

/*******************************************************************************
** reverb_deinit()
*******************************************************************************/
short int reverb_deinit(reverb* r)
{
  if (r == NULL)
    return 1;

  return 0;
}

/*******************************************************************************
** reverb_destroy()
*******************************************************************************/
short int reverb_destroy(reverb* r)
{
  if (r == NULL)
    return 1;

  reverb_deinit(r);
  free(r);

  return 0;
}

/*******************************************************************************
** reverb_setup()
*******************************************************************************/
short int reverb_setup(reverb* r, char delay, 
                                  char* c, 
                                  char feedback, 
                                  char vol)
{
  int i;

  if (r == NULL)
    return 1;

  /* clear buffers */
  for (i = 0; i < 512 * 64; i++)
    r->ring_buffer[i] = 0;

  for (i = 0; i < 8; i++)
    r->x[i] = 0;

  for (i = 0; i < 2; i++)
    r->y[i] = 0;

  /* determine starting read index */
  if ((delay < 0) || (delay > 63))
    r->read_index = 0;
  else if (delay == 0)
    r->read_index = 0;
  else
    r->read_index = (64 - delay) * 512;

  /* set starting write index */
  r->write_index = 0;

  /* set coefficients */
  for (i = 0; i < 8; i++)
    r->c[i] = c[i];

  /* set feedback */
  r->feedback = feedback;

  /* set volume */
  if ((vol < 0) || (vol > 127))
    r->volume = 0;
  else
    r->volume = vol;

  /* set level */
  r->level = 0;

  return 0;
}

/*******************************************************************************
** reverb_update()
*******************************************************************************/
short int reverb_update(reverb* r, int input)
{
  int i;

  if (r == NULL)
    return 1;

  /* shift input window                     */
  /* note that index 0 is the oldest sample */
  for (i = 0; i < 7; i++)
    r->x[i] = r->x[i + 1];

  /* update ring buffer */
  r->ring_buffer[r->write_index] = input;

  /* use (possibly) delayed input as next input to the window */
  r->x[7] = r->ring_buffer[r->read_index];

  /* shift output window */
  r->y[0] = r->y[1];

  /* compute level */
  r->y[1] = ((r->x[0] * r->c[0]) / 128) + 
            ((r->x[1] * r->c[1]) / 128) + 
            ((r->x[2] * r->c[2]) / 128) + 
            ((r->x[3] * r->c[3]) / 128) + 
            ((r->x[4] * r->c[4]) / 128) + 
            ((r->x[5] * r->c[5]) / 128) + 
            ((r->x[6] * r->c[6]) / 128) + 
            ((r->x[7] * r->c[7]) / 128) + 
            ((r->y[0] * r->feedback) / 128);

  /* set output level */
  r->level = input + ((r->y[1] * r->volume) / 128);

  /* update ring buffer indices */
  r->write_index = (r->write_index + 1) % (512 * 64);
  r->read_index = (r->read_index + 1) % (512 * 64);

  return 0;
}

