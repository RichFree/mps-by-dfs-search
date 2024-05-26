/* This code converts stp to gml
 * 
 */
#include <ogdf/fileformats/GraphIO.h>

#include <iostream>

using namespace ogdf;


int main(int argc, char* argv[]) 
{

    string input_file = argv[1];
    Graph G;

    if (!GraphIO::read(G, input_file, GraphIO::readSTP)) {
        std::cerr << "Could not read input.stp" << std::endl;
        return 1;
    }

    string output_file = argv[2];
    GraphIO::write(G, output_file, GraphIO::writeGML);
    

    return 0;
}
