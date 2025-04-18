//-----------------------------------------------------------------------------------
// Header for modules: mps.cpp, mps_test.cpp, node.cpp.
//-----------------------------------------------------------------------------------

#ifndef _MPS_H
#define _MPS_H

#include <iostream>
#include <fstream>
#include <vector>
#include <utility>
#include <climits>
#include <cmath>
#include <limits>
#include <random>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <ogdf/fileformats/GraphIO.h>

using namespace std;

class node;
class maximal_planar_subgraph_finder;

enum label {
	NOT_VISITED = 0,
	ARTIFICIAL_EDGE = 1,
	BOUNDARY_PATH = 2,
	DELETED = 3
};

enum node_type {
	P_NODE = 0,
	C_NODE = 1,
	REPLICA_NODE = 2,
	AE_VIRTUAL_ROOT = 3
};

enum mark_type {
    UNMARKED = 0,
    MARKED = 1,
    RESERVED = 2,
};

class node
{
public:
	//CONSTRUCTOR
	node(node_type t);

	//DESTRUCTOR
	~node() {}

	//TYPE, ID, INDEX
    node_type type();
	int post_order_index();
	void set_id(int i);
	void set_post_order_index(int i);
	void recursively_labeling();
	int node_id();

	//DFS-TREE
	void add_adj(node* n);
	int degree();
	node* adj(int i);
	void set_adj_list(vector<node*> vec);
	void DFS_visit(vector<node*> &dfsList, int &index);
    void guided_DFS_visit(vector<node *> &dfsList,
                          const vector<node *> &node_list,
                          int &return_index,
                          const unordered_map<int, int> &node_id_to_pos);
    void mutated_DFS_visit(vector<node *> &dfsList,
                           const vector<node *> &node_list,
                           int &index,
                           int &traversal_index,
                           const unordered_map<int, int> &node_id_to_pos,
                           int mutate_point,
                           mt19937 rng);

    // custom comparator function to sort nodes according to order in given vector
    bool sortByOrder(const unordered_map<int, int>& node_id_to_pos, node* a, node* b); 
    bool sortByFreeNeighbors(node* a, node* b);

	//PARENT-CHILDREN
	void set_parent(node* n) ;
	node* parent();
	int child_num();
	node* child(int i);
	void add_child(node* n);
	void clear_children();
	void remove_child(int i);
	void remove_child(node* n);
	vector<node*>* get_children_list();

	//BOUNDARY_PATH
	void set_to_boundary_path(node* n0, node* n1);
	void set_neighbor(int i, node* n);
	void set_neighbor(node* u, node* v);
	node* neighbor(int i);
	node* get_next(node* prev);

	//ARTIFICIAL EDGE
	node* AE(int i);
	void set_AE(int i, node* j);
	void add_AE(node* j);
	void inherit_AE(node* u);
	void init_AE(node* u);

	//REPLICA
	node* original_node();
	node* get_c_node();
	void set_c_node(node* c);
	bool is_sentinel();
	static bool is_same(node* n1, node* n2);
	void init_replica(node* u, node* c);

	//LABELING
	void set_1st_label(int i);
	void set_2nd_label(label i);
	int get_1st_label();
	label get_2nd_label();

	//C-NODE
	node* get_a_list_node();
	int c_node_size();
	node* essential(int i);
	void clear_essential();
	void add_essential(node* u);

	//MARK
	void mark();
	static void init_mark();
	void un_mark();
	bool is_marked();

	//Mark
	mark_type _mark;

    // keep track of unmarked neighbors
    void add_unmarked();
    int get_unmarked_count();



    // made public for access by iterative
	vector<node*> _adj_list;

private:
	//Basic information.
	node_type _type;
	pair<int, label> _label;

	//Information about neighborhood.
	node* _neighbor[2];
	node* _AE_root[2];

	//Information about higher hierarchy.
	node* _original_node; 
    node* _c_node;

	//Information about parent-children relation.
	node* _parent;
	vector<node*> _children;

