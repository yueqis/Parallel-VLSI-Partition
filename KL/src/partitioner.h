#ifndef PARTITIONER_H
#define PARTITIONER_H

#include <fstream>
#include <vector>
#include <map>
#include <numeric>
#include "cell.h"
#include "net.h"
using namespace std;

class Partitioner
{
public:
    // constructor and destructor
    Partitioner(fstream& inFile) :
        _netNum(0), _cellNum(0), _maxPinNum(0), _rFactor(0.1), _bFactor(0.2), _cutSize(0)
        {
        parseInput(inFile);
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

    // modify method
    void parseInput(fstream& inFile);
    void coarse();
    void KL();
    void uncoarse();
    void partition();

    
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
	
    // Clean up partitioner
    void clear();
};

#endif  // PARTITIONER_H
