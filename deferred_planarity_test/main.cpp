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
    int num_runs = stoi(argv[2]);
    // find the size of the graph here
    int node_size = get_graph_size(input_file);
    // generate order here
    vector<int> post_order;
    for (int i=0; i < node_size; ++i) {
        post_order.push_back(i);
    }

    for (int i=0; i< num_runs; ++i) {
        std::random_device rd;
        std::mt19937 g(rd());
    
        std::shuffle(post_order.begin(), post_order.end(), g);
        // print order
        std::copy(post_order.begin(), post_order.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;
        std::cout << "Number of removed edges: " << compute_removed_edge_size(input_file, post_order) << std::endl;
    }

	return 0;
}
