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
    double height[MAX_NUMBER_OF_LAYERS]; // last layer will be an integer!
} BlockTemplate;

double unobservable_hash_rate_function(double t) {
    // double initial_hash_rate = 10000000;
    double initial_hash_rate = 7000000;
    double NoiselessHashRate;

    // Superexponential hash function
    // NoiselessHashRate = (t < 2010.5*YEAR) ? initial_hash_rate : initial_hash_rate*30*exp((t-2010.5*YEAR)/(10*FORTNIGHT) + pow((t-2010.5*YEAR)/(15*FORTNIGHT), 2));

    // Exponential hash function
    // NoiselessHashRate = (t < 2010.5*YEAR) ? initial_hash_rate : initial_hash_rate*30*exp((t-2010.5*YEAR)/(10*FORTNIGHT));

    // Step hash function
    NoiselessHashRate = (t < 2010.5*YEAR) ? initial_hash_rate : initial_hash_rate*30;
    
    #ifdef NOISE
    return NoiselessHashRate * (1 + 0.51*sin((t-GENESIS_TIME)*4*FORTNIGHTLY)); // with noise
    #else
    return NoiselessHashRate; // without noise
    #endif  
}


int main(int argc, char* argv[]) {

    (void)argc;
    const int NUMBER_OF_LAYERS = atoi(argv[1]);
    FILE *fp;
    fp = fopen(("data/Output_with_layers_" + std::to_string(NUMBER_OF_LAYERS) + ".csv").c_str(), "w");

    std::vector<double> layer_frequency;

    switch (NUMBER_OF_LAYERS) {
        case 1:
            layer_frequency = std::vector<double>{1.0 * FORTNIGHTLY}; // unmodified st_peters (1 layer)
            break;
        case 2:
            layer_frequency = std::vector<double>{0.5 * FORTNIGHTLY, 0.5 * FORTNIGHTLY}; // modified st_peters (2 layers)
            // layer_frequency = std::vector<double>{0.8 * FORTNIGHTLY, 0.2 * FORTNIGHTLY}; // modified st_peters (2 layers)
            break;
        case 3:
            layer_frequency = std::vector<double>{0.5 * FORTNIGHTLY, 0.3 * FORTNIGHTLY, 0.2 * FORTNIGHTLY}; // hyper st_peters (3 layers)
            // layer_frequency = std::vector<double>{0.3333333 * FORTNIGHTLY, 0.3333333 * FORTNIGHTLY, 0.3333333 * FORTNIGHTLY}; // hyper st_peters (3 layers)
            break;
        case 4:
            layer_frequency = std::vector<double>{0.4 * FORTNIGHTLY, 0.3 * FORTNIGHTLY, 0.2 * FORTNIGHTLY, 0.1 * FORTNIGHTLY}; // hyperhyper st_peters (4 layers)
            // layer_frequency = std::vector<double>{0.25 * FORTNIGHTLY, 0.25 * FORTNIGHTLY, 0.25 * FORTNIGHTLY, 0.25 * FORTNIGHTLY}; // hyperhyper st_peters (4 layers)
            break;
        default:
            return 1;
    }

    srand48(time(NULL));
    
    BlockTemplate *blocks = (BlockTemplate *) calloc(1000000, sizeof(BlockTemplate));

    double blocks_virtual_relative_targets[1000000][MAX_NUMBER_OF_LAYERS];

    int n=0;
    
    double t = GENESIS_TIME;

    BlockTemplate candidate = {t, INITIAL_ABS_TARGET, {0} };

    blocks[n] = candidate; // genesis block
    blocks_virtual_relative_targets[0][0] = 1.0;
    blocks_virtual_relative_targets[0][1] = 1.0;
    blocks_virtual_relative_targets[0][2] = 1.0;
    blocks_virtual_relative_targets[0][3] = 1.0;

    // print headers
    fprintf(fp, "BLOCK,timestamp,t,block_time,real_hash_rate,");
    for (int i=0; i<NUMBER_OF_LAYERS; i++) {
        fprintf(fp, "layer_%d_virtual_difficulty,", i+1);
    } 
    fprintf(fp, "difficulty,skew\n");	 
    
    // print genesis block
    fprintf(fp, "%d,%.0f,%.10lf,%.10lf,%.0f", n, blocks[n].t, blocks[n].t/YEAR, -1.0, unobservable_hash_rate_function(t));
    for (int l=0; l<NUMBER_OF_LAYERS; l++) {
        fprintf(fp, ",%.10lf", 1.0/blocks_virtual_relative_targets[n][l]);
    }
    fprintf(fp, ",%.10lf,%0.10lf\n", INITIAL_ABS_TARGET/blocks[n].target, (blocks[n].t-(GENESIS_TIME+n*IDEAL_INTERBLOCK_TIME))/FORTNIGHT);

    double* virtual_relative_target = new double[NUMBER_OF_LAYERS];
    while ( (n + 1) < 1000000 && t < FINISH_TIME) {
        do {
            t += NETWORK_RESOLUTION;
            candidate.t = t;

            double* target = new double[NUMBER_OF_LAYERS];
            
            for (int l=0; l<NUMBER_OF_LAYERS; l++) {
                double skew = t - (GENESIS_TIME+blocks[n].height[l]*IDEAL_INTERBLOCK_TIME);
                virtual_relative_target[l] = exp(layer_frequency[l]*skew);
                target[l] = ((l == 0) ? INITIAL_ABS_TARGET : target[l-1]) * virtual_relative_target[l];
            }
            
            candidate.target = target[NUMBER_OF_LAYERS - 1];
            // candidate.target is the target given to the miners which is the target of the last layer

            for (int l=0; l<NUMBER_OF_LAYERS; l++) 
                candidate.height[l] = blocks[n].height[l] + target[l]/candidate.target;
                // candidate.target is the target given to the miners
                // target[l]/candidate.target is the amount of fluid block (block height) mined in layer l
                
                
            delete[] target;  

        } while (!(drand48() < candidate.target*unobservable_hash_rate_function(t)*NETWORK_RESOLUTION ));   

        n++;
        blocks[n] = candidate;
        for(int i=0; i<NUMBER_OF_LAYERS; i++){
            blocks_virtual_relative_targets[n][i] = virtual_relative_target[i];
        }

        fprintf(fp, "%d,%.0f,%.10lf,%.10lf,%.0f", n, blocks[n].t, blocks[n].t/YEAR, blocks[n].t-blocks[n-1].t, unobservable_hash_rate_function(t));
        for (int l=0; l<NUMBER_OF_LAYERS; l++) {
            fprintf(fp, ",%.10lf", 1.0/blocks_virtual_relative_targets[n][l]);
        }
        fprintf(fp, ",%.10lf,%0.10lf\n",INITIAL_ABS_TARGET/blocks[n].target, (blocks[n].t-(GENESIS_TIME+n*IDEAL_INTERBLOCK_TIME))/FORTNIGHT);	 

    }

    free(blocks);
      
    return 0;
}

