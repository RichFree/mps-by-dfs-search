#include <PCTree/PCTree.h>

// #include "hsuPC/include/PCTree.h"
// #include "hsuPC/include/PCNode.h"

#include <ogdf/basic/STNumbering.h>
#include <ogdf/basic/graph_generators.h>
#include <ogdf/basic/GraphCopy.h>
#include <ogdf/fileformats/GraphIO.h>
#include <ogdf/planarity/BoothLueker.h>
#include <ogdf/planarity/BoyerMyrvold.h>
#include <ogdf/basic/extended_graph_alg.h>

#include <ogdf/planarity/booth_lueker/IndInfo.h>
#include <ogdf/planarity/booth_lueker/PlanarPQTree.h>
#include <ogdf/planarity/planar_subgraph_fast/PlanarSubgraphPQTree.h>
#include <ogdf/planarity/PlanarSubgraphFast.h>
#include <ogdf/planarity/PlanarSubgraphBoyerMyrvold.h>
#include <ogdf/planarity/PlanarSubgraphPC.h>
#include <ogdf/planarity/MaximumPlanarSubgraph.h>
#include <ogdf/planarity/MaximalPlanarSubgraphSimple.h>

#include <iostream>
#include <map>
#include <chrono>
#include <getopt.h>

using namespace ogdf;

using std::chrono::time_point;
using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::nanoseconds;


void planarizePQ(const Graph &G, NodeArray<int> &numbering, List<edge> &delEdges) {
		using PlanarLeafKey = booth_lueker::PlanarLeafKey<whaInfo*>;

		NodeArray<SListPure<PlanarLeafKey*>> inLeaves(G);
		NodeArray<SListPure<PlanarLeafKey*>> outLeaves(G);
		Array<node> table(G.numberOfNodes()+1);

		for(node v : G.nodes) {
			for(adjEntry adj : v->adjEntries) {
				edge e = adj->theEdge();
				if (numbering[e->opposite(v)] > numbering[v]) { // sideeffect: ignores selfloops
					PlanarLeafKey* L = new PlanarLeafKey(e);
					inLeaves[v].pushFront(L);
				}
			}
			table[numbering[v]] = v;
		}

		for(node v : G.nodes) {
			for (PlanarLeafKey *L : inLeaves[v]) {
				outLeaves[L->userStructKey()->opposite(v)].pushFront(L);
			}
		}

		SList<PQLeafKey<edge,whaInfo*,bool>*> totalEliminatedKeys;

		PlanarSubgraphPQTree T;
		T.Initialize(inLeaves[table[1]]);
		for (int i = 2; i < G.numberOfNodes(); i++) {
			SList<PQLeafKey<edge,whaInfo*,bool>*> eliminatedKeys; // handle key elimination
			T.Reduction(outLeaves[table[i]], eliminatedKeys); // handle key elimination
			totalEliminatedKeys.conc(eliminatedKeys); // handle key elimination
			T.ReplaceRoot(inLeaves[table[i]]);
			T.emptyAllPertinentNodes();
		}

		// append eliminated edges to delEdges
		for (PQLeafKey<edge, whaInfo*, bool> *key : totalEliminatedKeys) {
			edge e = key->userStructKey();
			delEdges.pushBack(e);
		}

		//cleanup
		for(node v : G.nodes) {
			while (!inLeaves[v].empty()) {
				PlanarLeafKey *L = inLeaves[v].popFrontRet();
				delete L;
			}
		}

		T.Cleanup();	// Explicit call for destructor necessary. This allows to call virtual
		// function CleanNode for freeing node information class.
	}


