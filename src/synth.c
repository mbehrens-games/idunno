/*******************************************************************************
** synth.c (individual synth)
*******************************************************************************/

#include <stdlib.h>
#include <math.h>

#include "clock.h"
#include "envelope.h"
#include "filter.h"
#include "global.h"
#include "lfo.h"
#include "patch.h"
#include "reverb.h"
#include "shaping.h"
#include "synth.h"
#include "voice.h"

/*******************************************************************************
** synth_init()
*******************************************************************************/
short int synth_init(synth* s)
{
  int i;

  if (s == NULL)
    return 1;

  /* patch */
  patch_init(&s->p);

  /* voices */
  for (i = 0; i < SYNTH_MAX_VOICES; i++)
    voice_init(&s->v[i]);

  /* highpass filter */
  filter_init(&s->highpass);

  /* reverb */
  reverb_init(&s->r);

  /* output level */
  s->level = 0;

  return 0;
}

/*******************************************************************************
** synth_create()
*******************************************************************************/
synth* synth_create()
{
  synth* s;

  s = malloc(sizeof(synth));
  synth_init(s);

  return s;
}

/*******************************************************************************
** synth_deinit()
*******************************************************************************/
short int synth_deinit(synth* s)
{
  int i;

  if (s == NULL)
    return 1;

  patch_deinit(&s->p);

  for (i = 0; i < SYNTH_MAX_VOICES; i++)
    voice_deinit(&s->v[i]);

  filter_deinit(&s->highpass);

  reverb_deinit(&s->r);

  return 0;
}

/*******************************************************************************
** synth_destroy()
*******************************************************************************/
short int synth_destroy(synth* s)
{
  if (s == NULL)
    return 1;

  synth_deinit(s);
  free(s);

  return 0;
}

/*******************************************************************************
** synth_setup()
*******************************************************************************/
short int synth_setup(synth* s)
{
  int     i;

  patch*  p;

  if (s == NULL)
    return 1;

  p = &s->p;

  /* set patch pointer in each voice */
  for (i = 0; i < SYNTH_MAX_VOICES; i++)
    s->v[i].p = &s->p;

  /* setup highpass filter                                            */
  /* settings: 0 is off, then 1-7 are D#1, A1, D#2, A2, D#3, A3, D#4  */
  if (p->hpf == 1)
    filter_set_indices(&s->highpass, 27 * 32, 0);
  else if (p->hpf == 2)
    filter_set_indices(&s->highpass, 33 * 32, 0);
  else if (p->hpf == 3)
    filter_set_indices(&s->highpass, 39 * 32, 0);
  else if (p->hpf == 4)
    filter_set_indices(&s->highpass, 45 * 32, 0);
  else if (p->hpf == 5)
    filter_set_indices(&s->highpass, 51 * 32, 0);
  else if (p->hpf == 6)
    filter_set_indices(&s->highpass, 57 * 32, 0);
  else if (p->hpf == 7)
    filter_set_indices(&s->highpass, 63 * 32, 0);
  else
    filter_set_indices(&s->highpass, 21 * 32, 0);

  /* setup reverb */
  reverb_setup(&s->r, p->rev_delay, p->rev_c, p->rev_feedback, p->rev_vol);

  /* reset output level */
  s->level = 0;

  return 0;
}

/*******************************************************************************
** synth_key_on()
*******************************************************************************/
short int synth_key_on(synth* s, int voice_num, char note, char vol)
{
  if (s == NULL)
    return 1;

  /* if voice number is invalid, ignore */
  if ((voice_num < 0) || (voice_num >= SYNTH_MAX_VOICES))
    return 0;

  /* if note is out of the range A0 to C8, ignore */
  if ((note < 21) || (note > 108))
    return 0;

  /* if volume is out of range, ignore */
  if ((vol < 0) || (vol > 127))
    return 0;

  /* send key on command to voice */
  voice_key_on(&s->v[voice_num], note, vol);

  return 0;
}

/*******************************************************************************
** synth_key_off()
*******************************************************************************/
short int synth_key_off(synth* s, int voice_num)
{
  if (s == NULL)
    return 1;

  /* if voice number is invalid, ignore */
  if ((voice_num < 0) || (voice_num >= SYNTH_MAX_VOICES))
    return 0;

  /* send key off command to voice */
  voice_key_off(&s->v[voice_num]);

  return 0;
}

/*******************************************************************************
** synth_update()
*******************************************************************************/
short int synth_update(synth* s)
{
  int     i;

  patch*  p;

  int     level;

  if (s == NULL)
    return 1;

  p = &s->p;

  /* update voices */
  for (i = 0; i < SYNTH_MAX_VOICES; i++)
    voice_update(&s->v[i]);

  /* compute level */
  level = 0;

  for (i = 0; i < SYNTH_MAX_VOICES; i++)
    level += s->v[i].level;

  /* highpass filter */
  if (p->hpf != 0)
  {
    filter_update_highpass(&s->highpass, level);

    level = s->highpass.level;
  }

  /* reverb */
  reverb_update(&s->r, level);

  level = s->r.level;

  /* soft clipping */
  if (p->soft_clip == 1)
  {
    if (level > 32767 - 2)
      level = 32767;
    else if (level < -32767 + 2)
      level = -32767;
    else if (level >= 0)
      level = G_waveshaper_tanh_table[(level + 2) / 4];
    else
      level = -G_waveshaper_tanh_table[(-level + 2) / 4];
  }
  /* hard clipping */
  else
  {
    if (level > 32767)
      level = 32767;
    else if (level < -32767)
      level = -32767;
  }

  /* apply master volume */
  /*level = (level * s->volume) / 128;*/

  /* set voice level */
  s->level = level;

  return 0;
}

