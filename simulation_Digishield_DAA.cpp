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
// --- DIGISHIELD PARAMETER ---
#define DIGISHIELD_WINDOW 17                // Lookback window size (N)

typedef struct {  
    double t;
    double target;
    int height; 
} BlockTemplate;

int main() {

    FILE *fp;
    fp = fopen("data/DigiShield_Output.csv", "w");

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

        // --- DIGISHIELD DAA LOGIC ---
if (n > DIGISHIELD_WINDOW) {
            // 1. Compute historical window timespans with clear semantic names
            double ideal_timespan = DIGISHIELD_WINDOW * IDEAL_INTERBLOCK_TIME;
            // double actual_timespan = blocks[n].t - blocks[n - DIGISHIELD_WINDOW].t;
            double actual_timespan = blocks[n - 1].t - blocks[n - 1 - DIGISHIELD_WINDOW].t;
            // 2. Apply the DigiShield Tempering Factor to get the adjusted goal timespan
            // double goal_timespan = ideal_timespan + (actual_timespan - ideal_timespan) / 4.0;
            double goal_timespan = ideal_timespan + (actual_timespan - ideal_timespan) / (4.0 * 600 / 75); // adjusted tempering factor to account for the shorter ideal inter-block time
            // 3. Apply Asymmetrical Saturation Clamps
            // Max adjustment up: 16% | Max adjustment down: 32%
            double min_goal_timespan = ideal_timespan * 100.0 / 116.0;
            double max_goal_timespan = ideal_timespan * 100.0 / 68.0;
            if (goal_timespan < min_goal_timespan) goal_timespan = min_goal_timespan;
            if (goal_timespan > max_goal_timespan) goal_timespan = max_goal_timespan;            
            // 4. Adjust the target (Note: Target is inversely proportional to difficulty)
            current_target = current_target * (goal_timespan / ideal_timespan);
        } else {
            current_target = INITIAL_ABS_TARGET;
        }
        // -----------------------------
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