template<typename PCT, typename PCN, typename PCNT>
void planarizePC(const Graph &G, const NodeArray<int> &numbering, List<edge> &delEdges) {
    // initialize PCTree instance
    PCT T;

    // vector to store nodes of graph in order specified in numbering
    std::vector<node> order(G.numberOfNodes(), nullptr);
    for (node n : G.nodes) {
        order.at(numbering[n] - 1) = n;
    }

    // store leaf representations of edges in PCTree
    std::vector<PCN *> leafRepresentation(G.maxEdgeIndex() + 1, nullptr);
    // vectors declared for later use
    std::vector<edge> outEdges;
    std::vector<PCN *> consecutiveLeaves;

    // iterates over nodes in the 'order' vector
    for (node n : order) {
        // quit condition: if current node is last node 
        if (n == order[G.numberOfNodes() - 1]) {
            break;
        }

        // clear both vectors in for each node in iteration
        consecutiveLeaves.clear();
        outEdges.clear();

        // leafToEdge
        // create a map of PCN to edge
        std::map<PCN *, edge> leafToEdge;

            // iterate over adjacent entries of current node
            for (adjEntry adj : n->adjEntries)
        {
            // check if opposite node has higher numbering (belong to later node)
            // add to outEdges
            if (numbering[adj->theEdge()->opposite(n)] > numbering[n]) {
                outEdges.push_back(adj->theEdge());
            // add to consecutiveLeaves otherwise (declared earlier)
            // leafRepresentation will be filled later on
            } else {
                consecutiveLeaves.push_back(leafRepresentation[adj->theEdge()->index()]);
                // use leafToEdge to map leafRep object to edge object
                // then store the edge in a collection
                // <- insert code here ->
                leafToEdge[leafRepresentation[adj->theEdge()->index()]] = adj->theEdge();


            }
        }

        PCN *mergedLeaf;
        // If first node, new P-node is created
        if (n == order[0]) {
            mergedLeaf = T.newNode(PCNT::PNode);
        // consecutive leaves are merged
        } else {
            // T.makeConsecutive 
            // the function returns a boolean for success
            // I think it checks if its possible to make consecutive 

            // IMPORTANT HERE
            // here we can identify edges that result in non-planarity
            if (!T.makeConsecutive(consecutiveLeaves)){

                // keep trying different combinations until is passes
                std::vector<PCN *> goodLeaves;
                for (PCN* leaf: consecutiveLeaves) {
                    goodLeaves.push_back(leaf);
                    if (T.makeConsecutive(goodLeaves)) {
                        continue;
                    } else {
                        goodLeaves.pop_back();
                        delEdges.pushBack(leafToEdge[leaf]);
                    } 
                }
                consecutiveLeaves.clear();
                consecutiveLeaves.insert(consecutiveLeaves.end(),
                                         goodLeaves.begin(), 
                                         goodLeaves.end());
                

                // for rejected leaves, 
                // return false;
            }
            // remove all consecutive leaves except the first
            // returns a single leaf
            // second argument is "assumeConsecutive", which makeConsecutive checks
            mergedLeaf = T.mergeLeaves(consecutiveLeaves, true);
        }

        OGDF_ASSERT(!outEdges.empty());

        // if there is more than 1 outEdge
        if (outEdges.size() > 1) {
            // clear and re-use consecutiveLeaves variable
            consecutiveLeaves.clear(); 
            // make a reference variabled
            std::vector<PCN *> &addedLeaves = consecutiveLeaves;

            // if mergedLeaf if a P-node
            // PCTree_construction::insertLeaves
            if (mergedLeaf->getNodeType() == PCNT::PNode) {
                T.insertLeaves(outEdges.size(), mergedLeaf, &addedLeaves);
            // otherwise mergedLeaf is a Leaf node
            // PCTree_construction::replaceLeaf 
            } else {
                OGDF_ASSERT(mergedLeaf->getNodeType() == PCNT::Leaf);
                T.replaceLeaf(outEdges.size(), mergedLeaf, &addedLeaves);
            }

            // leaf representations are updated accordingly
            for (int i = 0; i < outEdges.size(); i++) {
                leafRepresentation[outEdges[i]->index()] = addedLeaves[i];
            }
        // if there is only 1 edge
        // first outEdge is updated to mergedLeaf node
        } else {
            leafRepresentation[outEdges.front()->index()] = mergedLeaf;
        }
    }

}



