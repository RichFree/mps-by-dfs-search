/* This code only tests the graph for planarity
 * 
 */
#include <ogdf/fileformats/GraphIO.h>
#include <ogdf/basic/extended_graph_alg.h>

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

    std::cout << ogdf::isPlanar(G) << std::endl;
    

    return 0;
}
