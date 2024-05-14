//-----------------------------------------------------------------------------------
// Implementation of a MPS algorithm via PC-tree.
//-----------------------------------------------------------------------------------

#include "mps.h"
#include <cassert>
#include <unordered_map>
#include <unordered_set>

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

int compute_removed_edge_size(const ogdf::Graph &G, vector<int> post_order) {
    maximal_planar_subgraph_finder m;
    return m.compute_removed_edge_size(G, post_order);
}

vector<int> generate_guided_post_order_iterative(const ogdf::Graph &G, const vector<int> &post_order) {
    maximal_planar_subgraph_finder m;
    return m.generate_guided_post_order_iterative(G, post_order);
}

vector<int> generate_mutated_post_order_iterative(const ogdf::Graph &G, const vector<int> &post_order, int mutate_point) {
    maximal_planar_subgraph_finder m;
    return m.generate_mutated_post_order_iterative(G, post_order, mutate_point);
}


vector<int> generate_post_order_iterative(const ogdf::Graph &G) {
    maximal_planar_subgraph_finder m;
    return m.generate_post_order_iterative(G);
}


void compute_mps(const ogdf::Graph &G, int mutate_point, vector<int> &post_order, int &return_edge_size) {
    maximal_planar_subgraph_finder m;
    m.compute_mps(G, mutate_point, post_order, return_edge_size);
}


//-----------------------------------------------------------------------------------
// Function implementation for maximal_planar_subgraph_finder object 
//-----------------------------------------------------------------------------------


vector<int> maximal_planar_subgraph_finder::generate_post_order_iterative(const ogdf::Graph &G) {
    init_from_graph(G);
    post_order_traversal_iterative();
    return return_post_order();
}

vector<int> maximal_planar_subgraph_finder::generate_guided_post_order_iterative(const ogdf::Graph &G, const vector<int> &post_order) {
    init_from_graph(G);
    guided_post_order_traversal_iterative(post_order);
    return return_post_order();
}

vector<int> maximal_planar_subgraph_finder::generate_mutated_post_order_iterative(const ogdf::Graph &G, const vector<int> &post_order, int mutate_point) {
    init_from_graph(G);
    mutated_post_order_traversal_iterative(post_order, mutate_point);
    return return_post_order();
}


// given a post_order, compute the removed edge size
int maximal_planar_subgraph_finder::compute_removed_edge_size(const ogdf::Graph &G, vector<int> post_order) {
    // read_from_gml
    init_from_graph(G);
    guided_post_order_traversal_iterative(post_order);
    // std::cout << "-----" << std::endl;
    // std::cout << "compute removedd edged size" << std::endl;
    // std::cout << "-----" << std::endl;
    // print_post_order();
    sort_adj_list();
    determine_edges();
    // print_edge_list();
    back_edge_traversal();
     
    return output_removed_edge_size();
}

// clear the internal _post_order_list
void maximal_planar_subgraph_finder::reset_state() {
    _post_order_list.clear();
    _edge_list.clear();
    _back_edge_list.clear();
    _is_back_edge_eliminate.clear();
    // we also need to reset the mark of every node
    for (auto node:_node_list) {
        node->un_mark();
    }
}

// void maximal_planar_subgraph_finder::compute_mps(const ogdf::Graph &G, int mutate_point, vector<int> &post_order, int &return_edge_size) {
//     // part 1:
//     // we first generate a new mutated order, and then compute the removed edge size for that
//     init_from_graph(G);
//     mutated_post_order_traversal_iterative(post_order, mutate_point);
//     // compute MPS on the immediate tree produced by mutated traversal
//     sort_adj_list();
//     determine_edges();
//     back_edge_traversal();
//     return_edge_size = output_removed_edge_size();
// 
// 
//     vector<int> temp_order = return_post_order();
//     reset_state();
//     // part 2: compute mps with correct orientation tree
//     guided_post_order_traversal_iterative(temp_order);
//     post_order = return_post_order();
// }

// previous implementation did not rotate result
void maximal_planar_subgraph_finder::compute_mps(const ogdf::Graph &G, int mutate_point, vector<int> &post_order, int &return_edge_size) {
    // part 1:
    // we first generate a new mutated order, and then compute the removed edge size for that
    init_from_graph(G);
    mutated_post_order_traversal_iterative(post_order, mutate_point);
    // print_post_order();
    sort_adj_list();
    determine_edges();
    // print_edge_list();
    back_edge_traversal();
    return_edge_size = output_removed_edge_size();
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
        _node_list[source->index()]->add_unmarked();
        _node_list[target->index()]->add_adj(_node_list[source->index()]);
        _node_list[target->index()]->add_unmarked();
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

// -----
// DFS for main to separate disconnected graph into separate components
// -----

void construct_connected_components(ogdf::Graph &G, ogdf::List<ogdf::Graph> &components) {
    maximal_planar_subgraph_finder m;
    m.construct_connected_components(G, components);
}

void 
maximal_planar_subgraph_finder::construct_connected_components(ogdf::Graph &G, 
ogdf::List<ogdf::Graph> &components) {
    init_from_graph(G);
    for (size_t i = 0; i < _node_list.size(); ++i) {
        if (!_node_list[i]->is_marked()) {
            ogdf::Graph component;
            // this is a vector of nodes in a connected component
            vector<node*> return_node_list;
            dfs_cc(_node_list[i], return_node_list);
            // now we construct a Graph using the return_node_list
            unordered_map<int, ogdf::node> index_to_node;
            for (auto current_node : return_node_list) {
                ogdf::node v = component.newNode(current_node->node_id());
                index_to_node[current_node->node_id()] = v;
            }
            // we keep an unordered set of edges to only add edges if its never been added
            unordered_set<pair<int,int>, PairHash> added_edges;
            for (auto current_node : return_node_list) {
                vector<node*> neighbor_list = current_node->_adj_list;
                for (auto it = neighbor_list.begin(); it != neighbor_list.end(); ++it) {
                    node* neighbor_node = (*it);
                    int first_node = current_node->node_id();
                    int second_node = neighbor_node->node_id();
                    // only add edge if never seen before
                    if (added_edges.find(make_pair(first_node, second_node)) == added_edges.end()) {
                        component.newEdge(index_to_node[first_node], 
                                        index_to_node[second_node]);
                        added_edges.insert(make_pair(first_node, second_node));
                        added_edges.insert(make_pair(second_node, first_node));
                    }
                }
            }
            components.pushBack(component);
        }
    }
}