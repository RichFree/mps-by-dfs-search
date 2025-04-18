//-----------------------------------------------------------------------------------
// Implementation of a MPS algorithm via PC-tree.
//-----------------------------------------------------------------------------------

#include "mps.h"

// #define DEBUG

// constructor can be made empty
maximal_planar_subgraph_finder::maximal_planar_subgraph_finder() {}

//Destructor
maximal_planar_subgraph_finder::~maximal_planar_subgraph_finder() {
    for (size_t i = 0; i < _node_list.size(); ++i) delete _node_list[i];
    for (size_t i = 0; i < _new_node_list.size(); ++i) delete _new_node_list[i];
}

node* 
maximal_planar_subgraph_finder::get_new_node(node_type t) {
    _new_node_list.push_back(new node(t));
    return _new_node_list[_new_node_list.size()-1];
}

vector<int>
maximal_planar_subgraph_finder::return_post_order() {
    vector<int> post_order;
    // we have arg number of elements
    post_order.reserve(_post_order_list.size()); // reserve for decreased reallocation
    for (size_t i = 0; i < _post_order_list.size(); ++i) {
        post_order.push_back(_post_order_list[i]->node_id());
    }
    return post_order;
}

bool 
maximal_planar_subgraph_finder::sort_by_order(const std::unordered_map<int, int> &node_id_to_pos, node* a, node* b) {
    auto iter_a = node_id_to_pos.find(a->node_id());
    auto iter_b = node_id_to_pos.find(b->node_id());

    // second yields the position
    return iter_a->second < iter_b->second;
}

// this is very inefficient
bool 
maximal_planar_subgraph_finder::sort_by_free_neighbors(node* a, node* b) {
    int count_a = a->get_unmarked_count();
    int count_b = b->get_unmarked_count();
    // count number of unmarked nodes in each node's neighbor list
    return count_a < count_b;
}

// ---- for main to find connected components

void
maximal_planar_subgraph_finder::dfs_cc(node* root_node, vector<node*> &return_node_list) {
    // mark all vertices as not visited
    vector<bool> in_post_order(_node_list.size(), false);

    // create stack for DFS
    stack<node*> stack;

    // push the current root node into the stack
    stack.push(root_node);

    while (!stack.empty()) {
        // pop node from stack
        node* current_node = stack.top();
        // std::cout << "1 top: " << current_node->node_id() << '\n';


        // stack may contain same vertex twice
        // print the popped item only if it is not visited
        // proceed if current node is not markd
        if (!current_node->is_marked()) {
            current_node->mark();
            vector<node*> neighbor_list = current_node->_adj_list;
            // stack is LIFO - last element in is first to be popped
            // hence we use a reverse iterator
            for (auto it = neighbor_list.rbegin(); it != neighbor_list.rend(); ++it) {
                node* node = (*it);
                // only add neighbor to stack if it is not visited
                if (!node->is_marked()) {
                    // std::cout << "2 add: " << node->node_id() << '\n';
                    node->set_parent(current_node);
                    stack.push(node);
                }
            }
        } else {
            // it is possible to see a node again for many times
            // this section deals with marked nodes that have been added many time
            // we want to skip nodes that were already added to the output
            if (in_post_order[current_node->node_id()]) {
                stack.pop();
                continue;
            }

            // here we save the node
            return_node_list.push_back(current_node);
            in_post_order[current_node->node_id()] = true;
            stack.pop();
        }
    }



}
// -----



void
maximal_planar_subgraph_finder::dfs(node* root_node, int &post_order_id) {
    // mark all vertices as not visited
    vector<bool> in_post_order(_node_list.size(), false);

    // create stack for DFS
    stack<node*> stack;

    vector<node*> local_post_order_list;
    // push the current root node into the stack
    stack.push(root_node);

    while (!stack.empty()) {
        // pop node from stack
        node* current_node = stack.top();
        // std::cout << "1 top: " << current_node->node_id() << '\n';


        // stack may contain same vertex twice
        // print the popped item only if it is not visited
        // proceed if current node is not markd
        if (!current_node->is_marked()) {
            current_node->mark();
            vector<node*> neighbor_list = current_node->_adj_list;
            // stack is LIFO - last element in is first to be popped
            // hence we use a reverse iterator
            for (auto it = neighbor_list.rbegin(); it != neighbor_list.rend(); ++it) {
                node* node = (*it);
                // only add neighbor to stack if it is not visited
                if (!node->is_marked()) {
                    // std::cout << "2 add: " << node->node_id() << '\n';
                    node->set_parent(current_node);
                    stack.push(node);
                }
            }
        } else {
            // it is possible to see a node again for many times
            // this section deals with marked nodes that have been added many time
            // we want to skip nodes that were already added to the output
            if (in_post_order[current_node->node_id()]) {
                stack.pop();
                continue;
            }

            // seeing it again for the first time means that we have ran out of next neighbors
            // it is going back up the traversed nodes
            // std::cout << "3 pop: " << current_node->node_id() << '\n';
            current_node->set_post_order_index(post_order_id++);
            local_post_order_list.push_back(current_node);
            in_post_order[current_node->node_id()] = true;
            stack.pop();
        }
    }

    _post_order_list.insert(_post_order_list.end(), local_post_order_list.begin(), local_post_order_list.end());


}

void 
maximal_planar_subgraph_finder::post_order_traversal_iterative() {
    // node::init_mark();
    int post_order_id = 0;
    // we need to iterate through nodes in case graph is disconnected
    for (size_t i = 0; i < _node_list.size(); ++i) {
        if (!_node_list[i]->is_marked()) {
            // set this node at i to be the root node of a new DFS tree
            dfs(_node_list[i], post_order_id);
        }
    }

}

