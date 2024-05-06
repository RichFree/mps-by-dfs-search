//-----------------------------------------------------------------------------------
// Implementation of a MPS algorithm via PC-tree.
//-----------------------------------------------------------------------------------

#include <chrono>
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

vector<int> generate_mutated_post_order(const ogdf::Graph &G, vector<int> post_order, int mutate_point) {
    maximal_planar_subgraph_finder m;
    return m.generate_mutated_post_order(G, post_order, mutate_point);
}

vector<int> generate_guided_post_order(const ogdf::Graph &G, vector<int> post_order) {
    maximal_planar_subgraph_finder m;
    return m.generate_guided_post_order(G, post_order);
}


void compute_mps(const ogdf::Graph &G, int mutate_point, vector<int> &post_order, int &return_edge_size) {
    maximal_planar_subgraph_finder m;
    m.compute_mps(G, mutate_point, post_order, return_edge_size);
}


// ---------

int maximal_planar_subgraph_finder::find_mps(const ogdf::Graph &G) {
    init_from_graph(G);
	postOrderTraversal();

    #ifdef DEBUG
    print_post_order();
    #endif

	sort_adj_list();
	determine_edges();
	back_edge_traversal();
	return output_removed_edge_size();
}

vector<int> maximal_planar_subgraph_finder::generate_post_order(const ogdf::Graph &G) {
    init_from_graph(G);
    postOrderTraversal();

    #ifdef DEBUG
    std::cout << "standard post order traversal" << std::endl;
    print_post_order();
    #endif

    return return_post_order();
}

// result of this will be used as input to "compute_removed_edge_size"
vector<int> maximal_planar_subgraph_finder::generate_mutated_post_order(const ogdf::Graph &G, const vector<int> &post_order, int mutate_point) {
	init_from_graph(G);

    mutatedPostOrderTraversal(post_order, mutate_point);

    #ifdef DEBUG
    std::cout << "mutated post order traversal" << std::endl;
    print_post_order();
    #endif

    return return_post_order();
}

// result of this will be used as input to "compute_removed_edge_size"
vector<int> maximal_planar_subgraph_finder::generate_guided_post_order(const ogdf::Graph &G, const vector<int> &post_order) {
	init_from_graph(G);
    guidedPostOrderTraversal(post_order);

    // #ifdef DEBUG
    // std::cout << "guided post order traversal" << std::endl;
    // print_post_order();
    // #endif

    return return_post_order();
}



int maximal_planar_subgraph_finder::compute_removed_edge_size(const ogdf::Graph &G, vector<int> post_order) {
    // read_from_gml
	init_from_graph(G);

    // guidedPostOrderTraversal
    guidedPostOrderTraversal(post_order);
	sort_adj_list();
 	determine_edges();
 	back_edge_traversal();
     
	return output_removed_edge_size();
}

void maximal_planar_subgraph_finder::reset_state() {
    _post_order_list.clear();
}

void maximal_planar_subgraph_finder::compute_mps(const ogdf::Graph &G, int mutate_point, vector<int> &post_order, int &return_edge_size) {
    init_from_graph(G);
    mutatedPostOrderTraversal(post_order, mutate_point);
	sort_adj_list();
	determine_edges();
    back_edge_traversal();
    return_edge_size = output_removed_edge_size();

    // now we get the canonical representation of the post order
    vector<int> temp_post_order = return_post_order(); 

    reset_state(); // clear the _post_order_list
    // perform guided Post Order Traversal to flip the tree
    guidedPostOrderTraversal(temp_post_order);

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