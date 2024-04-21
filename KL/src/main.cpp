#include <iostream>
#include <fstream>
#include <vector>
#include "partitioner.h"
using namespace std;

int main(int argc, char** argv)
{
    fstream input, output;

    if (argc == 3) {
        input.open(argv[1], ios::in);
        output.open(argv[2], ios::out);
        if (!input || !output) {
            cerr << "Cannot open the input/output file \"" 
                 << "\". The program will be terminated..." << endl;
            exit(1);
        }
    }
    else {
        cerr << "Usage: ./fm <input file> <output file>" << endl;
        exit(1);
    }

    Partitioner* partitioner = new Partitioner(input);
    partitioner->partition();
    partitioner->printSummary();
    partitioner->writeResult(output);
    return 0;
}