void
maximal_planar_subgraph_finder::dfs_guided(node* root_node, int &post_order_id, const unordered_map<int, int> &node_id_to_pos) {
    // mark all vertices as not visited
    vector<bool> in_post_order(_node_list.size(), false);

    // create stack for DFS
    stack<node*> stack;

    vector<node*> local_post_order_list;
    // push the current root node into the stack
    stack.push(root_node);

    while (!stack.empty()) {
        // pop node from stack
        node* current_node = stack.top();


        // stack may contain same vertex twice
        // print the popped item only if it is not visited
        // proceed if current node is not markd
        if (!current_node->is_marked()) {
            current_node->mark();
            vector<node*> neighbor_list = current_node->_adj_list;
            // change order of neighbors here
            std::sort(neighbor_list.begin(), neighbor_list.end(), [this, &node_id_to_pos](node *a, node *b)
                        { return sort_by_order(node_id_to_pos, a, b); });

            // purpose of this block: create list of neighbors ordered in the
            // order they appear in rev_post_order
            // we want to select neighbors that match the rev_post_order at the
            // specific traversal_index


            // stack is LIFO - last element in is first to be popped
            // hence we use a reverse iterator
            for (size_t i = neighbor_list.size() - 1; i != std::numeric_limits<size_t>::max(); --i) {
                node* node = neighbor_list[i];
                // only add neighbor to stack if it is not visited
                if (!node->is_marked()) {
                    node->set_parent(current_node);
                    stack.push(node);
                }
            }
        } else {
            // it is possible to see a node again for many times
            // this section deals with marked nodes that have been added many time
            // we want to skip nodes that were already added to the output
            if (in_post_order[current_node->node_id()]) {
                stack.pop();
                continue;
            }

            // seeing it again for the first time means that we have ran out of next neighbors
            // it is going back up the traversed nodes
            // std::cout << "3 pop: " << current_node->node_id() << '\n';
            current_node->set_post_order_index(post_order_id++);
            local_post_order_list.push_back(current_node);
            in_post_order[current_node->node_id()] = true;
            stack.pop();
        }
    }


    _post_order_list.insert(_post_order_list.end(), local_post_order_list.begin(), local_post_order_list.end());
}


// Determine the post-order-list by a DFS-traversal.
// take in a post-order argument then traces the graph in the same order
// return is by reference via _post_order_list
void 
maximal_planar_subgraph_finder::guided_post_order_traversal_iterative(const vector<int> &post_order) {
    // node::init_mark();

    // use unordered_map to map node_id to position in reversed post_order
    unordered_map<int, int> node_id_to_pos;
    node_id_to_pos.reserve(post_order.size());
    int j = 0;
    // we flip the post_order vector around
    for (size_t i = post_order.size() - 1; i != std::numeric_limits<size_t>::max(); --i) {
        node_id_to_pos[post_order[i]] = j++;
    }

    int post_order_id = 0;
    int end_condition = _node_list.size();
    // we start from the end of the post_order, which is the root node
    int start = post_order[post_order.size() - 1];
    int i = start;

    // reserve for _post_order_list to decrease reallocation
    _post_order_list.reserve(_node_list.size());


    while (true)
    {
        if (((start > 0) && (i == (start - 1))) || ((start == 0 ) && (i == end_condition - 1)))
        {
            if (!_node_list[i]->is_marked())
            {
                // set this node at i to be the root node of a new DFS tree
                dfs_guided(_node_list[i], post_order_id, node_id_to_pos);

            }
            break;
        }
        if (!_node_list[i]->is_marked())
        {
            dfs_guided(_node_list[i], post_order_id, node_id_to_pos);
        }
        i = (i + 1) % end_condition;
    }
}


void maximal_planar_subgraph_finder::dfs_mutated(node *root_node, int &post_order_id,
                                                 const unordered_map<int, int> &node_id_to_pos,
                                                 int mutate_point,
                                                 int &traversal_index,
                                                 mt19937 rng) {

    // traversal index tracks how many nodes we have moved in the tree
    // mark all vertices as not visited
    vector<bool> in_post_order(_node_list.size(), false);
    vector<node*> local_post_order_list;

    // create stack for DFS
    stack<node*> stack;

    // push the current root node into the stack
    stack.push(root_node);

    while (!stack.empty()) {
        // pop node from stack
        node* current_node = stack.top();


        // stack may contain same vertex twice
        // print the popped item only if it is not visited
        // proceed if current node is not markd
        if (!current_node->is_marked()) {
            current_node->mark();
            vector<node*> neighbor_list = current_node->_adj_list;
            // change order of neighbors here
            // purpose of this block: create list of neighbors ordered in the
            // order they appear in rev_post_order
            // we want to select neighbors that match the rev_post_order at the
            // specific traversal_index

            // if the current index comes before mutate point, we do guided traversal
            if (traversal_index < mutate_point) {
                std::sort(neighbor_list.begin(), neighbor_list.end(), [this, &node_id_to_pos](node *a, node *b)
                        { return sort_by_order(node_id_to_pos, a, b); });
            }

            // increment traversal index when encountering a node for first time
            traversal_index++;

            // stack is LIFO - last element in is first to be popped
            // hence we use a reverse iterator

            for (size_t i = neighbor_list.size() - 1; i != std::numeric_limits<size_t>::max(); --i) {
                if (traversal_index - 1 >= mutate_point) { // undo the earlier increment
                    std::sort(neighbor_list.begin(), neighbor_list.begin() + i, [this](node *a, node *b)
                            { return sort_by_free_neighbors(a, b); });
                    // we shuffle the section close to the index
                    // std::shuffle(neighbor_list.begin() + (i * 7/10), neighbor_list.begin() + i, rng);
                    // rather than shuffle, we swap the node at i and some position between i*7/10 and i
                    std::uniform_int_distribution<size_t> dist(i*7/10, i);
                    size_t select = dist(rng);
                    node* temp_node = neighbor_list[i];
                    neighbor_list[i] = neighbor_list[select];
                    neighbor_list[select] = temp_node;
                }
                node* node = neighbor_list[i];
                // only add neighbor to stack if it is not visited
                if (!node->is_marked()) {
                    node->set_parent(current_node);
                    stack.push(node);
                }
            }
        } else {
            // it is possible to see a node again for many times
            // this section deals with marked nodes that have been added many time
            // we want to skip nodes that were already added to the output
            if (in_post_order[current_node->node_id()]) {
                stack.pop();
                continue;
            }

            // seeing it again for the first time means that we have ran out of next neighbors
            // it is going back up the traversed nodes
            // std::cout << "3 pop: " << current_node->node_id() << '\n';
            current_node->set_post_order_index(post_order_id++);
            local_post_order_list.push_back(current_node);
            in_post_order[current_node->node_id()] = true;
            stack.pop();
        }
    }
    _post_order_list.insert(_post_order_list.end(), local_post_order_list.begin(), local_post_order_list.end());
}

