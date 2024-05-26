/*  Maximum Planar Subgraph by Integer Linear Program (ILP)
 * 
 */

#include <ogdf/fileformats/GraphIO.h>
#include <ogdf/planarity/MaximumPlanarSubgraph.h>
#include <filesystem>
#include <iostream>
#include <chrono>

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

    // store the deleted edges to be used to delete edges from G
    List<edge> *delEdges = new List<edge>; 


    auto start = std::chrono::high_resolution_clock::now();

    // perform planarization by ILP
    ogdf::MaximumPlanarSubgraph<int> mps;
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
