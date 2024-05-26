#!/bin/bash

g++ make_regular_graph.cpp -lOGDF -o bin/make_regular_graph -O3 -march=native -Wall

echo "built make_regular_graph"

g++ make_scale_free_graph.cpp -lOGDF -o bin/make_scale_free_graph -O3 -march=native -Wall

echo "built make_scale_free_graph"