//Determine the post-order-list by a DFS-traversal.
// take in a post-order argument then traces the graph in the same order
// return is by reference via _post_order_list
void maximal_planar_subgraph_finder::mutated_post_order_traversal_iterative(const vector<int> &post_order,
                                                                            int mutate_point) {
    // node::init_mark();

    // implementation: use unordered_map to map node_id to position in reversed post_order
    unordered_map<int, int> node_id_to_pos;
    node_id_to_pos.reserve(post_order.size());
    int j = 0;
    // we flip the post_order vector around
    for (size_t i = post_order.size() - 1; i != std::numeric_limits<size_t>::max(); --i) {
        node_id_to_pos[post_order[i]] = j++;
    }

    int post_order_id = 0;
    int traversal_index = 0;

    // setup random rng function
    std::random_device rd;
    std::mt19937 rng{rd()};


    int start = 0;
    // if we mutate first node, we will select a random starting node
    if (mutate_point == 0) {
        int first_value = 0;
        int last_value = post_order.size() - 1;  
        std::uniform_int_distribution<> dist{first_value, last_value}; 
        start = post_order[dist(rng)];
    // if we don't mutate first, we just use the root node of the post_order
    } else {
        start = post_order[post_order.size() - 1];
    }

    // reserve for _post_order_list to decrease reallocation
    _post_order_list.reserve(_node_list.size());

    // set loop variables
    int i = start;
    int end_condition = _node_list.size();
    // this loop assumes start is not from 0
    // if starting index is not 0, it just increments and loops around until it encounters the element before it
    while (true)
    {
        if (((start > 0) && (i == (start - 1))) || ((start == 0 ) && (i == end_condition - 1)))
        {
            if (!_node_list[i]->is_marked())
            {
                dfs_mutated(_node_list[i], post_order_id, node_id_to_pos,
                            mutate_point, traversal_index, rng);
            }
            break;
        }
        if (!_node_list[i]->is_marked())
        {
            dfs_mutated(_node_list[i], post_order_id, node_id_to_pos,
                        mutate_point, traversal_index, rng);

        }
        i = (i + 1) % end_condition;
    }
}


// function to print post order 
void
maximal_planar_subgraph_finder::print_post_order() {
    int current_index;
    int parent_index;
    for (size_t i = 0; i < _post_order_list.size(); ++i) {
        current_index = _post_order_list[i]->node_id();
        if (_post_order_list[i]->parent() != nullptr) {
            parent_index =_post_order_list[i]->post_order_index();
        }
        std::cout << "(" << current_index << ", " << parent_index << ")"  << ",";
    }
    std::cout << std::endl;
}


void
maximal_planar_subgraph_finder::print_edge_list() {
    int current_index;
    int parent_index;
    for (size_t i = 0; i < _edge_list.size(); ++i) {
        int first = _edge_list[i].first->node_id();
        int second = _edge_list[i].second->node_id();
                std::cout << "(" << first << ", " << second << ")"  << ",";
    }
    std::cout << std::endl;
}



//Sort the adj-list of every node increasingly according to post-order-index.
void
maximal_planar_subgraph_finder::sort_adj_list() {
    vector<vector<node*> > vecList;
    vecList.resize(_post_order_list.size());
    for (size_t i = 0; i < _post_order_list.size(); ++i) {
        for (int j = 0; j < _post_order_list[i]->degree(); ++j) {
            vecList[_post_order_list[i]->adj(j)->post_order_index()].push_back(_post_order_list[i]);
        }
    }
    for (size_t i = 0; i < _post_order_list.size(); ++i) {
        _post_order_list[i]->set_adj_list(vecList[i]);
    }
}

//Determine edge-list, and back-edge-list.
//Order the edges properly.
void 
maximal_planar_subgraph_finder::determine_edges() {
    for (size_t i = 0; i < _post_order_list.size(); ++i) {
        // if there is no parent, then just ignore the node
        if (_post_order_list[i]->parent() == nullptr) continue;
        _post_order_list[i]->set_1st_label(_post_order_list[i]->parent()->post_order_index());
        _edge_list.push_back(pair<node*, node*> (_post_order_list[i]->parent(), _post_order_list[i]));
    }
    for (size_t i = 0; i < _post_order_list.size(); ++i) {
        for (int j = 0; j < _post_order_list[i]->degree(); ++j) {
            // this ensures that added edges are indeed back edges
            // position in the _post_order_list is from leaf to root
            // the node is the child, the adj is the ancestor
            // the id in the _post_order_list of child must be lesser than the ancestor
            if (_post_order_list[i]->adj(j)->post_order_index() > static_cast<int>(i)) break;
            // if the child points to an immediate parent, then also ignore it
            if (_post_order_list[i]->adj(j)->get_1st_label() == static_cast<int>(i)) continue;
            _back_edge_list.push_back(pair<node*, node*> (_post_order_list[i], _post_order_list[i]->adj(j)));
            _is_back_edge_eliminate.push_back(false);
        }
    }
    for (size_t i = 0; i < _post_order_list.size(); ++i) {
        _post_order_list[i]->set_1st_label(INT_MAX);
    }
}

