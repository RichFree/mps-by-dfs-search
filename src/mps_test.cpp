//-----------------------------------------------------------------------------------
// Implementation of a MPS algorithm via PC-tree.
//-----------------------------------------------------------------------------------

#include "mps.h"

#include <ogdf/fileformats/GraphIO.h>

// #define DEBUG
// #define DEBUG_2
// #define TIME

//-----------------------------------------------------------------------------------
// Finding MPS
//-----------------------------------------------------------------------------------

// programs to call from main:

// read input file of gml format
ogdf::Graph read_from_gml(string input_file) {
    ogdf::Graph G;

    // utilize OGDF readGML
    if (!ogdf::GraphIO::read(G, input_file, ogdf::GraphIO::readGML)) {
        std::cerr << "Could not read " << input_file << ".gml" << std::endl;
    }
    return G;
}

int find_mps(const ogdf::Graph &G) {
	maximal_planar_subgraph_finder m;
	return m.find_mps(G);
}

int compute_removed_edge_size(const ogdf::Graph &G, vector<int> post_order) {
    maximal_planar_subgraph_finder m;
    return m.compute_removed_edge_size(G, post_order);
}

vector<int> generate_post_order(const ogdf::Graph &G) {
    maximal_planar_subgraph_finder m;
    return m.generate_post_order(G);
}


void compute_mps(const ogdf::Graph &G, int mutate_point, vector<int> &post_order, int &return_edge_size) {
    maximal_planar_subgraph_finder m;
    m.compute_mps(G, mutate_point, post_order, return_edge_size);
}


//-----------------------------------------------------------------------------------
// Function implementation for maximal_planar_subgraph_finder object 
//-----------------------------------------------------------------------------------


// original maximal planar subgraph implementation without additional features
int maximal_planar_subgraph_finder::find_mps(const ogdf::Graph &G) {
    init_from_graph(G);
	post_order_traversal();
	sort_adj_list();
	determine_edges();
	back_edge_traversal();
	return output_removed_edge_size();
}

vector<int> maximal_planar_subgraph_finder::generate_post_order(const ogdf::Graph &G) {
    init_from_graph(G);
    post_order_traversal();
    return return_post_order();
}

// given a post_order, compute the removed edge size
int maximal_planar_subgraph_finder::compute_removed_edge_size(const ogdf::Graph &G, vector<int> post_order) {
    // read_from_gml
	init_from_graph(G);
    guided_post_order_traversal(post_order);
	sort_adj_list();
 	determine_edges();
 	back_edge_traversal();
     
	return output_removed_edge_size();
}

// clear the internal _post_order_list
void maximal_planar_subgraph_finder::reset_state() {
    _post_order_list.clear();
}

void maximal_planar_subgraph_finder::compute_mps(const ogdf::Graph &G, int mutate_point, vector<int> &post_order, int &return_edge_size) {
    // part 1:
    // we first generate a new mutated order, and then compute the removed edge size for that
    init_from_graph(G);
    mutated_post_order_traversal(post_order, mutate_point);
	sort_adj_list();
	determine_edges();
    back_edge_traversal();
    return_edge_size = output_removed_edge_size();

    // part 2:
    // now we get the canonical representation of the post order
    // we run guided_post_order_traversal on the temp_post_order to rotate it back to the canonical representation
    vector<int> temp_post_order = return_post_order(); 
    reset_state(); // clear the _post_order_list
    // perform guided Post Order Traversal to flip the tree
    guided_post_order_traversal(temp_post_order);
    post_order = return_post_order();
}

//-----------------------------------------------------------------------------------
// Input, output
//-----------------------------------------------------------------------------------

void maximal_planar_subgraph_finder::init_from_graph(const ogdf::Graph &G) {
    // create nodes
    _node_list.reserve(G.numberOfNodes());
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
    for (size_t i = 0; i < _back_edge_list.size(); ++i) {
		if (_is_back_edge_eliminate[i]) ++sum; 
    }
    return sum;
}