#!/bin/bash

g++ ogdf_mps_bm.cpp -lOGDF -lCOIN -o bin/ogdf_mps_bm -O3 -march=native -Wall

echo "built ogdf_mps_bm"

g++ ogdf_mps_cactus.cpp -lOGDF -lCOIN -o bin/ogdf_mps_cactus -O3 -march=native -Wall

echo "built ogdf_mps_cactus"

g++ ogdf_mps_fast.cpp -lOGDF -lCOIN -o bin/ogdf_mps_fast -O3 -march=native -Wall

echo "built ogdf_mps_fast"

g++ ogdf_mps_exact.cpp -lOGDF -lCOIN -o bin/ogdf_mps_exact -O3 -march=native -Wall

echo "built ogdf_mps_exact"

g++ get_graph_characteristics.cpp -lOGDF -lCOIN -o bin/get_graph_characteristics -O3 -march=native -Wall

echo "built get_graph_characteristics"
