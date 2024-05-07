//-----------------------------------------------------------------------------------
// Implementation of a MPS algorithm via PC-tree.
//-----------------------------------------------------------------------------------

#include "mps.h"

// #define DEBUG
// #define DEBUG_MUTATION

//-----------------------------------------------------------------------------------
// CONSTRUCTOR
//-----------------------------------------------------------------------------------
node::node(node_type t) {
	_type = t;
	_label = pair<int, label>(INT_MAX, NOT_VISITED);
	_neighbor[0] = _neighbor[1] = 0;
	_AE_root[0] = _AE_root[1]  = 0;
	_original_node = 0; 
    _c_node = 0;
	_parent = 0;
	_post_order_index = INT_MAX;
	_node_id = INT_MAX;
	_mark = UNMARKED;
}

//-----------------------------------------------------------------------------------
// TYPE, ID, INDEX
//-----------------------------------------------------------------------------------
node_type node::type() {return _type;}

int node::post_order_index() {return _post_order_index;}

void node::set_id(int i) {_node_id = i;}

void node::set_post_order_index(int i) {_post_order_index = i;}

//Only used when consturcting c-node
//The first node calling this function would not be labeled.
void node::recursively_labeling() {		
	for (size_t i = 0; i < _children.size(); ++i) {
		_children[i]->_label.second = ARTIFICIAL_EDGE;
		_children[i]->recursively_labeling();
	} 
}

int node::node_id() {return _node_id;}

//-----------------------------------------------------------------------------------
// DFS-TREE
//-----------------------------------------------------------------------------------
void node::add_adj(node* n) {_adj_list.push_back(n);}

int node::degree() {return _adj_list.size();}

node* node::adj(int i) {return _adj_list[i];}

void node::set_adj_list(vector<node*> vec) {_adj_list = vec;}

void node::DFS_visit(vector<node*> &dfsList, int &index) {
	mark();
	for (size_t i = 0; i < _adj_list.size(); ++i) {
		if (!_adj_list[i]->is_marked()) {
			_adj_list[i]->_parent = this;
			_adj_list[i]->DFS_visit(dfsList, index);
		}
	}
	set_post_order_index(index);
	dfsList.push_back(this);
	++index;
}


bool node::sortByOrder(const std::unordered_map<int, int>& node_id_to_pos, node* a, node* b) {
    auto iter_a = node_id_to_pos.find(a->node_id());
    auto iter_b = node_id_to_pos.find(b->node_id());

    // second yields the position
    return iter_a->second < iter_b->second;
}

bool node::sortByFreeNeighbors(node* a, node* b) {
    vector<node*> node_list_a = a->_adj_list;
    vector<node*> node_list_b = b->_adj_list;
    int count_a = 0;
    int count_b = 0;
    // count number of unmarked nodes in each node's neighbor list
    for (auto x:node_list_a) {
        if (!x->is_marked()) {
            count_a++;
        }
    }
    for (auto x:node_list_b) {
        if (!x->is_marked()) {
            count_b++;
        }
    }
    return count_a < count_b;
}




void node::guided_DFS_visit(vector<node *> &dfsList, 
                            const vector<node *> &node_list, 
                            int &return_index, 
                            const unordered_map<int, int> &node_id_to_pos)
{

	mark();

    // purpose of this block: create list of neighbors ordered in the order they appear in rev_post_order
    // we want to select neighbors that match the rev_post_order at the specific traversal_index

    // sort elements of _adj_list using sortByOrder
    vector<node*> neighbor_list = _adj_list; 
    std::sort(neighbor_list.begin(), neighbor_list.end(), [this, &node_id_to_pos](node* a, node* b) {
        return sortByOrder(node_id_to_pos, a, b);
    });

	
	for (size_t i = 0; i < neighbor_list.size(); ++i) {
		if (!neighbor_list[i]->is_marked()) {
			neighbor_list[i]->_parent = this;
			neighbor_list[i]->guided_DFS_visit(dfsList, node_list, return_index, node_id_to_pos);
		}
	}

    set_post_order_index(return_index);
	dfsList.push_back(this);
	++return_index;
}

void node::mutated_DFS_visit(vector<node *> &dfsList,
                             const vector<node *> &node_list,
                             int &return_index,
                             int &traversal_index,
                             const unordered_map<int, int> &node_id_to_pos,
                             int mutate_point,
                             mt19937 rng)
{

    // mark current node
	mark();
    
    // purpose of this block: create list of neighbors ordered in the order they appear in rev_post_order
    // we want to select neighbors that match the rev_post_order at the specific traversal_index

    vector<node*> neighbor_list = _adj_list; 

    // if the current index matches or exceeds the mutate_point, then we know this is the cycle to mutate
    if (traversal_index >= mutate_point) {
        // we sort by the number of unmarked nodes
        std::sort(neighbor_list.begin(), neighbor_list.end(), [this](node *a, node *b)
            { return sortByFreeNeighbors(a,b); });
        // I chose to randomly shuffle the first half to introduce back some randomness
        // I chose half, but it could very well be other values
        std::shuffle(neighbor_list.begin(), neighbor_list.begin() + (neighbor_list.size()/2), rng);

    // otherwise just sort based on the order set by node_id_to_pos, which is
    // set by the reversed post_order
    } else {
        std::sort(neighbor_list.begin(), neighbor_list.end(), [this, &node_id_to_pos](node *a, node *b)
                  { return sortByOrder(node_id_to_pos, a, b); });
    }

    
    // increment traversal index after checking
    // next node will receive incremented index
    traversal_index++;
    
    for (size_t i = 0; i < neighbor_list.size(); ++i)
    {
        if (!neighbor_list[i]->is_marked())
        {
            neighbor_list[i]->_parent = this;
            neighbor_list[i]->mutated_DFS_visit(dfsList, node_list, return_index, traversal_index, node_id_to_pos, mutate_point, rng);
        }
    }

	set_post_order_index(return_index);
	dfsList.push_back(this);
	++return_index;
}

