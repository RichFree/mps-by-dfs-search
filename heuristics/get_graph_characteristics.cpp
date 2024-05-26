/*  get number of nodes and edge of graph
 * 
 */
#include <ogdf/fileformats/GraphIO.h>
#include <filesystem>
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
    string filename = std::filesystem::path(input_file).stem();
    std::cout <<  filename << ", " << G.numberOfNodes() << ", " << G.numberOfEdges() << std::endl;
    

    return 0;
}