//The main part of the whole algorithm: Back-edge-traversal
void 
maximal_planar_subgraph_finder::back_edge_traversal() {
    node* i_node = 0;
    node* current_node = 0;
    for (size_t i = 0; i < _back_edge_list.size(); ++i) {
        current_node = _back_edge_list[i].second;
        i_node = _back_edge_list[i].first;
        if (!back_edge_traversal(current_node, i_node->post_order_index())) _is_back_edge_eliminate[i] = true;
    }
}

//sub-function for the for-loop of back_edge_traversal().
bool maximal_planar_subgraph_finder::back_edge_traversal(node* traverse_node, int index) {
    node* parent_node; //The next node to traverse.
    //If the node has been deleted. 
    if (traverse_node == 0 || traverse_node->get_2nd_label() == DELETED) {
        return false;
    }
    //We have reached the i-node, stop.
    if (traverse_node->post_order_index() == index) {
        return true;
    }
    //Case 1
    if (traverse_node->get_2nd_label() == NOT_VISITED) {
        //1.1
        if (traverse_node->get_1st_label() == INT_MAX) {
            traverse_node->set_1st_label(index);
            parent_node = traverse_node->parent();
        }
        //1.2
        else if (traverse_node->get_1st_label() == index) {
            return true;
        }
        //1.3
        else if (traverse_node->get_1st_label() < index) {
            parent_node = construct(traverse_node);
            traverse_node->set_1st_label(index);
        }
    }
    //Case 2: Find the top-tier c-node.
    else {
        node* my_c_node = find(traverse_node);
        make_essential(traverse_node, my_c_node);
        //2.1
        if (my_c_node->get_1st_label() == index) {
            parent_node = my_c_node;
        }
        //2.2
        else if (my_c_node->get_1st_label() < index) {
            node* my_c_node_2 = construct(my_c_node, traverse_node);
            parent_node = my_c_node_2;
        }
        traverse_node->set_1st_label(index);
        traverse_node->set_2nd_label(NOT_VISITED);
    }
    if (back_edge_traversal(parent_node, index)) {
        if (parent_node != _post_order_list[index]) parent_node->add_child(traverse_node);
        return true;
    }
    else {
        eliminate(traverse_node);
        return false;
    }
}

//The p_node is originally a normal node in c_node's boundary cycle.
//Now we transfer it to be an essential node by the following steps:
//1. Create a replica-node of p_node to be representative of p_node in c_node.
//2. Take out the p_node from c_node, and then set the parent of p_node to be c_node.
//Note: We are not adding p_node to the c_node's children-list.
void 
maximal_planar_subgraph_finder::make_essential(node* p_node, node* c_node) {
    node* sentinel = get_new_node(REPLICA_NODE);
    node* n0 = p_node->neighbor(0);
    node* n1 = p_node->neighbor(1);
    sentinel->init_replica(p_node, c_node);
    c_node->add_essential(sentinel);
    sentinel->set_to_boundary_path(n0, n1);
    sentinel->inherit_AE(p_node);
    n0->set_neighbor(n0->get_next(p_node), sentinel);
    n1->set_neighbor(n1->get_next(p_node), sentinel);
    p_node->set_neighbor((node*)0, (node*)0);
    p_node->set_parent(c_node);
}

//Find the top-tier c-node of the input node.
//Note: We don't set the input node to be essential node of the top-tier c-node.
//When terminated, the input node will be in the boundary cycle of top-tier c-node.
node* maximal_planar_subgraph_finder::find(node* n) {
    pair<pair<node*, node*>, pair<node*, node*> > boundary;
    node* c_node_new = 0;
    int c_node_size = 0;
    node* return_node = 0;
    if (n->parent() == 0) {
        //If n is already a node in boundary cycle.
        //Note: n must not be an essential node, otherwise it will never enter the function.
        //Find the first(nearest to n) essential node.
        boundary.first = parallel_search_sentinel(n, c_node_new);
    }
    else {
        //If n is not a node in boundary cycle.
        //It is in an Artificial edge.
        //Trim it.
        boundary = trim(n);
        //Find the first(nearest to n) essential node.
        boundary.first = parallel_search_sentinel(boundary.first.first, boundary.first.second, boundary.second.first, boundary.second.second, c_node_new);
    }
    //Find the c-node in the current hierachy .
    //If it is top-tier, return it.
    if (c_node_new != 0) return c_node_new;
    c_node_new = (boundary.first).first->get_c_node();
       
    //If not, find the two nearest essential node, eliminate the rest nodes.
    c_node_size = c_node_new->c_node_size();
    boundary.second = count_sentinel_elimination(boundary.first, c_node_size);
    //Go to the higher hierachy, and continue to find.
    if (c_node_new->get_2nd_label() == ARTIFICIAL_EDGE) {
        //A peculiar technic: 
        //Remove all the children of c_node_new but the one that should remains(Let it be u).
        //Remove all other essential nodes, pretend to be a c-node of size equals 2.
        //Call find(u).
        node* u = 0;
        for (int i = 0; i < c_node_new->child_num(); ++i) {
            if (node::is_same(boundary.first.first, c_node_new->child(i)) || node::is_same(boundary.second.first, c_node_new->child(i))) {
                u = c_node_new->child(i);
            }
            else eliminate(c_node_new->child(i));
        }
        c_node_new->clear_children();
        c_node_new->add_child(u);
        c_node_new->clear_essential();
        c_node_new->add_essential(boundary.first.first);
        c_node_new->add_essential(boundary.second.first);
        return_node = find(u);
    }
    else return_node = find(c_node_new);
    //Merge the part of boundary cycle remains in current hierachy to the top-tier c-node.
    merge(boundary, c_node_new);
    return return_node;
}

