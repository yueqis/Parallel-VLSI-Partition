#include <iostream>
#include <fstream>
#include <vector>
#include <mpi.h>
#include <chrono> 
#include "partitioner.h"

using namespace std;

string toString(char* a)
{
    string s(a);
    return s;
}
 

int main(int argc, char** argv)
{
    fstream input, output;
	const auto init_start = std::chrono::steady_clock::now();
    int pid;
    int nproc;

    // Initialize MPI
    MPI_Init(&argc, &argv);
    // Get process rank
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    // Get total number of processes specificed at start of run
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);
	
    if (argc == 3) {
        input.open(argv[1], ios::in);
        output.open(argv[2], ios::out);
        if (!input || !output) {
            cerr << "Cannot open the input/output file \"" 
                 << "\". The program will be terminated..." << endl;
            MPI_Finalize();
            exit(EXIT_FAILURE);
        }
    }
    else {
        cerr << "Usage: ./fm <input file> <output file>" << endl;
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
	
	bool isDense = (toString(argv[1]).find("dense") != string::npos)? true:false;
    Partitioner* partitioner = new Partitioner(input, pid, nproc, isDense);
	partitioner->initial_partition();
	auto start = chrono::high_resolution_clock::now();
    partitioner->partition();
	if (pid == 0) {
		auto stop = chrono::high_resolution_clock::now();
		auto duration = duration_cast<chrono::microseconds>(stop - start);
		cout << "CellNum = "<< partitioner->getCellNum() << " Time taken by " << nproc << " threads: "
         << duration.count()/1000000.0 << " seconds" << endl; 
		partitioner->printSummary();
		//partitioner->writeResult(output);
	}
    MPI_Finalize();
}
