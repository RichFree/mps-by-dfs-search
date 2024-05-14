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
    vector<int> old_order = generate_post_order_iterative(G);
    // vector_printer(old_order);
    vector<int> temp_order = old_order;
    int new_removed_size;
    int old_removed_size = INT_MAX;

    // prepare random selection
    std::random_device rd;
    std::mt19937 gen{rd()}; // seed the generator

    const int final_value = old_order.size() - 1;
    const int iter_size = k_max;
    const int end_plat_iter = static_cast<int>(0.1 * iter_size); // End of the plateau
    double growth_factor = std::log(final_value) / (iter_size - end_plat_iter - 1);

    int mutate_index = 0;
    double updated_value = 0;

    for (int r = 0; r < reruns; ++r) {

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

            mutate_index = 0;

            compute_mps(G, mutate_index, temp_order, new_removed_size);
            vector<int> check_order = generate_guided_post_order_iterative(G, temp_order);
            int check1 = compute_removed_edge_size(G, check_order);


            assert(new_removed_size == check1);


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
            // if (k % 100 == 0) {
            //     std::cout << k << "," << old_removed_size << std::endl;;
            // }
        }
    }
    std::cout << "final: " << old_removed_size << std::endl;;
    return old_order;
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
        std::cout << "component: " << loop_count++ << std::endl;
        std::cout << component.numberOfNodes() << std::endl;
        std::cout << component.numberOfEdges() << std::endl;
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
