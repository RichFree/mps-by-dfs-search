/*  Maximum Planar Subgraph by cactus 
 * 
 */

#include <ogdf/fileformats/GraphIO.h>
#include <ogdf/planarity/MaximumPlanarSubgraph.h>
#include <ogdf/planarity/PlanarSubgraphCactus.h>
#include <ogdf/planarity/MaximalPlanarSubgraphSimple.h>
#include <filesystem>
#include <chrono>
#include <iostream>

using namespace ogdf;


int main(int argc, char* argv[]) 
{

    string input_file = argv[1];
    Graph G;

    if (!GraphIO::read(G, input_file, GraphIO::readGML)) {
        std::cerr << "Could not read input.gml" << std::endl;
        return 1;
    }
        
    // --------------------------------
    // planarize 
    // --------------------------------

    List<edge> *delEdges = new List<edge>; // empty list

    auto start = std::chrono::high_resolution_clock::now();

    // compute a cactus, then compute edge addition
    PlanarSubgraphCactus<int> psc;
    ogdf::MaximalPlanarSubgraphSimple<int> mps(psc);
    mps.call(G, *delEdges);

    auto end = std::chrono::high_resolution_clock::now();
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    double time = static_cast<double>(microseconds) / 1'000'000.0;

    // --------------------------------
    // print out result of planarization
    // --------------------------------

    int removed_edges =  delEdges->size();
    string filename = std::filesystem::path(input_file).stem();
    std::cout << filename << ", " << removed_edges << ", " << time << std::endl;


    return 0;
}