//The list_node is a c-node. 
//The boundary indicates the part of the boundary cycle of c-node needs to be merge to the higher hierachy.
//Replace the list_node by boundary.
//Set list_node to be DELETED.
//Note: We do not eliminate anything in this function.
void 
maximal_planar_subgraph_finder::merge(pair<pair<node*, node*>, pair<node*, node*> > boundary, node* list_node) {
    node* n0 = list_node->neighbor(0);
    node* n1 = list_node->neighbor(1);
    node* s0, * s0_prev;
    node* s1, * s1_prev;
    if (node::is_same(boundary.first.first, n0)) {
        s0 = boundary.first.first;
        s0_prev = boundary.first.second;
        s1 = boundary.second.first;
        s1_prev = boundary.second.second;
    }
    else {
        s0 = boundary.second.first;
        s0_prev = boundary.second.second;
        s1 = boundary.first.first;
        s1_prev = boundary.first.second;
    }
    if (s0_prev == s1 && s1_prev == s0) {
        n0->set_neighbor(n0->get_next(list_node), n1);
        n1->set_neighbor(n1->get_next(list_node), n0);
    }
    else {
        n0->set_neighbor(n0->get_next(list_node), s0_prev);
        n1->set_neighbor(n1->get_next(list_node), s1_prev);
        s0_prev->set_neighbor(s0_prev->get_next(s0), n0);
        s1_prev->set_neighbor(s1_prev->get_next(s1), n1);
    }
    //Inherit AE.
    n0->inherit_AE(s0);
    n1->inherit_AE(s1);
    //Delete c-node
    list_node->set_2nd_label(DELETED);
}

//Set u and its subtree to be DELETED.
//If u has some AE, eliminate them.
//We don't do anything about u's parent, neighborhood.(Only children are affected.)
//If u is a p-node, we don't eliminate anything in the lower hierachy that corresponds to the same p-node.
//If u is a c-node, we eliminate all nodes in u's boundary cycle.
void 
maximal_planar_subgraph_finder::eliminate(node* u) {
    if (u->get_2nd_label() == DELETED) return;
       u->set_2nd_label(DELETED);
    if (u->type() == C_NODE) {
        node* list_node = u->get_a_list_node();
        node* n0, * n0_prev;;
        node* temp = 0;
        n0 = list_node;
        n0_prev = list_node->neighbor(0);
        while (true) {
            eliminate(n0);
            temp = n0;
            n0 = n0->get_next(n0_prev);
            n0_prev = temp;
            if (n0 == list_node) break;
        }
    }
    else if (u->type() == P_NODE) {
        for (int i = 0; i < u->degree(); ++i) {
            if (u->adj(i)->post_order_index() < u->post_order_index() && u->adj(i)->get_1st_label() == INT_MAX) eliminate(u->adj(i));
        }
    }
    if (u->AE(0) != 0) eliminate(u->AE(0));
    if (u->AE(1) != 0) eliminate(u->AE(1));
    for (int i = 0; i < u->child_num(); ++i) {
        eliminate(u->child(i));
    }
}

//Eliminate the AE of(u,v)-link that points to u.(If exists)
void 
maximal_planar_subgraph_finder::eliminate_AE(node* u, node* v) {
    int v_index = v->post_order_index();
    if (u->AE(0) != 0 && u->AE(0)->get_1st_label() == v_index) {
        eliminate (u->AE(0));
        u->set_AE(0, 0);
    }
    if (u->AE(1) != 0 && u->AE(1)->get_1st_label() == v_index) {
        eliminate (u->AE(1));
        u->set_AE(1, 0);
    }
}

//The input node u must not be c-node.
//The traversed node is in the AE = (up <- down).
//The returned boundary = [up, up_prev ..., down_prev, down].
//Direction: up it higher than down.
pair<pair<node*, node*>, pair<node*, node*> > 
maximal_planar_subgraph_finder::trim(node* u) {
    node* up = 0;
    node* down = 0;
    //Since we may do c-node extension in the future, we need to memorize next in order to deduce prev.
    node* up_next = 0;
    node* down_next = 0;
    node* new_AE_root = 0;
    //The index from small to large indicates the path that we traversed, note that u = node_list[0].
    vector<node*> node_list;
    node* curr = u;
    node_list.push_back(u);
    //Traverse upward.
    while (true) {
        curr = curr->parent();
        if (curr->type() == AE_VIRTUAL_ROOT) {
            up = curr->parent();
            //case 1: We are in a newly created c-node.
            //It has only one AE, and the two neighbor-pointer point to the same one.
            if (up->neighbor(0) == up->neighbor(1)) {
                down = up->neighbor(0);
                up->set_neighbor(down, node_list[node_list.size()-1]);
                down->set_neighbor(up, node_list[0]);
                curr->remove_child(node_list[node_list.size()-1]);
                //There's no other child, just delete the AE.
                if (curr->child_num() == 0) {
                    up->set_AE(0, 0);
                    up->set_AE(1, 0);
                }
            }
            //case 2: General case.
            else {
                if (up->neighbor(0)->post_order_index() == curr->get_1st_label()) down = up->neighbor(0);
                else down = up->neighbor(1);
                up->set_neighbor(up->get_next(down), node_list[node_list.size()-1]);
                down->set_neighbor(down->get_next(up), node_list[0]);
                curr->remove_child(node_list[node_list.size()-1]);
                eliminate_AE(up, down);
            }
            break;
        }
        node_list.push_back(curr);
    }
    //Set the "downward" AE of node_list[0].
    new_AE_root = get_new_node(AE_VIRTUAL_ROOT);
    new_AE_root->init_AE(node_list[0]);
    //Eliminate the children other than the path.
    for (size_t i = 1; i < node_list.size(); ++i) {
        for (int j = 0; j < node_list[i]->child_num(); ++j) {
            if (node_list[i]->child(j) != node_list[i-1]) eliminate(node_list[i]->child(j));
        }
    }
    //Set to the boundary path.
    if (node_list.size() == 1) node_list[0]->set_to_boundary_path(up, down);
    else {
        node_list[0]->set_to_boundary_path(down, node_list[1]);
        node_list[node_list.size()-1]->set_to_boundary_path(up, node_list[node_list.size()-2]);
        for (size_t i = 1; i < node_list.size()-1; ++i) {
            node_list[i]->set_to_boundary_path(node_list[i-1], node_list[i+1]);
        }
    }
    //Set the next of up and down.
    up_next = up->get_next(node_list[node_list.size()-1]);
    down_next = down->get_next(node_list[0]);
    //Unfold the c-nodes in the node_list.
    for (size_t i = 0; i < node_list.size(); ++i) {
        if (node_list[i]->type() == C_NODE) c_node_extension(node_list[i]);
    }
    //Return the new boundary.
    return pair<pair<node*, node*>, pair<node*, node*> > (pair<node*, node*>(up, up->get_next(up_next)), pair<node*, node*>(down, down->get_next(down_next)));
}

