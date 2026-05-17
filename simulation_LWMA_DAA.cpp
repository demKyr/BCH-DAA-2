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

#include "hashrate.h"

#define PRINT_EVERY_THIS_MANY_BLOCKS 100    // parameter to control the number of printed blocks

// --- LWMA PARAMETERS ---
#define LWMA_WINDOW 60                      // Standard lookback window size (N)

typedef struct {  
    double t;
    double target;
    int height; 
} BlockTemplate;

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

    #ifdef ADAPTIVE
    // queue for avg difficulty/avg target
    std::queue <double> q_cur;
    std::queue <double> q_prev;
    double cur_running_sum = NUM_OF_LAST_BLOCKS;
    double prev_running_sum = NUM_OF_LAST_BLOCKS;
    double cur_running_avg = 1;
    double prev_running_avg = 1;
    for (int i = 0; i < NUM_OF_LAST_BLOCKS; i++){
        q_cur.push(1.0);
        q_prev.push(1.0);
    }
    #endif
     
    fprintf(fp, "BLOCK,timestamp,t,block_time,real_hash_rate,difficulty,skew\n");  
    #ifndef ADAPTIVE       
    fprintf(fp, "%d,%.0f,%.10lf,%.10lf,%.0f,%.10lf,%0.10lf\n", n, blocks[n].t, blocks[n].t/YEAR, -1.0, unobservable_hash_rate_function(t), INITIAL_ABS_TARGET/blocks[n].target, (blocks[n].t-(GENESIS_TIME+n*IDEAL_INTERBLOCK_TIME))/FORTNIGHT);
    #endif
    #ifdef ADAPTIVE
    fprintf(fp, "%d,%.0f,%.10lf,%.10lf,%.0f,%.10lf,%0.10lf\n", n, blocks[n].t, blocks[n].t/YEAR, -1.0, unobservable_hash_rate_function(t, cur_running_avg, prev_running_avg), INITIAL_ABS_TARGET/blocks[n].target, (blocks[n].t-(GENESIS_TIME+n*IDEAL_INTERBLOCK_TIME))/FORTNIGHT);
    #endif

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
        #ifndef ADAPTIVE
        fprintf(fp, "%d,%.0f,%.10lf,%.10lf,%.0f,%.10lf,%0.10lf\n", n, blocks[n].t, blocks[n].t/YEAR, blocks[n].t-blocks[n-1].t, unobservable_hash_rate_function(t), INITIAL_ABS_TARGET/blocks[n].target, (blocks[n].t-(GENESIS_TIME+n*IDEAL_INTERBLOCK_TIME))/FORTNIGHT);
        #endif
        #ifdef ADAPTIVE
        fprintf(fp, "%d,%.0f,%.10lf,%.10lf,%.0f,%.10lf,%0.10lf\n", n, blocks[n].t, blocks[n].t/YEAR, blocks[n].t-blocks[n-1].t, unobservable_hash_rate_function(t, cur_running_avg, prev_running_avg), INITIAL_ABS_TARGET/blocks[n].target, (blocks[n].t-(GENESIS_TIME+n*IDEAL_INTERBLOCK_TIME))/FORTNIGHT);
        #endif

        #ifdef ADAPTIVE
        q_cur.push(1.0/(blocks[n].target)/INITIAL_ABS_DIFFICULTY);
        cur_running_sum = cur_running_sum + q_cur.back() - q_cur.front();
        cur_running_avg = cur_running_sum / NUM_OF_LAST_BLOCKS;
        q_prev.push(q_cur.front());
        prev_running_sum = prev_running_sum + q_prev.back() - q_prev.front();
        prev_running_avg = prev_running_sum / NUM_OF_LAST_BLOCKS;
        q_cur.pop();
        q_prev.pop();
        #endif
    }

    free(blocks);
      
    return 0;
}