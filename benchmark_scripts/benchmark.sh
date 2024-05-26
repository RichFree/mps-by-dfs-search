#!/bin/bash

DATA_DIR=$1

THREADS=4

CYCLE_COUNT=(10 100 1000)

RERUNS=1

TYPE_SET=("north" "rome" "steinlib" "scale_free" "random_regular")

# mkdir output directory in case not present
mkdir -p output

# guided mutation with ranked-node traversal

for NUM_CYCLES in "${CYCLE_COUNT[@]}"; do
    echo "Now running for $NUM_CYCLES iterations"
    for TYPE in "${TYPE_SET[@]}"; do
        start_time=$(date +%s.%N)
        echo "running $TYPE: dfs"
        mkdir -p output/$TYPE
        cat input/$TYPE.txt | 
            sed "s|^|$DATA_DIR/|" |
            parallel --jobs $THREADS "./bin/dpt_planarizer {} $NUM_CYCLES $RERUNS" > output/$TYPE/dfs_${NUM_CYCLES}.csv
        end_time=$(date +%s.%N)
        elapsed_time=$(echo "$end_time - $start_time" | bc)
        echo "Elapsed time: $elapsed_time seconds"
        echo ""
    done
done

# boyer myrvold
for TYPE in "${TYPE_SET[@]}"; do
    echo "running $TYPE: bm"
    start_time=$(date +%s.%N)
    cat input/$TYPE.txt | 
        sed "s|^|$DATA_DIR/|" |
        parallel --jobs $THREADS "./bin/ogdf_mps_bm {}" > output/$TYPE/bm.csv
    end_time=$(date +%s.%N)
    elapsed_time=$(echo "$end_time - $start_time" | bc)
    echo "Elapsed time: $elapsed_time seconds"
    echo ""
done


# fast
echo "running $TYPE: fast"
for TYPE in "${TYPE_SET[@]}"; do
    start_time=$(date +%s.%N)
    cat input/$TYPE.txt | 
        sed "s|^|$DATA_DIR/|" |
        parallel --jobs $THREADS "./bin/ogdf_mps_fast {}" > output/$TYPE/fast.csv
    end_time=$(date +%s.%N)
    elapsed_time=$(echo "$end_time - $start_time" | bc)
    echo "Elapsed time: $elapsed_time seconds"
    echo ""
done


# cactus
echo "running $TYPE: cactus"
for TYPE in "${TYPE_SET[@]}"; do
    start_time=$(date +%s.%N)
    cat input/$TYPE.txt | 
        sed "s|^|$DATA_DIR/|" |
        parallel --jobs $THREADS "./bin/ogdf_mps_cactus {}" > output/$TYPE/cactus.csv
    end_time=$(date +%s.%N)
    elapsed_time=$(echo "$end_time - $start_time" | bc)
    echo "Elapsed time: $elapsed_time seconds"
    echo ""
done


# get graph properties
echo "getting properties..."
for TYPE in "${TYPE_SET[@]}"; do
    cat input/$TYPE.txt | 
        sed "s|^|$DATA_DIR/|" |
        parallel --jobs $THREADS "./bin/get_graph_characteristics {}" > output/$TYPE/graph_properties.csv
done

echo "benchmark finished"
