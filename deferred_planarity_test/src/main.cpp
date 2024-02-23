//-----------------------------------------------------------------------------------
// A simple code that test the MPS algorighm.
//-----------------------------------------------------------------------------------

#include "mps.h"
#include <iostream>
#include <cstdlib>
#include <climits>
#include <string>
#include <algorithm>
#include <iterator>
#include <random>
#include <vector>
 
#include <ogdf/fileformats/GraphIO.h>
#define START_TEMP 100

using namespace std;

int compute_removed_edge_size(string input_file, vector<int> post_order);

// these functions are defined in mps_test.cpp
// but their signatures are not in mps.h, hence they are declared here
vector<int> generate_post_order(string input_file);
vector<int> generate_mutated_post_order(string input_file, vector<int> post_order, int mutate_point);
vector<int> generate_guided_post_order(string input_file, vector<int> post_order);

void vector_printer(vector<int> state) {
    for (int i = 0; i < state.size(); ++i) {
        std::cout << state[i] << ",";
    }
    std::cout << std::endl;
}


vector<int> repeated_mutation(string input_file, int k_max, int mutate_point) {
    // generate first post order
    std::cout << "generate first post order" << std::endl;
    vector<int> state_old = generate_post_order(input_file);
    vector<int> state_new;

    for (int k = 0; k < k_max; ++k) {
        std::cout << "cycle:" << k << std::endl;

        vector_printer(state_old);
        
        
        // mutation produces rotated view
        state_new = generate_mutated_post_order(input_file, state_old, mutate_point);
        vector_printer(state_new);

        // another round of guided post order gives canonical representation
        state_new = generate_guided_post_order(input_file, state_new);
        vector_printer(state_new);


        std::cout << std::endl;
    }
    return state_new;
}

void test_correctness(string input_file) {
    vector<int> state_old = generate_post_order(input_file);
    int num_removed_edges;
    num_removed_edges = compute_removed_edge_size(input_file, state_old);
}


int get_graph_size(string input_file) {
    ogdf::Graph G;

    // utilize OGDF readGML
    if (!ogdf::GraphIO::read(G, input_file, ogdf::GraphIO::readGML)) {
        std::cerr << "Could not read " << input_file << ".gml" << std::endl;
    }

    return G.numberOfNodes();
}

//-----------------------------------------------------------------------------------
// Main function.
//-----------------------------------------------------------------------------------


int main(int argc, char* argv[]) {
    string input_file = argv[1];
    int k_max = std::stoi(argv[2]);
    int mutate_point = std::stoi(argv[3]);

    // generate order here
    vector<int> post_order = repeated_mutation(input_file, k_max, mutate_point);
    // test_correctness(input_file);

    // // print final order and number of edges
    // // print post_order
    // std::copy(post_order.begin(), post_order.end(), std::ostream_iterator<int>(std::cout, ","));
    // std::cout << std::endl;
    // int removed_edges = compute_removed_edge_size(input_file, post_order);
    // std::cout << "Number of removed edges: " << removed_edges << std::endl;

	return 0;
}
