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
// SELECT ONE OF THE FOLLOWING HASH RATE FUNCTIONS (NON_ADAPTIVE OR ADAPTIVE) TO RUN THE SIMULATION
#define NON_ADAPTIVE
// #define ADAPTIVE
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

typedef struct {  
  double t;
  double target;
  double height[MAX_NUMBER_OF_LAYERS]; // last layer will be an integer!
} BlockTemplate;

#ifdef NON_ADAPTIVE
double non_adaptive_hash_rate_function(double t) {
  // Will and Iain's hash functions
  return (t < 2010.5*YEAR) ? 10000000 : 10000000*30*exp((t-2010.5*YEAR)/(10*FORTNIGHT) + pow((t-2010.5*YEAR)/(15*FORTNIGHT), 2));
  // return (t < 2010.5*YEAR) ? 10000000 : 10000000*30*exp((t-2010.5*YEAR)/(10*FORTNIGHT));
  // return (t < 2010.5*YEAR) ? 10000000 : 10000000*30;

  // constant hash function
  // return INITIAL_HASH_RATE;

  // step hash function
  // return (t < 2010.5*YEAR) ? 10000000 : 10000000*30;

  // step hash function
  // return (t < 2010.5*YEAR) ? 10000000 : 10000000*30;

  // negative step hash function
  // return (t < 2010.5*YEAR) ? 10000000 : 10000000/30;

  // cos hash function
  // return (10000000 - 5000000 * sin(2*M_PI*(t-GENESIS_TIME)/(4 * YEAR)));

  // rectangular hash function
  // return (t < 2010.5*YEAR) ? 10000000 : (t < 2016.5*YEAR) ? 10000000*30 : 10000000;

  // staircase hash function
  // return (t < 2010.5*YEAR) ? 10000000 : (t < 2013.5*YEAR) ? 10000000*10 : 10000000 * 100;

  // linearly increasing hash function
  // return INITIAL_HASH_RATE + (INITIAL_HASH_RATE/YEAR) * (t-GENESIS_TIME);

  // quadratically increasing hash function
  // return INITIAL_HASH_RATE + INITIAL_HASH_RATE/pow(YEAR,2) * pow((t-GENESIS_TIME),2);

  // exponentially increasing hash function
  // return INITIAL_HASH_RATE * exp(log(2) * (t-GENESIS_TIME)/YEAR);

  // superexponentially increasing hash function
  // return INITIAL_HASH_RATE * exp(log(2) * pow(((t-GENESIS_TIME)/YEAR),2));
  
  // sin and linearly increasing hash function
  // return INITIAL_HASH_RATE + INITIAL_HASH_RATE/YEAR * (t-GENESIS_TIME) + INITIAL_HASH_RATE * sin(2*M_PI*(t-GENESIS_TIME)/(YEAR));
}
#endif


