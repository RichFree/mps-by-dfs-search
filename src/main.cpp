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
// #define TIME

using namespace std;

// these functions are defined in mps_test.cpp
// but their signatures are not in mps.h, hence they are declared here
ogdf::Graph read_from_gml(string input_file);
vector<int> generate_post_order(const ogdf::Graph &G);
void compute_mps(const ogdf::Graph &G, int mutate_point, vector<int> &post_order, int &return_edge_size);
int compute_removed_edge_size(const ogdf::Graph &G, vector<int> post_order);

void vector_printer(const vector<int>& state) {
    for (size_t i = 0; i < state.size(); ++i) {
        std::cout << state[i] << ",";
    }
    std::cout << std::endl;
}


vector<int> repeated_mutation(const ogdf::Graph &G, int k_max) {
    // generate first post order
    vector<int> old_order = generate_post_order(G);
    vector_printer(old_order);
    vector<int> temp_order = old_order;
    int new_removed_size;
    int old_removed_size = INT_MAX;

    // prepare random selection
    std::random_device rd;
    std::mt19937 gen{rd()}; // seed the generator
    int first_value = 0;
    // we want the index of the third last value
    // at a given traversal index, only the next iteration has the mutated value
    int last_value = (old_order.size() - 1) - 2;

    const int final_value = old_order.size() - 1;
    const int iter_size = k_max;
    const int end_plat_iter = static_cast<int>(0.5 * iter_size); // End of the plateau
    double growth_factor = std::log(final_value) / (iter_size - end_plat_iter - 1);

    int mutate_index = 0;
    double updated_value = 0;

    for (int k = 0; k < k_max; ++k) {
        // initial plateau phase
        if (k < end_plat_iter) {
            mutate_index = 0;
        // exponential growth
        } else {
            mutate_index = static_cast<int>(updated_value);
            // update for next round
            updated_value = std::exp(growth_factor * (k - end_plat_iter));
        }

        // std::cout << "k: " << k << ", mutate_index: " << mutate_index << '\n';

        compute_mps(G, mutate_index, temp_order, new_removed_size);

        // if there is an improvement
        // 1. update the removed size to use the new smaller size
        // 2. update the old_order to be the new_order
        if (new_removed_size < old_removed_size) {
            old_removed_size = new_removed_size;
            old_order = temp_order;
        // if there is no improvement, we revert the temp_order to the old_order
        } else {
            temp_order = old_order;
        }
    }
    return old_order;
}


//-----------------------------------------------------------------------------------
// Main function.
//-----------------------------------------------------------------------------------


int main(int argc, char* argv[]) {
    string input_file = argv[1];
    int k_max = std::stoi(argv[2]);

    const ogdf::Graph G = read_from_gml(input_file);

    // generate order here
    vector<int> post_order = repeated_mutation(G, k_max);

    // // print final order and number of edges
    std::cout << "---" << std::endl;
    std::cout << "final report" << std::endl;
    // std::copy(post_order.begin(), post_order.end(), std::ostream_iterator<int>(std::cout, ","));
    // std::cout << std::endl;
    int removed_edges = compute_removed_edge_size(G, post_order);
    std::cout << "Number of removed edges: " << removed_edges << std::endl;

	return 0;
}
