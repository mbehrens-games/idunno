/*******************************************************************************
** clock.h (clock rates)
*******************************************************************************/

#ifndef CLOCK_H
#define CLOCK_H

/* Genesis: FM Clock is CPU Clock / 6, EG Clock is FM Clock / 3               */
/* 6 Channels with 4 Operators each means that the Per Op Clocks are 1/24     */
/* 1 Hz Phase Increment is 2^28 (size of phase counter) / Per Op FM Clock     */
#define GENESIS_CPU_CLOCK             7670455.0f
#define GENESIS_FM_CLOCK              1278409.66666666667f

/*
#define GENESIS_PER_OP_FM_CLOCK       53267.0486111111111
#define GENESIS_1HZ_PHASE_INCREMENT   5039.42799534056324
#define GENESIS_DELTA_T_NANOSECONDS   18773
*/

#define GENESIS_PER_OP_FM_CLOCK       53267
#define GENESIS_1HZ_PHASE_INCREMENT   5039.432594289147127f
#define GENESIS_DELTA_T_NANOSECONDS   18773
#define GENESIS_DELTA_T_SECONDS       0.000018773f

#endif