//The trim's sub-function.
//The input c-node does not contain any children nor parent, but it has two neighbors, which is originally c-node's parent and one child.
//If c-node's size equals 2, then we don't need to unfold it.
//Otherwise, it must has size equals 3. 
//And then we find that redundent essential node, and remove the nodes that need not remains.
//Merge the remain part to the higher hierachy.
void 
maximal_planar_subgraph_finder::c_node_extension(node* c_node) {
    //size == 2
    if (c_node->c_node_size() == 2) return;
    //size == 3
    node* sentinel = 0;
    for (int i = 0; i < c_node->c_node_size(); ++i) {
        if (!node::is_same(c_node->essential(i), c_node->neighbor(0)) && !node::is_same(c_node->essential(i),  c_node->neighbor(1))) {
            sentinel = c_node->essential(i);
            break;
        }
    }
    eliminate(sentinel);
    //The two other essential nodes and their subsequent neighbor.
    pair<node*, node*> sentinel_0; 
    pair<node*, node*> sentinel_1;
    node* n0, * n0_prev = sentinel;
    node* n1, * n1_prev = sentinel;
    node* temp = 0;
    n0 = sentinel->neighbor(0);
    n1 = sentinel->neighbor(1);
    while (true) {//Toward the direction of n0.
        if (n0->is_sentinel()) {//If we meet a essential node, stop, don't remove it.
            sentinel_0 = pair<node*, node*> (n0, n0->get_next(n0_prev));
            break;
        }
        eliminate(n0);
        temp = n0;
        n0 = n0->get_next(n0_prev);
        n0_prev = temp;
    }
    while (true) {//Toward the direction of n0.
        if (n1->is_sentinel()) {//If we meet a essential node, stop, don't remove it.
            sentinel_1 = pair<node*, node*> (n1, n1->get_next(n1_prev));
            break;
        }
        eliminate(n1);
        temp = n1;
        n1 = n1->get_next(n1_prev);
        n1_prev = temp;
    }

    //Remember to remove the AE toward two essential nodes that is in the delete region. 
    eliminate_AE(sentinel_0.first, sentinel_0.first->get_next(sentinel_0.second));
    eliminate_AE(sentinel_1.first, sentinel_1.first->get_next(sentinel_1.second));
    //Reset the neighborhood of two essential nodes.
    sentinel_0.first->set_neighbor(sentinel_1.first, sentinel_0.second);
    sentinel_1.first->set_neighbor(sentinel_0.first, sentinel_1.second);
    //Merge to upper boundary cycle.
    merge(pair<pair<node*, node*>, pair<node*, node*> >(sentinel_0, sentinel_1), c_node);
}

//u is a normal p-node.
//We'll do the work of elimination, and renewing of children-list.
void 
maximal_planar_subgraph_finder::recursively_shaving(node* u) { 
    node* parent_node = 0;
    node* node_x = 0;
    pair<node*, node*> new_two_child;
    vector<node*> new_child_list;
    //p-node
    if (u->type() == P_NODE) {
        for (int i = 0; i < u->child_num(); ++i) recursively_shaving(u->child(i));
    }
    //c-node
    else {
        //We don't need to shave if u has only one child.
        if (u->child_num() == 1) {
            recursively_shaving(u->child(0));
            return;
        }
        //More than one child.
        parent_node = u->parent();
        //Find node_x, and shave it.
        for (int i = 0; i < u->c_node_size(); ++i) {
            if (node::is_same(u->essential(i), parent_node)) {
                node_x = u->essential(i);
                new_two_child = shave(node_x);
                break;
            }
        }
        //Reset children-list and essential node.
        for (int i = 0; i < u->child_num(); ++i) {
            if (node::is_same(u->child(i), new_two_child.first) || node::is_same(u->child(i), new_two_child.second)) new_child_list.push_back(u->child(i));
            else eliminate(u->child(i));
        }
        u->clear_children();
        u->clear_essential();
        u->add_essential(node_x);
        u->add_essential(new_two_child.first);
        u->add_essential(new_two_child.second);
        u->add_child(new_child_list[0]);
        u->add_child(new_child_list[1]);
        for (int i = 0; i < u->child_num(); ++i) recursively_shaving(u->child(i));
    }
}

