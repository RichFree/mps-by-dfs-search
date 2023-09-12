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

using namespace std;

int compute_removed_edge_size(string input_file, vector<int> post_order);

vector<int> generate_post_order(string input_file);
vector<int> generate_mutated_post_order(string input_file, vector<int> post_order);

double temp_decay(int k, int k_max) {
    return 1.0 - ((k + 1.0) / (k_max));
}

vector<int> sa_solve(string input_file, int k_max) {

    // create sampling function
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<> distribution(0.0, 1.0);

    // generate first state
    vector<int> state_old = generate_post_order(input_file);
    vector<int> state_new;
    int e_old = compute_removed_edge_size(input_file, state_old);
    int e_new = 0;
    int delta = 0;
    // initialize terms
    double temp = 100;

    for (int k = 0; k < k_max; ++k) {
        temp = temp_decay(k, k_max);

        state_new = generate_mutated_post_order(input_file, state_old);
        e_new = compute_removed_edge_size(input_file, state_new);
        delta = e_new - e_old;

        if (std::exp( -(delta) / temp) > distribution(rng)) {
           state_old = state_new; 
           e_old = e_new;
        }
        // std::cout << "temp: " << temp << ", score: " << e_old << std::endl;
    }

    return state_old;

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
    vector<int> post_order = sa_solve(input_file, k_max);

    // std::copy(post_order.begin(), post_order.end(), std::ostream_iterator<int>(std::cout, " "));
    // std::cout << std::endl;

    // print order
    int removed_edges = compute_removed_edge_size(input_file, post_order);
    std::cout << "Number of removed edges: " << removed_edges << std::endl;

	return 0;
}
