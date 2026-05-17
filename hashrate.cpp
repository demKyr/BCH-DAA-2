#include <math.h>

// ----------------- SIMULATION PARAMETERS -----------------
// SELECT ONE OR NONE OF THE FOLLOWING HASH RATE FUNCTIONS
// #define NOISE
// #define ADAPTIVE
// ----------------- SIMULATION PARAMETERS -----------------


double unobservable_hash_rate_function(double t, double cur_avg_difficulty, double prev_avg_difficulty) {
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
    NoiselessHashRate = (t < 2010.5*YEAR) ? initial_hash_rate : initial_hash_rate*30*exp((t-2010.5*YEAR)/(10*FORTNIGHT) + pow((t-2010.5*YEAR)/(15*FORTNIGHT), 2));

    // Step hash function
    // NoiselessHashRate = (t < 2010.5*YEAR) ? initial_hash_rate : initial_hash_rate*30;

    #ifdef ADAPTIVE
        double hb = NoiselessHashRate / 3;
        double hv = 4 * hb;
        double hg = 4 * hb;
        double h = hb;
        double epsilon = 0.05;
        // double epsilon = 0.03;
        double difficulty_change = cur_avg_difficulty / prev_avg_difficulty;
        if(difficulty_change <= 1 - epsilon) h += hg + hv;
        else if(difficulty_change <= 1 - epsilon/3) h += hg + hv / (1 + exp((-1)*(1-difficulty_change)/(epsilon*epsilon)));
        else if(difficulty_change <= 1 + epsilon) h += hv / (1 + exp((-1)*(1-difficulty_change)/(epsilon*epsilon)));
        return h;
    #elifdef NOISE
        return NoiselessHashRate * (1 + 0.51*sin((t-GENESIS_TIME)*4*FORTNIGHTLY)); // with noise
    #else
        return NoiselessHashRate; // without noise or adaptive hash rate function
    #endif  
}