#include <stdio.h>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <vector>
#include <queue>

// ----------------- SIMULATION PARAMETERS -----------------
// #define NOISE
// ----------------- SIMULATION PARAMETERS -----------------

#define NETWORK_RESOLUTION 1.0

#define DAY (86400.0)
#define FORTNIGHT (14.0*DAY)
#define YEAR (365.25*DAY)

#define DAILY (1.0/DAY)
#define FORTNIGHTLY (1.0/FORTNIGHT)
#define YEARLY (1.0/YEAR)

#define INITIAL_ABS_TARGET (1.0/4294967296.0)
#define IDEAL_INTERBLOCK_TIME (600.0)
#define GENESIS_TIME (2009.0*YEAR)
#define FINISH_TIME (2025.0*YEAR)

#define MAX_NUMBER_OF_LAYERS 4
#define NUM_OF_LAST_BLOCKS 6

#define PRINT_EVERY_THIS_MANY_BLOCKS 100    // parameter to control the number of printed blocks

typedef struct {  
    double t;
    double target;
    // double height[MAX_NUMBER_OF_LAYERS]; // last layer will be an integer!
    int height; // last layer will be an integer!
} BlockTemplate;

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


int main() {

    FILE *fp;
    fp = fopen("data/ETH_Output.csv", "w");

    srand48(time(NULL));
    
    BlockTemplate *blocks = (BlockTemplate *) calloc(1000000, sizeof(BlockTemplate));
    int n=0;
    double t = GENESIS_TIME;
    double current_target = INITIAL_ABS_TARGET;
    double current_difficulty = 1.0 / INITIAL_ABS_TARGET;
    BlockTemplate candidate = {t, INITIAL_ABS_TARGET, 0 };

    blocks[n] = candidate; // genesis block
	 
    fprintf(fp, "BLOCK,timestamp,t,block_time,real_hash_rate,difficulty,skew\n");	     
    fprintf(fp, "%d,%.0f,%.10lf,%.10lf,%.0f,%.10lf,%0.10lf\n", n, blocks[n].t, blocks[n].t/YEAR, -1.0, unobservable_hash_rate_function(t), INITIAL_ABS_TARGET/blocks[n].target, (blocks[n].t-(GENESIS_TIME+n*IDEAL_INTERBLOCK_TIME))/FORTNIGHT);

    while ( (n + 1) < 1000000 && t < FINISH_TIME) {
        do {
            t += NETWORK_RESOLUTION;
            candidate.t = t;
            candidate.target = current_target;
            candidate.height = blocks[n].height + 1; 
        } while (!(drand48() < candidate.target*unobservable_hash_rate_function(t)*NETWORK_RESOLUTION ));   

        n++;
        blocks[n] = candidate;


        // --- ETHEREUM CLASSIC DAA LOGIC ---
        #define ADJUSTMENT_DIVISOR 45.0    // Scaled down from 2048 to maintain agility
        double block_time = blocks[n].t - blocks[n-1].t;
        // Scale the step window to your 600-second target using the existing macro
        double time_factor = 1.0 - floor(block_time / IDEAL_INTERBLOCK_TIME);
        double adjustment_multiplier = std::max(time_factor, -99.0);
        // Use the scaled divisor here to ensure rapid adaptation
        double difficulty_step = (current_difficulty / ADJUSTMENT_DIVISOR) * adjustment_multiplier;
        current_difficulty = current_difficulty + difficulty_step;
        // if (current_difficulty < 1.0) { 
        //     current_difficulty = 1.0; 
        // }
        current_target = 1.0 / current_difficulty;

        fprintf(fp, "%d,%.0f,%.10lf,%.10lf,%.0f,%.10lf,%0.10lf\n", n, blocks[n].t, blocks[n].t/YEAR, blocks[n].t-blocks[n-1].t, unobservable_hash_rate_function(t), INITIAL_ABS_TARGET/blocks[n].target, (blocks[n].t-(GENESIS_TIME+n*IDEAL_INTERBLOCK_TIME))/FORTNIGHT);
    }

    free(blocks);
      
    return 0;
}

