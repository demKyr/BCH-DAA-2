#include <stdio.h>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <vector>

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
#define FINISH_TIME (2026.0*YEAR)

#define MAX_NUMBER_OF_LAYERS 4

typedef struct {  
  double t;
  double target;
  double height[MAX_NUMBER_OF_LAYERS]; // last layer will be an integer!
} BlockTemplate;

double UNOBSERVABLE_hash_rate(double t) {
  // return (t < 2010.5*YEAR) ? 10000000 : 10000000*30*exp((t-2010.5*YEAR)/(10*FORTNIGHT) + pow((t-2010.5*YEAR)/(15*FORTNIGHT), 2));
  return (t < 2010.5*YEAR) ? 10000000 : 10000000*30*exp((t-2010.5*YEAR)/(10*FORTNIGHT));
  // return (t < 2010.5*YEAR) ? 10000000 : 10000000*30;
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
  
  BlockTemplate *blocks = (BlockTemplate *) calloc(1000000, sizeof(BlockTemplate));

  int n=0;
  
  double t = GENESIS_TIME;

  BlockTemplate candidate = {t, INITIAL_ABS_TARGET, {0} };

  blocks[n] = candidate; // genesis block

  fprintf(fp, "BLOCK,timestamp,t,block_time,real_hash_rate,difficulty,skew\n");	 
  fprintf(fp, "%d,%.0f,%.10lf,%.10lf,%.0f,%.10lf,%0.10lf\n", n, blocks[n].t, blocks[n].t/YEAR, -1.0, UNOBSERVABLE_hash_rate(t), INITIAL_ABS_TARGET/blocks[n].target, (blocks[n].t-(GENESIS_TIME+n*IDEAL_INTERBLOCK_TIME))/FORTNIGHT);	       

  while ( (n + 1) < 1000000 && t < FINISH_TIME) {

    do {
      t += NETWORK_RESOLUTION;
      candidate.t = t;

      double target[NUMBER_OF_LAYERS];
      
      for (int l=0; l<NUMBER_OF_LAYERS; l++) {
        double skew = t - (GENESIS_TIME+blocks[n].height[l]*IDEAL_INTERBLOCK_TIME);
        target[l] = ((l == 0) ? INITIAL_ABS_TARGET : target[l-1]) * exp(layer_frequency[l]*skew);
      }
      
      candidate.target = target[NUMBER_OF_LAYERS - 1];

      for (int l=0; l<NUMBER_OF_LAYERS; l++) 
	      candidate.height[l] = blocks[n].height[l] + target[l]/candidate.target;
      
    } while (!(drand48() < candidate.target*UNOBSERVABLE_hash_rate(t)*NETWORK_RESOLUTION ));   

    n++;
    blocks[n] = candidate;

    fprintf(fp, "%d,%.0f,%.10lf,%.10lf,%.0f,%.10lf,%0.10lf\n", n, blocks[n].t, blocks[n].t/YEAR, blocks[n].t-blocks[n-1].t, UNOBSERVABLE_hash_rate(t), INITIAL_ABS_TARGET/blocks[n].target, (blocks[n].t-(GENESIS_TIME+n*IDEAL_INTERBLOCK_TIME))/FORTNIGHT);	       
  }

  free(blocks);
  
  return 0;
}