//In this function, we only deal with inner part of c-node.
//x,y,z are essential nodes, let y,z be x's nearest essential nodes in w's boundary cycle.
//Anything outside [x,y], and[x,z] will be eliminated.
//Definition of y_prev, z_prev: ..., y, y_prev, ..., x, ..., z_prev, z, ...
//Return pair = (y,z). Note: What we return is the replica-node in the inner part of c-node.
//The work of deleting children will be done by recursively_shaving().
pair<node*, node*> 
maximal_planar_subgraph_finder::shave(node* x) {
    //c-node.
    node* c_node = x->get_c_node(); 
    //No need to shave if child_num == 1.
    if (c_node->child_num() == 1) return pair<node*, node*>((node*)0, (node*)0); 
    //sentinel_1 = (y, y_prev). Note: At this time, c-node must has type equals ARTIFICIAL_EDGE, so no problem here.
    pair<node*, node*> sentinel_1 = parallel_search_sentinel(x, c_node);
    //sentinel_2 = (z, z_prev). Same as above.
    pair<node*, node*> sentinel_2 = count_sentinel_elimination(sentinel_1, c_node->child_num());
    return pair<node*, node*>(sentinel_1.first, sentinel_2.first);
}

//Use parallel_search to find essential nodes. Return (essential nodes that we find, its prev).
//x is not in the searching region.
//If the c-node found is top-tier, then set all the nodes during searching a pointer to c-node, set c to be that c-node, and return pair be all null.
pair<node*, node*> 
maximal_planar_subgraph_finder::parallel_search_sentinel(node* x, node* &c) {
    node* n0, * n0_prev = x;
    node* n1, * n1_prev = x;
    n0 = x->neighbor(0);
    n1 = x->neighbor(1);
    return parallel_search_sentinel(n0, n0_prev, n1, n1_prev, c);
}

//Another version of parallel search: n0, n0_prev, ..., n1_prev, n1
//searching region = (...n0] [n1...). Find the nearest essential node.
//return (essential nodes that we find, its prev).
pair<node*, node*> 
maximal_planar_subgraph_finder::parallel_search_sentinel(node* n0, node* n0_prev, node* n1, node* n1_prev, node* & c) {
    node* temp = 0;
    vector<node*> traversed;
    while (true) {
        //If c-node is top-tier.   
        //note: If c points to a c-node traversed in some previous iteration, then it must not be top-tier, so it'll not pass the if-condition.
        if (n0->get_c_node() != 0 && n0->get_c_node()->get_2nd_label() == NOT_VISITED) {
            c = n0->get_c_node();
            break;
        }
        if (n1->get_c_node() != 0 && n1->get_c_node()->get_2nd_label() == NOT_VISITED) {
            c = n1->get_c_node();
            break;
        }
        //If an essential-node found.
        if (n0->is_sentinel()) return pair<node*, node*>(n0, n0_prev);
        if (n1->is_sentinel()) return pair<node*, node*>(n1, n1_prev);
        //Just a normal node..
        traversed.push_back(n0);
        traversed.push_back(n1);
        temp = n0;
        n0 = n0->get_next(n0_prev);
        n0_prev = temp;
        temp = n1;
        n1 = n1->get_next(n1_prev);
        n1_prev = temp;
    }

    //If the c-node found is top-tier, then assign all the traversed node a pointer to c-node.
    for (size_t i = 0; i < traversed.size(); ++i) traversed[i]->set_c_node(c);
    return pair<node*, node*>((node*)0, (node*)0);
}

// sentinel_1= (y, y_prev)
// return pair = (z, z_prev)
// ..., y_prev, y,[ ...(contains num_sentinel-2 essential nodes)...], z, z_prev, ... : eliminate the [...] part.
// Which means, num_sentinel = Number of essential nodes in the region [y, z].
// Note: y, z(Of course, and their prev,) will not be eliminated.
// Note: All the node that correspond to the same one as deleted node in higher hierachy will not be affected.
// The boundary cycle of c-node will be re-connected, AE be properly handled.
// Do nothing outside the c-node.
pair<node*, node*> maximal_planar_subgraph_finder::count_sentinel_elimination(pair<node*, node*> sentinel_1, int num_sentinel) {
    pair<node*, node*> sentinel_2; //(z, z_prev)
    int count = 1;//Count the essential nodes traversed.
    node* n0 = sentinel_1.first->get_next(sentinel_1.second), * n0_prev = sentinel_1.first;//Going one step further.
    node* temp = 0;
    while (true) {
        if (n0->is_sentinel()) {
            ++count;//counter
            if (count == num_sentinel) {//We have reached y. Note: We will not eleminate y.
                sentinel_2.first = n0;
                sentinel_2.second = n0->get_next(n0_prev);
                break;
            }
        }
        eliminate(n0);
        temp = n0;
        n0 = n0->get_next(n0_prev);
        n0_prev = temp;
    }
    //Remember to eliminate AE toward two essential nodes that is in the deleted region.
    eliminate_AE(sentinel_2.first, sentinel_2.first->get_next(sentinel_2.second));
    eliminate_AE(sentinel_1.first, sentinel_1.first->get_next(sentinel_1.second));
    //Reset neighborhood of two essential nodes.
    sentinel_2.first->set_neighbor(sentinel_1.first, sentinel_2.second);
    sentinel_1.first->set_neighbor(sentinel_2.first, sentinel_1.second);
    return sentinel_2;
}

