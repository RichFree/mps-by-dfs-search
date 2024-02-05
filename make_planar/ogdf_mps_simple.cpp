#include <ogdf/basic/STNumbering.h>
#include <ogdf/fileformats/GraphIO.h>

#include <ogdf/planarity/PlanarSubgraphFast.h>
// #include <ogdf/planarity/PlanarSubgraphBoyerMyrvold.h>
#include <ogdf/planarity/MaximumPlanarSubgraph.h>
#include <ogdf/planarity/MaximalPlanarSubgraphSimple.h>

#include <iostream>

using namespace ogdf;


int main(int argc, char* argv[]) 
{

    string inputFile = argv[1];
    Graph G;

    if (!GraphIO::read(G, inputFile, GraphIO::readGML)) {
        std::cerr << "Could not read input.gml" << std::endl;
        return 1;
    }

        
    NodeArray<int> numbering(G);
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

    ogdf::MaximalPlanarSubgraphSimple<int> mps(*(new PlanarSubgraphFast<int>));
    mps.call(G, *delEdges);

    
    std::cout << "Edges removed:" << delEdges->size() << std::endl;


    // delete removed edges
    for (edge e: *delEdges) {
        // print removed edges
        // std::cout << e->adjSource() << std::endl;
        G.delEdge(e);
    }


    // write processed graph to new gml file
    // GraphIO::write(G, "output.gml", GraphIO::writeGML);
    

    std::cout << "G planarity: " << ogdf::isPlanar(G) << std::endl;
    std::cout << "Original number of nodes: " << G.numberOfNodes() << std::endl;
    std::cout << "Subgraph number of edges: " << G.numberOfEdges() << std::endl;


    return 0;
}