#ifdef ADAPTIVE
double adaptive_hash_rate_function(double cur_avg_difficulty, double prev_avg_difficulty, double t) {
  // Will and Iain's hash functions
  double BASE_HASH_RATE =  (t < 2010.5*YEAR) ? 10000000 : 10000000*30*exp((t-2010.5*YEAR)/(10*FORTNIGHT) + pow((t-2010.5*YEAR)/(15*FORTNIGHT), 2));
  // constant hash function
  // double BASE_HASH_RATE = INITIAL_HASH_RATE;
  // step hash function
  // double BASE_HASH_RATE = (t < 2010.5*YEAR) ? 10000000 : 10000000*30;
  // staircase hash function
  // double BASE_HASH_RATE = (t < 2010.5*YEAR) ? 10000000 : (t < 2013.5*YEAR) ? 10000000*10 : 10000000 * 100;
  // exponentially increasing hash function
  // double BASE_HASH_RATE = INITIAL_HASH_RATE * exp(log(2) * (t-GENESIS_TIME)/YEAR);
  // sin and linearly increasing hash function
  // double BASE_HASH_RATE = INITIAL_HASH_RATE + INITIAL_HASH_RATE/YEAR * (t-GENESIS_TIME) + INITIAL_HASH_RATE * sin(2*M_PI*(t-GENESIS_TIME)/(YEAR));

  double hb = BASE_HASH_RATE / 3;
  double hv = 4 * hb;
  double hg = 4 * hb;
  double h = hb;
  double epsilon = 0.05;
  // double epsilon = 0.03;
  double difficulty_change = cur_avg_difficulty / prev_avg_difficulty;

  if(difficulty_change <= 1 - epsilon){
    h += hg + hv;
  }
  else if(difficulty_change <= 1 - epsilon/3){
    h += hg + hv / (1 + exp((-1)*(1-difficulty_change)/(epsilon*epsilon)));
  }
  else if(difficulty_change <= 1 + epsilon){
    h += hv / (1 + exp((-1)*(1-difficulty_change)/(epsilon*epsilon)));
  }
  return h;
}
#endif



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
      break;
    case 3:
      layer_frequency = std::vector<double>{0.5 * FORTNIGHTLY, 0.3 * FORTNIGHTLY, 0.2 * FORTNIGHTLY}; // hyper st_peters (3 layers)
      break;
    case 4:
      layer_frequency = std::vector<double>{0.4 * FORTNIGHTLY, 0.3 * FORTNIGHTLY, 0.2 * FORTNIGHTLY, 0.1 * FORTNIGHTLY}; // hyperhyper st_peters (4 layers)
      break;
    default:
      return 1;
  }

  srand48(time(NULL));

    #ifdef ADAPTIVE
    std::queue < double > q_cur;
    std::queue < double > q_prev;
    double cur_running_sum = NUM_OF_LAST_BLOCKS;
    double prev_running_sum = NUM_OF_LAST_BLOCKS;
    double cur_running_avg = 1;
    double prev_running_avg = 1;
    for (int i = 0; i < NUM_OF_LAST_BLOCKS; i++){
        q_cur.push(1.0);
        q_prev.push(1.0);
    }
    #endif
  
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


  fprintf(fp, "BLOCK,timestamp,t,block_time,real_hash_rate,");
  for (int i=0; i<NUMBER_OF_LAYERS; i++) {
    fprintf(fp, "layer_%d_virtual_difficulty,", i+1);
  } 
  fprintf(fp, "difficulty,skew\n");	 
  #ifdef NON_ADAPTIVE 
  fprintf(fp, "%d,%.0f,%.10lf,%.10lf,%.0f", n, blocks[n].t, blocks[n].t/YEAR, -1.0, non_adaptive_hash_rate_function(t));
  for (int l=0; l<NUMBER_OF_LAYERS; l++) {
    fprintf(fp, ",%.10lf", 1.0/blocks_virtual_relative_targets[n][l]);
  }
  fprintf(fp, ",%.10lf,%0.10lf\n", INITIAL_ABS_TARGET/blocks[n].target, (blocks[n].t-(GENESIS_TIME+n*IDEAL_INTERBLOCK_TIME))/FORTNIGHT);
  // fprintf(fp, "%d,%.0f,%.10lf,%.10lf,%.0f,%.10lf,%0.10lf\n", n, blocks[n].t, blocks[n].t/YEAR, -1.0, non_adaptive_hash_rate_function(t), INITIAL_ABS_TARGET/blocks[n].target, (blocks[n].t-(GENESIS_TIME+n*IDEAL_INTERBLOCK_TIME))/FORTNIGHT);	   
  #endif
  #ifdef ADAPTIVE
  fprintf(fp, "%d,%.0f,%.10lf,%.10lf,%.0f", n, blocks[n].t, blocks[n].t/YEAR, -1.0, adaptive_hash_rate_function(cur_running_avg, prev_running_avg, t));
  for (int l=0; l<NUMBER_OF_LAYERS; l++) {
    fprintf(fp, ",%.10lf", 1.0/blocks_virtual_relative_targets[n][l]);
  }
  fprintf(fp, ",%.10lf,%0.10lf\n", INITIAL_ABS_TARGET/blocks[n].target, (blocks[n].t-(GENESIS_TIME+n*IDEAL_INTERBLOCK_TIME))/FORTNIGHT);
  // fprintf(fp, "%d,%.0f,%.10lf,%.10lf,%.0f,%.10lf,%0.10lf\n", n, blocks[n].t, blocks[n].t/YEAR, -1.0, adaptive_hash_rate_function(cur_running_avg, prev_running_avg, t), INITIAL_ABS_TARGET/blocks[n].target, (blocks[n].t-(GENESIS_TIME+n*IDEAL_INTERBLOCK_TIME))/FORTNIGHT);	   
  #endif

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

      for (int l=0; l<NUMBER_OF_LAYERS; l++) 
	      candidate.height[l] = blocks[n].height[l] + target[l]/candidate.target;

    delete[] target;  
    #ifdef NON_ADAPTIVE
    } while (!(drand48() < candidate.target*non_adaptive_hash_rate_function(t)*NETWORK_RESOLUTION ));   
    #endif
    #ifdef ADAPTIVE
    } while (!(drand48() < candidate.target*adaptive_hash_rate_function(cur_running_avg, prev_running_avg, t)*NETWORK_RESOLUTION ));      // if the candidate block is valid, break (the mul with adaptive_hash_rate_function(t) simulates the effect of volatile network hash rate)
    #endif

    n++;
    blocks[n] = candidate;
    for(int i=0; i<NUMBER_OF_LAYERS; i++){
      blocks_virtual_relative_targets[n][i] = virtual_relative_target[i];
    }

    #ifdef NON_ADAPTIVE
    fprintf(fp, "%d,%.0f,%.10lf,%.10lf,%.0f", n, blocks[n].t, blocks[n].t/YEAR, blocks[n].t-blocks[n-1].t, non_adaptive_hash_rate_function(t));
    for (int l=0; l<NUMBER_OF_LAYERS; l++) {
      fprintf(fp, ",%.10lf", 1.0/blocks_virtual_relative_targets[n][l]);
    }
    fprintf(fp, ",%.10lf,%0.10lf\n",INITIAL_ABS_TARGET/blocks[n].target, (blocks[n].t-(GENESIS_TIME+n*IDEAL_INTERBLOCK_TIME))/FORTNIGHT);	 
    #endif
    #ifdef ADAPTIVE
    fprintf(fp, "%d,%.0f,%.10lf,%.10lf,%.0f", n, blocks[n].t, blocks[n].t/YEAR, blocks[n].t-blocks[n-1].t, adaptive_hash_rate_function(cur_running_avg, prev_running_avg, t));
    for (int l=0; l<NUMBER_OF_LAYERS; l++) {
      fprintf(fp, ",%.10lf", 1.0/blocks_virtual_relative_targets[n][l]);
    }
    fprintf(fp, ",%.10lf,%0.10lf\n", INITIAL_ABS_TARGET/blocks[n].target, (blocks[n].t-(GENESIS_TIME+n*IDEAL_INTERBLOCK_TIME))/FORTNIGHT);
    #endif      

    #ifdef ADAPTIVE
    q_cur.push(INITIAL_ABS_TARGET/(blocks[n].target));
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