//Used when u has label equals <i, 0>, i<j, where j is current iteration. 
//Create a c-node with u being first essential node, and i being head. Return it.
//We'll done the parent-linke of (u -> c-node -> node_i).
//We don't create child-link here.
//Default label of newly contructed c-node is (INT_MAX, NOT_VISITED).
node*
maximal_planar_subgraph_finder::construct(node* u) {
    //Basic works.
    // int i_label = u->get_1st_label(); // unused
    node* node_i = _post_order_list[u->get_1st_label()];
    parenting_labeling_shaving(u, node_i);

    //Get some new nodes.
    node* i_sentinel = get_new_node(REPLICA_NODE);
    node* u_sentinel = get_new_node(REPLICA_NODE);
    node* new_c_node = get_new_node(C_NODE);
    node* new_AE_root = get_new_node(AE_VIRTUAL_ROOT);

    //Setting of replica-nodes.
    i_sentinel->init_replica(node_i, new_c_node);
    u_sentinel->init_replica(u, new_c_node);
    for (int i = 0; i < u->child_num(); ++i) {
        u_sentinel->add_child(u->child(i));
    }
    new_AE_root->init_AE(u_sentinel);

    //Neighborhood setting of replica-nodes in c-node.
    i_sentinel->set_to_boundary_path(u_sentinel, u_sentinel);
    u_sentinel->set_to_boundary_path(i_sentinel, i_sentinel);

    //Default label of c-node.
    new_c_node->set_1st_label(INT_MAX);
    new_c_node->set_2nd_label(NOT_VISITED);

    //Set essential node of c-node.
    new_c_node->add_essential(i_sentinel);
    new_c_node->add_essential(u_sentinel);

    //Parenting
    new_c_node->set_parent(node_i);
    u->set_parent(new_c_node);

    //Clear children-list of u_node. (which has benn transfered to AE inside c-node.)
    u->clear_children();

    return new_c_node;
    }

//The case when the first explored node is c-node (The input parameter c). 
//The p-node that trigger c(The input parameter p), has p->c parent-link already, and p is essential(not essential before triggered).
//But we don't have c->p child-link yet.
//We are not going to establish that child-link in this function. (Will be done in BET's main loop.)
//Set c to be DELETED.
node* 
maximal_planar_subgraph_finder::construct(node* c, node* p) {
    //Basic works.
    // int i_label = c->get_1st_label(); // unused var
    node* node_i = _post_order_list[c->get_1st_label()];
    parenting_labeling_shaving(p, node_i);
    //note: Now, c must have exactly two children left, and c has a parent-link to p, p has achild link to c, too. 
    //Remember to handle them later.

    //Get some new nodes.
    node* i_sentinel = get_new_node(REPLICA_NODE);
    node* new_c_node = get_new_node(C_NODE);
    i_sentinel->init_replica(node_i, new_c_node);

    //Strategy: Build thisboundary cycle first: (i, child(0), c, child(1), i).
    //And then find the two replica-node corresponding to the two children in c, and merge.
    node* ch0 = c->child(0);
    node* ch1 = c->child(1);
    node* AE_root_0 = get_new_node(AE_VIRTUAL_ROOT);
    node* AE_root_1 = get_new_node(AE_VIRTUAL_ROOT);
    AE_root_0->init_AE(ch0);
    AE_root_1->init_AE(ch1);
    i_sentinel->set_to_boundary_path(ch0, ch1);
    ch0->set_to_boundary_path(i_sentinel, c);
    ch1->set_to_boundary_path(i_sentinel, c);
    c->set_to_boundary_path(ch0, ch1);
        
    //find the boundary in c, merge!
    node* sent_0;
    node* sent_1;
    node* sent_p;
    for (int i = 0; i < c->c_node_size(); ++i) {
        if (node::is_same(c->essential(i), ch0)) sent_0 = c->essential(i);
        else if (node::is_same(c->essential(i), ch1)) sent_1 = c->essential(i);
        else if (node::is_same(c->essential(i), p)) sent_p = c->essential(i);
    }
    merge(pair<pair<node*, node*>, pair<node*, node*> > (pair<node*, node*>(sent_0, sent_0->get_next(sent_1)), pair<node*, node*>(sent_1, sent_1->get_next(sent_0))), c);

    //Set essential-node of c-node.
    new_c_node->add_essential(i_sentinel);
    new_c_node->add_essential(sent_p);

    //Default label of c-node.
    new_c_node->set_1st_label(INT_MAX);
    new_c_node->set_2nd_label(NOT_VISITED);

    //Parenting.
    new_c_node->set_parent(node_i);

    //p-node, p_sent.
    sent_p->set_c_node(new_c_node);
    p->clear_children();
    p->set_parent(new_c_node);

    //Delete c-node
    c->set_2nd_label(DELETED);

    return new_c_node;
}

//Some basic works in constructing c-node.
//u is the first explored node in the newly constructed c-node.
//In the case of newly constructed c-node itself is c-node, u will be the p-node that trigger the c-node.
//And in this case, p->c parent-link has been established, but c->p child-link not.
void 
maximal_planar_subgraph_finder::parenting_labeling_shaving(node* u, node* node_i) {
    //reverse parent-children relation in [u, node_i] as following. 
    //(u-> ... ->y->i) -> (u<- ... <-y , i).
    vector<node*> u_i_path;
    u_i_path.push_back(u);
    while (true) {
        u_i_path.push_back(u_i_path[u_i_path.size()-1]->parent());
        if (u_i_path[u_i_path.size()-1] == node_i) break;
    }
    for (size_t i = 0; i < u_i_path.size()-2; ++i) {
        u_i_path[i]->add_child(u_i_path[i+1]);
        u_i_path[i+1]->set_parent(u_i_path[i]);
    }
    for (size_t i = 0; i < u_i_path.size()-2; ++i) {
        for (int j = 0; j < u_i_path[i+1]->child_num(); ++j) {
            if (u_i_path[i+1]->child(j) == u_i_path[i]) {
                u_i_path[i+1]->remove_child(j);
            }
        }
    }
    u_i_path[0]->set_parent(0);

    //BFS-traversal, all labeled to <i,1>, and then shave the c-node.
    u->recursively_labeling();
    recursively_shaving(u);
}
