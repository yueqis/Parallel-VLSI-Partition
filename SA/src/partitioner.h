#ifndef PARTITIONER_H
#define PARTITIONER_H

#include <fstream>
#include <vector>
#include <map>
#include <numeric>
#include <string>
#include "cell.h"
#include "net.h"
using namespace std;

class Partitioner
{
public:
    // constructor and destructor
    Partitioner(fstream& inFile, int pid, int nproc, bool isDense) :
        _netNum(0), _cellNum(0), _maxPinNum(0), _rFactor(0.1), _bFactor(0.2), _cutSize(0), _iterations(100), _pid(pid), _nproc(nproc), _isfirst(true)
        {
		if(isDense){
			parseDenseInput(inFile);
		} else {
			parseUniformInput(inFile);
		}
        
        _partSize[0] = _cellNum;
        _partSize[1] = 0;
    }
    ~Partitioner() {
        clear();
    }

    // basic access methods
    int getNetNum() const           { return _netNum; }
    int getCellNum() const          { return _cellNum; }
    double getBFactor() const       { return _bFactor; }
    int getPartSize(int part) const { return _partSize[part]; }
	vector<int> getCluster() const  {return _cluster;}

    // modify method
    void parseUniformInput(fstream& inFile);
	void parseDenseInput(fstream& inFile);
    void partition();
	void initial_partition();
	void syncCells();
	void reshuffle(int bal_condtion);
	void addCluster(int c){
		_cluster.push_back(c);
	}
	void cluster();

    // member functions about reporting
    void printSummary() const;
    void reportNet() const;
    void reportCell() const;
    void writeResult(fstream& outFile);
	int balcondition();
	double find_bal();
	void reportCellGain() const;
	void reportCellPart() const;
	void reportBucket() const;
	void reportNetPart() const;
	void ensureCutsize();


private:
	int 				_pid;			// process id
	int					_nproc;			// number of processors
	int 				_iterations;    // number of SA iterations
    int                 _cutSize;       // cut size 
    int                 _partSize[2];   // size (cell number) of partition A(0) and B(1)
    int                 _netNum;        // number of nets
    int                 _cellNum;       // number of cells
    int                 _maxPinNum;     // Pmax for building bucket list
    double              _bFactor;       // the balance factor to be met
	double				_rFactor;		// chance to move cell randomly
    vector<Net*>        _netArray;      // net array of the circuit
    vector<Cell*>       _cellArray;     // cell array of the circuit
	int 				_upperBound;	// maximum number of cells in one partition
	int 				_lowerBound;	// minimum number of cells in one partition
	vector<int>			_sortedCells;	// sorted index of cells according to pin number
	vector<int>			_changedCells;  // the changedcells
	bool				_isfirst;		// first time to shuffle
	vector<int>			_cluster;		// its cluster of other threads
    // Clean up partitioner
    void clear();
};

#endif  // PARTITIONER_H
