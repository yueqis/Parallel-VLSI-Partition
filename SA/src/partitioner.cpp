#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cassert>
#include <vector>
#include <cmath>
#include <map>
#include <algorithm>
#include <random>
#include <numeric> 
#include "cell.h"
#include "net.h"
#include "partitioner.h"
using namespace std;


void Partitioner::parseInput(fstream& inFile)
{
    string str;
    inFile >> str;
    _cellNum = stod(str);
	inFile >> str;
    _netNum = stod(str);
	for (int i = 0; i<_cellNum; i++){
		_cellArray.push_back(new Cell(0, i+1)); 
	}
	for (int i = 0; i<_netNum; i++){
		_netArray.push_back(new Net(i)); 
	}
	string cell1, cell2;
	for(int i = 0; i<_netNum; i++) {
		inFile >> cell1;
		int cellID1 = stoi(cell1);
		inFile >> cell2;
		int cellID2 = stoi(cell2);
		cellID1 --;
		cellID2 --;
		_cellArray[cellID1]->addNet(i);
		_cellArray[cellID1]->incPinNum();
		if(_cellArray[cellID1]->getPinNum()>_maxPinNum){
			_maxPinNum=_cellArray[cellID1]->getPinNum();
		}
		_cellArray[cellID2]->addNet(i);
		_cellArray[cellID2]->incPinNum();
		if(_cellArray[cellID2]->getPinNum()>_maxPinNum){
			_maxPinNum=_cellArray[cellID2]->getPinNum();
		}
		_netArray[i]->addCell(cellID1);
		_netArray[i]->addCell(cellID2);
	}
    return;
}

int Partitioner::balcondition()
{
	if((getPartSize(0)-1)<_lowerBound)return 1;    //search only 1
	else if((getPartSize(1)-1)<_lowerBound)return 0; //search only 0
	else return 2; //both sides are okay
}

double Partitioner::find_bal()
{
	double factor = getPartSize(0)*1.0/(getPartSize(0)+getPartSize(1));
	return abs(factor-0.5);
}

void Partitioner::partition()
{	
	_lowerBound =  (int) ceil((1-_bFactor)/2.0 * (getCellNum()));
	_upperBound =  (int)(1+_bFactor)/2.0 * (getCellNum());
	
	vector<int> randomCells(_cellNum);
	std::iota (begin(randomCells), end(randomCells), 0); 
	random_shuffle( randomCells.begin(), randomCells.end() );
    for (int i = 0; i<_cellNum/2; i++){
		int cellID = randomCells[i];
		_cellArray[cellID]->setPart(1);
		vector<int> nets = _cellArray[cellID]->getNetList();
		for (int j = 0; j < nets.size(); j++){
			_netArray[nets[j]]->incPartCount(1);
			_netArray[nets[j]]->decPartCount(0);
		}
		_partSize[0]--;
		_partSize[1]++;
	}
	for (int i = 0; i<_netNum; i++){
		if(_netArray[i]->getPartCount(0) == 1){
			_cutSize++;
		}
	}
	
}

void Partitioner::printSummary() const
{
    cout << endl;
    cout << "==================== Summary ====================" << endl;
    cout << " Cutsize: " << _cutSize << endl;
    cout << " Total cell number: " << _cellNum << endl;
    cout << " Total net number:  " << _netNum << endl;
    cout << " Cell Number of partition A: " << _partSize[0] << endl;
    cout << " Cell Number of partition B: " << _partSize[1] << endl;
    cout << "=================================================" << endl;
    cout << endl;
    return;
}

void Partitioner::reportNet() const
{
    cout << "Number of nets: " << _netNum << endl;
    for (size_t i = 0, end_i = _netArray.size(); i < end_i; ++i) {
        cout << setw(8) << _netArray[i]->getID() << ": ";
        vector<int> cellList = _netArray[i]->getCellList();
        for (size_t j = 0, end_j = cellList.size(); j < end_j; ++j) {
            cout << setw(8) << _cellArray[cellList[j]]->getID() << " ";
        }
        cout << endl;
    }
    return;
}

void Partitioner::reportCell() const
{
    cout << "Number of cells: " << _cellNum << endl;
    for (size_t i = 0, end_i = _cellArray.size(); i < end_i; ++i) {
        cout << setw(8) << _cellArray[i]->getID() << ": ";
        vector<int> netList = _cellArray[i]->getNetList();
        for (size_t j = 0, end_j = netList.size(); j < end_j; ++j) {
            cout << setw(8) << _netArray[netList[j]]->getID() << " ";
        }
        cout << endl;
    }
    return;
}

void Partitioner::reportCellPart() const
{
    cout << "Cell Part: " << endl;
    for (size_t i = 0, end_i = _cellArray.size(); i < end_i; ++i) {
        cout << setw(8) << _cellArray[i]->getID() << ": ";
        cout << _cellArray[i]->getPart();
        cout << endl;
    }
    return;
}

void Partitioner::reportNetPart() const
{
    cout << "NetPart: " << endl;
    for (size_t i = 0, end_i = _netArray.size(); i < end_i; ++i) {
        cout << setw(8) << _netArray[i]->getID() << "--> ";
        cout << setw(4) <<"A: "<< _netArray[i]->getPartCount(0) << "; "<<"B: "<< _netArray[i]->getPartCount(1) << endl;
    }
    return;
}

void Partitioner::writeResult(fstream& outFile)
{
    stringstream buff;
    buff << _cutSize;
    outFile << "Cutsize = " << buff.str() << '\n';
    buff.str("");
    buff << _partSize[0];
    outFile << "G1 " << buff.str() << '\n';
    for (size_t i = 0, end = _cellArray.size(); i < end; ++i) {
        if (_cellArray[i]->getPart() == 0) {
            outFile << _cellArray[i]->getID() << " ";
        }
    }
    outFile << ";\n";
    buff.str("");
    buff << _partSize[1];
    outFile << "G2 " << buff.str() << '\n';
    for (size_t i = 0, end = _cellArray.size(); i < end; ++i) {
        if (_cellArray[i]->getPart() == 1) {
            outFile << _cellArray[i]->getID() << " ";
        }
    }
    outFile << ";\n";
    return;
}

void Partitioner::clear()
{
    for (size_t i = 0, end = _cellArray.size(); i < end; ++i) {
        delete _cellArray[i];
    }
    for (size_t i = 0, end = _netArray.size(); i < end; ++i) {
        delete _netArray[i];
    }
    return;
}
