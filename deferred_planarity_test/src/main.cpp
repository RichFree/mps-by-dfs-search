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

vector<int> generate_post_order(string input_file);
vector<int> generate_mutated_post_order(string input_file, vector<int> post_order);


vector<int> repeated_mutation(string input_file, int k_max) {
    vector<int> state_old = generate_post_order(input_file);
    vector<int> state_new;
    int num_removed_edges;
    for (int k = 0; k < k_max; ++k) {
        state_new = generate_mutated_post_order(input_file, state_old);
        num_removed_edges = compute_removed_edge_size(input_file, state_new);
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

    // generate order here
    // vector<int> post_order = repeated_mutation(input_file, k_max);
    test_correctness(input_file);

    // // print final order and number of edges
    // // print post_order
    // std::copy(post_order.begin(), post_order.end(), std::ostream_iterator<int>(std::cout, ","));
    // std::cout << std::endl;
    // int removed_edges = compute_removed_edge_size(input_file, post_order);
    // std::cout << "Number of removed edges: " << removed_edges << std::endl;

	return 0;
}
