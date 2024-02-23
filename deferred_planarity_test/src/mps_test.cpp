//-----------------------------------------------------------------------------------
// Implementation of a MPS algorithm via PC-tree.
//-----------------------------------------------------------------------------------

#include "mps.h"
#include <ogdf/fileformats/GraphIO.h>

// #define DEBUG

//-----------------------------------------------------------------------------------
// Finding MPS
//-----------------------------------------------------------------------------------

// programs to call from main:

int find_mps(string input_file) {
	maximal_planar_subgraph_finder m;
	return m.find_mps(input_file);
}

int compute_removed_edge_size(string input_file, vector<int> post_order) {
    maximal_planar_subgraph_finder m;
    return m.compute_removed_edge_size(input_file, post_order);
}

vector<int> generate_post_order(string input_file) {
    maximal_planar_subgraph_finder m;
    return m.generate_post_order(input_file);
}

vector<int> generate_mutated_post_order(string input_file, vector<int> post_order, int mutate_point) {
    maximal_planar_subgraph_finder m;
    return m.generate_mutated_post_order(input_file, post_order, mutate_point);
}

vector<int> generate_guided_post_order(string input_file, vector<int> post_order) {
    maximal_planar_subgraph_finder m;
    return m.generate_guided_post_order(input_file, post_order);
}


// ---------

int maximal_planar_subgraph_finder::find_mps(string input_file) {
	read_from_gml(input_file);
	postOrderTraversal();

    #ifdef DEBUG
    print_post_order();
    #endif

	sort_adj_list();
	determine_edges();
	back_edge_traversal();
	return output_removed_edge_size();
}

vector<int> maximal_planar_subgraph_finder::generate_post_order(string input_file) {
    read_from_gml(input_file);
    postOrderTraversal();

    #ifdef DEBUG
    std::cout << "standard post order traversal" << std::endl;
    print_post_order();
    #endif

    return return_post_order();
}

// result of this will be used as input to "compute_removed_edge_size"
vector<int> maximal_planar_subgraph_finder::generate_mutated_post_order(string input_file, vector<int> post_order, int mutate_point) {
    read_from_gml(input_file);
    mutatedPostOrderTraversal(post_order, mutate_point);

    #ifdef DEBUG
    std::cout << "mutated post order traversal" << std::endl;
    print_post_order();
    #endif

    return return_post_order();
}

// result of this will be used as input to "compute_removed_edge_size"
vector<int> maximal_planar_subgraph_finder::generate_guided_post_order(string input_file, vector<int> post_order) {
    read_from_gml(input_file);
    guidedPostOrderTraversal(post_order);

    // #ifdef DEBUG
    // std::cout << "guided post order traversal" << std::endl;
    // print_post_order();
    // #endif

    return return_post_order();
}



int maximal_planar_subgraph_finder::compute_removed_edge_size(string input_file, vector<int> post_order) {
	read_from_gml(input_file);
    guidedPostOrderTraversal(post_order);
    
    #ifdef DEBUG
    std::cout << "guided post order traversal" << std::endl;
    print_post_order();
    #endif

	sort_adj_list();
	determine_edges();
	back_edge_traversal();
	return output_removed_edge_size();
}

//-----------------------------------------------------------------------------------
// Imput, output
//-----------------------------------------------------------------------------------


// read input file of gml format
void maximal_planar_subgraph_finder::read_from_gml(string input_file) {
    ogdf::Graph G;

    // utilize OGDF readGML
    if (!ogdf::GraphIO::read(G, input_file, ogdf::GraphIO::readGML)) {
        std::cerr << "Could not read " << input_file << ".gml" << std::endl;
    }

    // create nodes
	for (int i = 0; i < G.numberOfNodes(); ++i) {
		_node_list.push_back(new node(P_NODE));
		_node_list[i]->set_id(i);
	}

    // create edges
    for (ogdf::edge e : G.edges) {
        ogdf::node source = e->source();
        ogdf::node target = e->target();
        _node_list[source->index()]->add_adj(_node_list[target->index()]);
        _node_list[target->index()]->add_adj(_node_list[source->index()]);
    }
}



// count the number of removed edges
int maximal_planar_subgraph_finder::output_removed_edge_size() {
    int sum = 0;
    for (int i = 0; i < _back_edge_list.size(); ++i) {
		if (_is_back_edge_eliminate[i]) ++sum; 
    }
    return sum;
}