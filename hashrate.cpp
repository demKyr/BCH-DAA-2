#include <math.h>
#include "hashrate.h"

// ----------------- SIMULATION PARAMETERS -----------------
// #define NOISE
// ----------------- SIMULATION PARAMETERS -----------------


double unobservable_hash_rate_function(double t) {
    // double initial_hash_rate = 10000000;
    double initial_hash_rate = 7000000;
    double NoiselessHashRate;

    // linearly increasing hash function
    // NoiselessHashRate = initial_hash_rate + (initial_hash_rate/YEAR) * (t-GENESIS_TIME);

    // quadratically increasing hash function
    // NoiselessHashRate = initial_hash_rate + (initial_hash_rate/pow(YEAR,2)) * pow((t-GENESIS_TIME),2);

    // Exponential hash function
    // NoiselessHashRate = initial_hash_rate*exp((t-GENESIS_TIME)/(10*FORTNIGHT));

    // Exponential hash function with jump in 2010.5
    // NoiselessHashRate = (t < 2010.5*YEAR) ? initial_hash_rate : initial_hash_rate*30*exp((t-2010.5*YEAR)/(10*FORTNIGHT));

    // Exponential-of-quadratic hash function with jump in 2010.5
    // NoiselessHashRate = (t < 2010.5*YEAR) ? initial_hash_rate : initial_hash_rate*30*exp((t-2010.5*YEAR)/(10*FORTNIGHT) + pow((t-2010.5*YEAR)/(15*FORTNIGHT), 2));

    // Step hash function
    NoiselessHashRate = (t < 2010.5*YEAR) ? initial_hash_rate : initial_hash_rate*30;

    
    #ifdef NOISE
    return NoiselessHashRate * (1 + 0.51*sin((t-GENESIS_TIME)*4*FORTNIGHTLY)); // with noise
    #else
    return NoiselessHashRate; // without noise
    #endif  
}