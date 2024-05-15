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
#include <sys/resource.h>
#include <filesystem>
#include <chrono>
#include <cassert>
 
#include <ogdf/fileformats/GraphIO.h>

using namespace std;

// these functions are defined in mps_test.cpp
// but their signatures are not in mps.h, hence they are declared here
ogdf::Graph read_from_gml(string input_file);
void construct_connected_components(ogdf::Graph &G, ogdf::List<ogdf::Graph> &components);
vector<int> generate_post_order_iterative(const ogdf::Graph &G);
vector<int> generate_guided_post_order_iterative(const ogdf::Graph &G, const vector<int> &post_order);
vector<int> generate_mutated_post_order_iterative(const ogdf::Graph &G, const vector<int> &post_order, int mutate_point);
void compute_mps(const ogdf::Graph &G, int mutate_point, vector<int> &post_order, int &return_edge_size);
int compute_removed_edge_size(const ogdf::Graph &G, vector<int> post_order);

void vector_printer(const vector<int>& state) {
    for (size_t i = 0; i < state.size(); ++i) {
        std::cout << state[i] << ",";
    }
    std::cout << std::endl;
}


vector<int> repeated_mutation(const ogdf::Graph &G, int k_max, int reruns) {
    // generate first post order
    vector<int> best_order = generate_post_order_iterative(G);
    vector<int> current_order = best_order;
    int new_removed_size;
    int best_removed_size = INT_MAX;

    // prepare random selection
    std::random_device rd;
    std::mt19937 gen{rd()}; // seed the generator

    // variables to manage exponential mutate_index iteration
    int index_range = best_order.size() - 1; // assumes start from 0
    int index_start = 0;
    // given k_max and number of range for mutate_index, we compute growth factor
    // so that at mutate_factor spans index_range in an exponential rate
    double growth_factor = std::log(index_range + 1) / (k_max);
    int mutate_index = 0;

    for (int r = 0; r < reruns; ++r) {

        for (int k = 0; k < k_max; ++k) {
            // update mutate_index based on exponential rate wrt k
            mutate_index = index_start + static_cast<int>(std::exp(growth_factor * k) - 1);

            // internally compute_mps already ran a round of guided traversal to rotate the result back
            // function will return via reference to current_order and new_removed_size
            compute_mps(G, mutate_index, current_order, new_removed_size);


            // if there is an improvement
            // 1. update the removed size to use the new smaller size
            // 2. update the old_order to be the new_order
            if (new_removed_size < best_removed_size) {
                best_removed_size = new_removed_size;
                best_order = current_order;
            // if there is no improvement, we revert the temp_order to the old_order
            } else {
                current_order = best_order;
            }
        }
    }
    return best_order;
}



//-----------------------------------------------------------------------------------
// Main function.
//-----------------------------------------------------------------------------------


int main(int argc, char* argv[]) {
    string input_file = argv[1];
    int k_max = std::stoi(argv[2]);
    int reruns = std::stoi(argv[3]);

    ogdf::Graph G = read_from_gml(input_file);


    auto start = std::chrono::high_resolution_clock::now();


    // lets create graphs of connected components
    ogdf::List<ogdf::Graph> components;
    construct_connected_components(G, components);
    vector<int> post_order;

    int removed_edges = 0;
    int loop_count = 0;
    for (auto component:components) {
        if (component.numberOfEdges() == 0) {
            continue;
        }
        // std::cout << "component: " << loop_count++ << std::endl;
        // std::cout << component.numberOfNodes() << std::endl;
        // std::cout << component.numberOfEdges() << std::endl;
        post_order = repeated_mutation(component, k_max, reruns);
        removed_edges += compute_removed_edge_size(component, post_order);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    double time = static_cast<double>(microseconds) / 1'000'000.0;

   
    string filename = filesystem::path(input_file).stem();
    std::cout << filename << ", " << removed_edges << ", " << time << std::endl;

    // post_order = generate_guided_post_order_iterative(G, post_order);
    // removed_edges = compute_removed_edge_size(G, post_order);
    // std::cout << filename << ", " << removed_edges << ", " << time << std::endl;

    // post_order = generate_guided_post_order_iterative(G, post_order);
    // removed_edges = compute_removed_edge_size(G, post_order);
    // std::cout << filename << ", " << removed_edges << ", " << time << std::endl;


    return 0;
}
