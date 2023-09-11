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

    // generate order here
    vector<int> post_order = generate_post_order(input_file);
    std::copy(post_order.begin(), post_order.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << std::endl;

    // generate mutated order
    vector<int> mutated_order = generate_mutated_post_order(input_file, post_order);
    post_order = mutated_order;
    std::copy(post_order.begin(), post_order.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << std::endl;


    // print order
    int removed_edges = compute_removed_edge_size(input_file, post_order);
    std::cout << "Number of removed edges: " << removed_edges << std::endl;


	return 0;
}
