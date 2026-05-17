#include <stdio.h>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <vector>
#include <queue>
#include <algorithm> // Required for std::max and std::min

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

// --- LWMA PARAMETERS ---
#define LWMA_WINDOW 60                      // Standard lookback window size (N)

typedef struct {  
    double t;
    double target;
    int height; 
} BlockTemplate;

double unobservable_hash_rate_function(double t) {
    double initial_hash_rate = 7000000;
    double NoiselessHashRate;

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
    fp = fopen("data/LWMA_Output.csv", "w"); // Updated file destination path

    srand48(time(NULL));
    
    BlockTemplate *blocks = (BlockTemplate *) calloc(1000000, sizeof(BlockTemplate));
    int n=0;
    double t = GENESIS_TIME;
    double current_target = INITIAL_ABS_TARGET;
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

        // --- LWMA DAA LOGIC ---
        if (n > LWMA_WINDOW) {
            double weighted_actual_timespan = 0.0;
            double target_sum = 0.0;
            // 1. Compute the linearly weighted actual timespan and sum of targets
            for (int k = 1; k <= LWMA_WINDOW; k++) {
                int index = (n - 1) - LWMA_WINDOW + k;
                double block_time = blocks[index].t - blocks[index - 1].t;
                // Linear weight 'k' scales from 1 up to LWMA_WINDOW
                weighted_actual_timespan += block_time * k;
                target_sum += blocks[index].target;
            }
            // 2. Compute the ideal weighted timespan base using arithmetic series sum
            // (LWMA_WINDOW * (LWMA_WINDOW + 1) / 2.0) is the sum of arithmetic series from 1 to LWMA_WINDOW
            double weighted_ideal_timespan = IDEAL_INTERBLOCK_TIME * (LWMA_WINDOW * (LWMA_WINDOW + 1) / 2.0);
            // 3. Compute the arithmetic mean of targets (which maps to harmonic difficulty mean)
            double mean_target = target_sum / LWMA_WINDOW;
            // 4. Update the target using the ratio of actual over ideal timespans
            current_target = mean_target * (weighted_actual_timespan / weighted_ideal_timespan);
            // Safety limit checks to avoid extreme single-block macro movements
            // if (current_target > INITIAL_ABS_TARGET) {
            //     current_target = INITIAL_ABS_TARGET;
            // }
        } else {
            // Keep the baseline target stable during the initial priming window
            current_target = INITIAL_ABS_TARGET;
        }
        // ----------------------
        // if (n % PRINT_EVERY_THIS_MANY_BLOCKS == 0) {
        //     fprintf(fp, "%d,%.0f,%.10lf,%.10lf,%.0f,%.10lf,%0.10lf\n", n, blocks[n].t, blocks[n].t/YEAR, blocks[n].t-blocks[n-1].t, unobservable_hash_rate_function(t), INITIAL_ABS_TARGET/blocks[n].target, (blocks[n].t-(GENESIS_TIME+n*IDEAL_INTERBLOCK_TIME))/FORTNIGHT);
        // }
        fprintf(fp, "%d,%.0f,%.10lf,%.10lf,%.0f,%.10lf,%0.10lf\n", n, blocks[n].t, blocks[n].t/YEAR, blocks[n].t-blocks[n-1].t, unobservable_hash_rate_function(t), INITIAL_ABS_TARGET/blocks[n].target, (blocks[n].t-(GENESIS_TIME+n*IDEAL_INTERBLOCK_TIME))/FORTNIGHT);
    }

    free(blocks);
      
    return 0;
}