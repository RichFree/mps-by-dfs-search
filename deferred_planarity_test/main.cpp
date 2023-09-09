//-----------------------------------------------------------------------------------
// A simple code that test the MPS algorighm.
//-----------------------------------------------------------------------------------

#include <iostream>
#include <cstdlib>
#include <climits>
#include <string>
#include "mps.h"
using namespace std;

int find_mps(string input_file);


//-----------------------------------------------------------------------------------
// Main function.
//-----------------------------------------------------------------------------------


int main(int argc, char* argv[]) {
    string input_file = argv[1];
	std::cout << "Number of removed edges: " << find_mps(input_file) << std::endl;

	return 0;
}
