//-----------------------------------------------------------------------------------
// Implementation of a MPS algorithm via PC-tree.
//-----------------------------------------------------------------------------------

#include "mps.h"

// #define DEBUG
#define DEBUG_MUTATION

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
	_mark = 0;
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
	for (int i = 0; i < _children.size(); ++i) {
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
	for (int i = 0; i < _adj_list.size(); ++i) {
		if (!_adj_list[i]->is_marked()) {
			_adj_list[i]->_parent = this;
			_adj_list[i]->DFS_visit(dfsList, index);
		}
	}
	set_post_order_index(index);
	dfsList.push_back(this);
	++index;
}


void node::guided_DFS_visit(vector<node *> &dfsList, 
                            vector<node *> &node_list, 
                            int &return_index, 
                            vector<int> rev_post_order,
                            int prev_node)
{

	mark();

    // purpose of this block: create list of neighbors ordered in the order they appear in rev_post_order
    // we want to select neighbors that match the rev_post_order at the specific traversal_index

    // create an unordered set to efficiently check for presence of an element
    std::unordered_set<int> neighbor_set; 
    for (int i = 0; i < _adj_list.size(); ++i) {
        neighbor_set.insert(_adj_list[i]->node_id());
    }
    // when an element in rev_post_order is found in neighbor_set, we add that to neighbor_list
    // this produces a neighbor_list that follows the order by which they occur in the rev_post_order
    // it is ok if the neighbor was already visited before, 
    // it would've been marked and will be subsequently ignored
    vector<node *> neighbor_list;
    for (int i = 0; i < rev_post_order.size(); ++i) {
        if (neighbor_set.find(rev_post_order[i]) != neighbor_set.end()) {
            // only add if newly encountered
            if (!node_list[rev_post_order[i]]->is_marked()) {
                neighbor_list.push_back(node_list[rev_post_order[i]]);
            }
        }
    }

    #ifdef DEBUG
    std::cout << "current node:" << this->node_id() << std::endl;
    std::cout << "prev node:" << prev_node << std::endl;
    for (int i = 0; i < neighbor_list.size(); ++i) {
        std::cout << neighbor_list[i]->node_id() << "(" << neighbor_list[i]->is_marked() << ")" << ",";
    }
    std::cout << std::endl;
    #endif


	
	for (int i = 0; i < neighbor_list.size(); ++i) {
		if (!neighbor_list[i]->is_marked()) {
			neighbor_list[i]->_parent = this;
			neighbor_list[i]->guided_DFS_visit(dfsList, node_list, return_index, rev_post_order, this->node_id());
		}
	}

    set_post_order_index(return_index);
	dfsList.push_back(this);
	++return_index;
}


void node::mutated_DFS_visit(vector<node*> &dfsList, 
                             vector<node*> &node_list, 
                             int &return_index, 
                             int &traversal_index, 
                             vector<int> rev_post_order, 
                             int mutate_point)
{

    // mark current node
	mark();
    
    // purpose of this block: create list of neighbors ordered in the order they appear in rev_post_order
    // we want to select neighbors that match the rev_post_order at the specific traversal_index

    // create an unordered set to efficiently check for presence of an element
    std::unordered_set<int> neighbor_set; 
    for (int i = 0; i < _adj_list.size(); ++i) {
        neighbor_set.insert(_adj_list[i]->node_id());
    }
    // when an element in rev_post_order is found in neighbor_set, we add that to neighbor_list
    // this produces a neighbor_list that follows the order by which they occur in the rev_post_order
    // it is ok if the neighbor was already visited before, 
    // it would've been marked and will be subsequently ignored
    vector<node *> neighbor_list;
    for (int i = 0; i < rev_post_order.size(); ++i) {
        if (neighbor_set.find(rev_post_order[i]) != neighbor_set.end()) {
            neighbor_list.push_back(node_list[rev_post_order[i]]);
        }
    }

    

    // since we increment the index before this line, the current index is "index - 1"
    // if the current index matches the mutate_point, then we know this is the cycle to mutate
    if (traversal_index == mutate_point) {
        // Create a random number generator and seed it
        // std::cout << "mutated at index: " << index - 1<< "and at mutate point: " << mutate_point << std::endl;
        std::random_device rd;
        std::mt19937 rng(rd());
        // Use std::shuffle to shuffle the elements in the vector
        std::shuffle(neighbor_list.begin(), neighbor_list.end(), rng);

        #ifdef DEBUG_MUTATION
        std::cout << "current node:" << this->node_id() << std::endl;
        for (int i = 0; i < neighbor_list.size(); ++i) {
            std::cout << neighbor_list[i]->node_id() << "(" << neighbor_list[i]->is_marked() << ")" << ",";
        }
        std::cout << std::endl;
        #endif
    } 

    // increment traversal index after checking
    // next node will receive incremented index
    traversal_index++;
    
    for (int i = 0; i < neighbor_list.size(); ++i)
    {
        if (!neighbor_list[i]->is_marked())
        {
            neighbor_list[i]->_parent = this;
            neighbor_list[i]->mutated_DFS_visit(dfsList, node_list, return_index, traversal_index, rev_post_order, mutate_point);
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
	for (int i = 0; i < _children.size(); ++i) {
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
	for (int i = 0; i < _children.size(); ++i) {
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
void node::mark() {_mark = _ref_mark;}

void node::init_mark() {++_ref_mark;}

void node::un_mark() {_mark = 0;}

bool node::is_marked() {return _mark == _ref_mark;}

int node::_ref_mark = 1;
