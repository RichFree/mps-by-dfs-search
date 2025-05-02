#!/bin/bash

# guided mutation for 1000 iterations
echo "guided mutation"
# dpt_planarizer <input.gml> <no. iterations> <reruns>
./guided_mutation/bin/dpt_planarizer make_graphs/graphs/graphn100e4s42i0.gml 1000 1

echo "cactus"
./heuristics/bin/ogdf_mps_cactus make_graphs/graphs/graphn100e4s42i0.gml

echo "fast"
./heuristics/bin/ogdf_mps_fast make_graphs/graphs/graphn100e4s42i0.gml

echo "bm"
./heuristics/bin/ogdf_mps_bm make_graphs/graphs/graphn100e4s42i0.gml