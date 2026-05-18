#!/bin/bash

NUM_RUNS=1

echo "=========================================================="
echo " Starting Automated Multi-Run Simulation Framework "
echo "=========================================================="

# Ensure both the data folder and the multirun subfolder exist
mkdir -p data/multirun

for i in $(seq 1 $NUM_RUNS)
do
    echo "Processing Simulation Run #$i of $NUM_RUNS..."
    ./simulation_NefdaN 1 && mv data/Output_with_layers_1.csv data/multirun/layers_1_run_$i.csv
    ./simulation_NefdaN 2 && mv data/Output_with_layers_2.csv data/multirun/layers_2_run_$i.csv
    ./simulation_NefdaN 3 && mv data/Output_with_layers_3.csv data/multirun/layers_3_run_$i.csv
    ./simulation_NefdaN 4 && mv data/Output_with_layers_4.csv data/multirun/layers_4_run_$i.csv    
    ./simulation_BTC_DAA        && mv data/BTC_Output.csv data/multirun/btc_run_$i.csv
    ./simulation_LWMA_DAA       && mv data/LWMA_Output.csv data/multirun/lwma_run_$i.csv
    ./simulation_Digishield_DAA && mv data/DigiShield_Output.csv data/multirun/digishield_run_$i.csv
done

echo "=========================================================="
echo " Success! All datasets are in data/multirun/ "
echo "=========================================================="