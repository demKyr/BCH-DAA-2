#include <stdio.h>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <vector>
#include <queue>

#include "hashrate.h"

#define PRINT_EVERY_THIS_MANY_BLOCKS 100    // parameter to control the number of printed blocks


typedef struct {  
    double t;
    double target;
    int height; // last layer will be an integer!
} BlockTemplate;


int main() {

    FILE *fp;
    fp = fopen("data/BTC_Output.csv", "w");

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
        if (n % 2016 == 0) {
            current_target = current_target * std::max( std::min( ((blocks[n].t - blocks[n-2016].t) / (2016.0*IDEAL_INTERBLOCK_TIME)), 4.0 ), 0.25 );
        }

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