int main(int argc, char* argv[]) 
{

    string inputFile = argv[1];
    string selection = argv[2];
    string planarity = argv[3];
    Graph G;

    if (!GraphIO::read(G, inputFile, GraphIO::readGML)) {
        std::cerr << "Could not read input.gml" << std::endl;
        return 1;
    }

    int b;
    std::istringstream(planarity) >> b;

    if (b) {
        // create map from integer to node
        // find edge and change its attribute
        // implicit assumption that nodes retain order as given by input
        std::unordered_map<int, ogdf::node> nodeMap;
        // Add nodes to the graph and map them to integers
        int index = 0; 
        for (ogdf::node v : G.nodes) {
            nodeMap[index] = v;
            ++index;
        }

        // manually add our own random edges
        int numEdgesToAdd = b; // Number of edges to add
        int count = 0;
        std::srand(150); // Seed the random number generator
        int numNodes = G.numberOfNodes();

        while (count < numEdgesToAdd) {
            int sourceIndex = std::rand() % numNodes; // Random source node index
            int targetIndex = std::rand() % numNodes; // Random target node index

            if (sourceIndex == targetIndex) continue;
            ogdf::edge targetEdge = G.searchEdge(nodeMap[sourceIndex], nodeMap[targetIndex], false);
            if (targetEdge == nullptr) {
                G.newEdge(nodeMap[sourceIndex], nodeMap[targetIndex]);
                std::cout << sourceIndex << ", " << targetIndex << std::endl;
                ++count;
            }
        }
    }
    
    NodeArray<int> numbering(G);
    // int num = computeSTNumbering(G, numbering, 0);
    int num = computeSTNumbering(G, numbering, nullptr, nullptr, true);
    OGDF_ASSERT(num == G.numberOfNodes());

    // print after input
    // graphPrinter(G);
    std::cout << "G Planarity: " << ogdf::isPlanar(G) << std::endl;
    std::cout << "Original number of nodes: " << G.numberOfNodes() << std::endl;
    std::cout << "Original number of edges: " << G.numberOfEdges() << std::endl;

    // separator for planarization
    // <-------------->

    // PQ implementation to make planar subgraph
    std::cout << "start planarization" << std::endl;
    List<edge> *delEdges = new List<edge>; // empty list
    // changed to PC
    if (selection == "pc") {
        planarizePC<pc_tree::PCTree, pc_tree::PCNode, pc_tree::PCNodeType>(G, numbering, *delEdges);
    } else if (selection == "pq") {
        planarizePQ(G, numbering, *delEdges);
    // } else if (selection == "bm") {
    //     SubgraphPlanarizer *crossMin = new SubgraphPlanarizer;
    //     crossMin->call(G)
    } else if (selection == "fast") {
        // PlanarSubgraphModule<int> *ps = new PlanarSubgraphFast<int>;
        // ogdf::MaximalPlanarSubgraphSimple<int> mps(*(new PlanarSubgraphBoyerMyrvold));
        ogdf::MaximalPlanarSubgraphSimple<int> mps(*(new PlanarSubgraphFast<int>));
        mps.call(G, *delEdges);

    } else {
        std::cout << "running maximum" << std::endl;
        ogdf::MaximumPlanarSubgraph<int> mps;
        mps.call(G, *delEdges);
    }

    std::cout << "Edges removed:" << delEdges->size() << std::endl;


    // delete removed edges
    for (edge e: *delEdges) {
        // print removed edges
        // std::cout << e->adjSource() << std::endl;
        G.delEdge(e);
    }


    GraphIO::write(G, "output.gml", GraphIO::writeGML);
    

    // edgeListPrinter(*delEdges);
    // std::cout << std::endl;
    // std::cout << "subG planarity: " << ogdf::isPlanar(subgraph) << std::endl;
    std::cout << "G planarity: " << ogdf::isPlanar(G) << std::endl;
    std::cout << "Original number of nodes: " << G.numberOfNodes() << std::endl;
    std::cout << "Subgraph number of edges: " << G.numberOfEdges() << std::endl;
    // graphPrinter(G);
    // std::cout << isPlanarPC<pc_tree::hsu::PCTree, pc_tree::hsu::PCNode, pc_tree::hsu::PCTree::PCNodeType>(G, numbering) << std::endl;
    // bool result = isPlanarPC<pc_tree::PCTree, pc_tree::PCNode, pc_tree::PCNodeType>(subG, numbering);
    // std::cout << result << std::endl;


    return 0;
}
