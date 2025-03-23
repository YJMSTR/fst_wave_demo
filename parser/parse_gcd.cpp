#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "include/Parser.h"
#include "include/PNode.h"
#include "syntax.hpp"
#include "firParser.cpp"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <fir_file>" << std::endl;
        return 1;
    }

    // Read FIR file
    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << argv[1] << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    // Parse FIR file
    PNode* root = parseFIR((char*)content.c_str());
    
    if (root) {
        std::cout << "Successfully parsed FIR file!" << std::endl;
        // Additional processing can be added here, such as printing the parse tree
    } else {
        std::cerr << "Failed to parse FIR file" << std::endl;
    }

    return 0;
} 