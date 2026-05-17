#ifndef HASHRATE_H
#define HASHRATE_H

#define NETWORK_RESOLUTION 1.0

#define DAY (86400.0)
#define FORTNIGHT (14.0*DAY)
#define YEAR (365.25*DAY)

#define DAILY (1.0/DAY)
#define FORTNIGHTLY (1.0/FORTNIGHT)
#define YEARLY (1.0/YEAR)

#define INITIAL_ABS_TARGET (1.0/4294967296.0)
#define INITIAL_ABS_DIFFICULTY (4294967296.0) // 2^32
#define IDEAL_INTERBLOCK_TIME (600.0)
#define GENESIS_TIME (2009.0*YEAR)
#define FINISH_TIME (2025.0*YEAR)

#define NUM_OF_LAST_BLOCKS 6

double unobservable_hash_rate_function(double t, double cur_avg_difficulty = 1.0, double prev_avg_difficulty = 1.0);

#include "hashrate.cpp"

#endif