//-----------------------------------------------------------------------------------
// PARENT-CHILDREN
//-----------------------------------------------------------------------------------
int node::child_num() {return _children.size();}

node* node::child(int i) {return _children[i];}

node* node::parent() {return _parent;}

void node::clear_children() {
	_children.clear();
}

void node::remove_child(int i) {
	_children[i] = _children[_children.size()-1];
	_children.resize(_children.size()-1);
}

void node::remove_child(node* n) {
	for (size_t i = 0; i < _children.size(); ++i) {
	    if (_children[i] == n) {
			_children[i] = _children[_children.size()-1];
	        _children.resize(_children.size()-1);
		}
	}
}

void node::add_child(node* n) {
	_children.push_back(n);
}

vector<node*>* node::get_children_list() {
	vector<node*>* ptr = new vector<node*>(_children);
	return ptr;
}

void node::set_parent(node* n) {
	_parent= n;
}

//-----------------------------------------------------------------------------------
// BOUNDARY_PATH
//-----------------------------------------------------------------------------------
void node::set_to_boundary_path(node* n0, node* n1) {
	_parent = 0;
	_children.clear();
	_neighbor[0] = n0;
	_neighbor[1] = n1;
	set_2nd_label(BOUNDARY_PATH);
}

node* node::get_next(node* prev) {
	if (_neighbor[0] != prev) return _neighbor[0];
	else return _neighbor[1];
}

node* node::neighbor(int i) {return _neighbor[i];}

void node::set_neighbor(int i, node* n) {_neighbor[i] = n;}

void node::set_neighbor(node* u, node* v) {
	_neighbor[0] = u;
	_neighbor[1] = v;
}

//-----------------------------------------------------------------------------------
// ARTIFICIAL EDGE
//-----------------------------------------------------------------------------------
node* node::AE(int i) {return _AE_root[i];}

void node::set_AE(int i, node* j) {
	_AE_root[i] = j;
	if (j != 0) j->set_parent(this);
} 

void node::add_AE(node* j) {
	if (j == 0) return;
	if (_AE_root[0] == 0) set_AE(0, j);
	else if (_AE_root[1] == 0) set_AE(1, j);
}

//Inherit u's artificial edge.
void node::inherit_AE(node* u) {
	if (u->_AE_root[0] != 0) add_AE(u->_AE_root[0]);
	if (u->_AE_root[1] != 0) add_AE(u->_AE_root[1]);
	u->_AE_root[0] = u->_AE_root[1] = 0;
}

//Set itself to be an AE-root-node in u.
//Inherite u's chilren-list.
//Do nothing if u does not have any children.
void node::init_AE(node* u) {
	if (u->child_num() == 0) return;
	_children = u->_children;
	u->clear_children();
	for (size_t i = 0; i < _children.size(); ++i) {
		_children[i]->set_parent(this);
	}
	set_parent(u);
	set_1st_label(_children[0]->get_1st_label());
	set_2nd_label(ARTIFICIAL_EDGE);
	u->add_AE(this);
}

//-----------------------------------------------------------------------------------
// REPLICA
//-----------------------------------------------------------------------------------
node* node::original_node() {return _original_node;}

node* node::get_c_node() {return _c_node;}

void node::set_c_node(node* c) {_c_node = c;}

bool node::is_sentinel() {return type() == REPLICA_NODE;} 

//Check if n1 and n2 correspond to the same node
bool node::is_same(node* n1, node* n2) {
	node* s1 = (n1->type() == REPLICA_NODE)? n1->original_node() : n1;
	node* s2 = (n2->type() == REPLICA_NODE)? n2->original_node() : n2;
	return s1 == s2;
}

//Set itself to be a replica-node of u in c.
//Only inherit some basic setting, not including info about neighborhood.
void node::init_replica(node* u, node* c) {
	set_post_order_index(u->post_order_index());
	set_2nd_label(BOUNDARY_PATH);
	_original_node = (u->type() == REPLICA_NODE)? u->original_node() : u;
	_c_node = c;
}

//-----------------------------------------------------------------------------------
// LABELING
//-----------------------------------------------------------------------------------
void node::set_1st_label(int i) {_label.first = i;}

void node::set_2nd_label(label i) {_label.second = i;}

int node::get_1st_label() {return _label.first;}

label node::get_2nd_label() {return _label.second;}

//-----------------------------------------------------------------------------------
// C-NODE
//-----------------------------------------------------------------------------------
node* node::get_a_list_node() {
	return _essential_list[0];
}

int node::c_node_size() {
	return _essential_list.size();
}

node* node::essential(int i) {
	return _essential_list[i];
}

void node::clear_essential() {_essential_list.clear();}

void node::add_essential(node* u) {_essential_list.push_back(u);}

//-----------------------------------------------------------------------------------
// MARK
//-----------------------------------------------------------------------------------
void node::mark() {_mark = MARKED;}

void node::un_mark() {_mark = UNMARKED;}

bool node::is_marked() {return _mark == MARKED;}

// int node::_ref_mark = 1;