	//Information about about p-nodes in DFS-tree
	// vector<node*> _adj_list;
	int _post_order_index;
	int _node_id;

	//List of essential nodes in c-node
	vector<node*> _essential_list;

    // store number of unmarked neighbors
    int _unmarked_neighbors_count;

};

class maximal_planar_subgraph_finder
{
public:
	maximal_planar_subgraph_finder();
	~maximal_planar_subgraph_finder();

    // functions that prepare state
    void init_from_graph(const ogdf::Graph &G);
	vector<int> generate_post_order_iterative(const ogdf::Graph &G);
	vector<int> generate_guided_post_order_iterative(const ogdf::Graph &G, const vector<int> &post_order);
	vector<int> generate_mutated_post_order_iterative(const ogdf::Graph &G, const vector<int> &post_order, int mutate_point);


    bool sort_by_order(const unordered_map<int, int>& node_id_to_pos, node* a, node* b); 
    bool sort_by_free_neighbors(node* a, node* b);

    void construct_connected_components(ogdf::Graph &G, ogdf::List<ogdf::Graph> &components);
    struct PairHash {
        size_t operator()(const pair<int, int>& p) const {
            return hash<int>()(p.first) ^ hash<int>()(p.second);
        }
    };


	void post_order_traversal_iterative();
	void guided_post_order_traversal_iterative(const vector<int> &post_order);
	void mutated_post_order_traversal_iterative(const vector<int> &post_order, int mutate_point);
    void dfs_cc(node* root_node, vector<node*> &return_node_list);
    void dfs(node* root_node, int &post_order_id);
    void dfs_guided(node* root_node, int &post_order_id, const unordered_map<int, int> &node_id_to_pos);
    void dfs_mutated(node* root_node, int &post_order_id, 
                    const unordered_map<int, int> &node_id_to_pos, 
                    int mutate_point, int &traversal_index, mt19937 rng);

    // compute_mps combines functionality to reduce repeating object initialization
    // the results are returned by modifying mutable reference
    void compute_mps(const ogdf::Graph &G, int mutate_point, vector<int> &post_order, int &return_edge_size);

	int find_mps(const ogdf::Graph &G);
	int compute_removed_edge_size(const ogdf::Graph &G, vector<int> post_order);
	node* get_new_node(node_type t);
    void reset_state();
	int output_removed_edge_size();
    vector<int> return_post_order();
    void print_post_order();
    void print_edge_list();
	void sort_adj_list();
	void determine_edges();
	void back_edge_traversal();
	bool back_edge_traversal(node* traverse_node, int index);
	void make_essential(node* p_node, node* c_node);
	node* find(node* n);
	void merge(pair<pair<node*, node*>, pair<node*, node*> > boundary, node* list_node);
	void eliminate(node* u);
	void eliminate_AE(node* u, node* v);
	pair<pair<node*, node*>, pair<node*, node*> > trim(node* u);
	void c_node_extension(node* c_node);
	void recursively_shaving(node* u);
	pair<node*, node*> shave(node* x);
	pair<node*, node*> parallel_search_sentinel(node* x, node* &c);
	pair<node*, node*> parallel_search_sentinel(node* n0, node* n0_prev, node* n1, node* n1_prev, node* & c);
	pair<node*, node*> count_sentinel_elimination(pair<node*, node*> sentinel_1, int num_sentinel);
	node* construct(node* u);
	node* construct(node* c, node* p);
	void parenting_labeling_shaving(node* u, node* node_i) ;

private:
	vector<node*> _node_list; //List of nodes input.
	vector<pair<node*, node*> > _edge_list; // Edges in DFS-tree. These edges must be contained in the maximal planar subgraph that we found.
	vector<node*> _post_order_list; //The sorted version (increasing with post-order-index) of _node_list.
	vector<pair<node*, node*> > _back_edge_list; // Edges other than that in DFS-tree. (The first node's index is higher than the second's.)
	vector<bool> _is_back_edge_eliminate; //Record that if the back-edge has been eliminated or not.
	vector<node*> _new_node_list; //Newly added nodes.
};

#endif // for MPS_H
