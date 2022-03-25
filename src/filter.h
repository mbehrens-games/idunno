/*******************************************************************************
** filter.h (filter)
*******************************************************************************/

#ifndef FILTER_H
#define FILTER_H

typedef struct filter
{
  int   fc_index;
  int   res_index;

  int   s[2];
  int   v[2];
  int   y[2];

  int   level;
} filter;

/* function declarations */
short int filter_init(filter* fltr);
filter*   filter_create();
short int filter_deinit(filter* fltr);
short int filter_destroy(filter* fltr);

short int filter_set_indices(filter* fltr, int fc_index, int res_index);

short int filter_reset(filter* fltr);
short int filter_update_highpass(filter* fltr, int input);
short int filter_update_lowpass(filter* fltr, int input);

#endif
