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
#include <queue>
#include "cell.h"
#include "net.h"
#include "partitioner.h"
#include <mpi.h>
#include <time.h>
#define MAXLIMIT 500000
using namespace std;


void Partitioner::parseUniformInput(fstream& inFile)
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
		_netArray[i]->incPartCount(0);
		_netArray[i]->incPartCount(0);
	}
	_lowerBound =  (int) ceil((1-_bFactor)/2.0 * (getCellNum()));
	_upperBound =  (int)(1+_bFactor)/2.0 * (getCellNum());
	vector<int> sorted_cells(_cellNum);
    for (int idx = 0; idx < _cellNum; idx++) {
        sorted_cells[idx] = idx;
    }
    auto compare_pin = [&](int idx1, int idx2) {
        return _cellArray[idx1]->getPinNum() > _cellArray[idx2]->getPinNum();
    };
    sort(sorted_cells.begin(), sorted_cells.end(), compare_pin);
	_sortedCells = sorted_cells;
    return;
}

void Partitioner::parseDenseInput(fstream& inFile)
{  
    map<string, int>    _cellName2Id;   
	string str;
	inFile >> str;
	while (inFile >> str && _netNum < MAXLIMIT) {
        if (str == "NET") {
            string netName, cellName, tmpCellName = "";
            inFile >> netName;
			vector<string> cells;
            while (inFile >> cellName) {
                if (cellName == ";") {
                    break;
                }
                else {
					cells.push_back(cellName);
                    
                }
            }
            for(int i = 0; i< cells.size(); i++){
				// a newly seen cell
				string cellName = cells[i];
				if (_cellName2Id.count(cellName) == 0) {
					int cellId = _cellNum;
					int t_part;
					if(cellId%2 == 0){		
						t_part = 0;
					} 
					else{
						_partSize[1] ++;
						_partSize[0] --;
						t_part = 1;
					}                                  
					_cellArray.push_back(new Cell(t_part, cellId));
					_cellName2Id[cellName] = cellId;
					
					++_cellNum;
				}
				for(int j = 0; j<i; j++) {
					int cell1 = _cellName2Id[cells[i]];
					int cell2 = _cellName2Id[cells[j]];
					_netArray.push_back(new Net(_netNum));
					_cellArray[cell1]->addNet(_netNum);
					_cellArray[cell1]->incPinNum();
					_netArray[_netNum]->addCell(cell1);
					_netArray[_netNum]->incPartCount(_cellArray[cell1]->getPart());
					_cellArray[cell2]->addNet(_netNum);
					_cellArray[cell2]->incPinNum();
					_netArray[_netNum]->addCell(cell2);
					_netArray[_netNum]->incPartCount(_cellArray[cell2]->getPart());
					_netNum++;
					if(_cellArray[cell1]->getPinNum()>_maxPinNum){
						_maxPinNum=_cellArray[cell1]->getPinNum();
					}
					if(_cellArray[cell2]->getPinNum()>_maxPinNum){
						_maxPinNum=_cellArray[cell2]->getPinNum();
					}
				}
			}
        }
    }
	_lowerBound =  (int) ceil((1-_bFactor)/2.0 * (getCellNum()));
	_upperBound =  (int)(1+_bFactor)/2.0 * (getCellNum());
	
	vector<int> sorted_cells(_cellNum);
    for (int idx = 0; idx < _cellNum; idx++) {
        sorted_cells[idx] = idx;
    }
    auto compare_pin = [&](int idx1, int idx2) {
        return _cellArray[idx1]->getPinNum() > _cellArray[idx2]->getPinNum();
    };
    sort(sorted_cells.begin(), sorted_cells.end(), compare_pin);
	_sortedCells = sorted_cells;
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

void Partitioner::initial_partition(){
	vector<int> randomCells(_cellNum);
	std::iota (begin(randomCells), end(randomCells), 0); 
	random_shuffle( randomCells.begin(), randomCells.end() );
    for (int i = 0; i<_cellNum/2; i++){
		int cellID = randomCells[(i*_nproc+1)%_cellNum];
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

void Partitioner::partition()
{	
	// SA
	for(int it = 0; it < _iterations; it++){
		for(int i = _pid+_nproc; i<_cellNum; i+=_nproc){
			// check whether moving the cell to the other side can improve the performance
			int cellID = _sortedCells[i];
			vector<int> nets = _cellArray[cellID]->getNetList();
			int cellPart = _cellArray[cellID]->getPart();
			int cutChange = 0;
			for (int j = 0; j < nets.size(); j++){
				int netID = nets[j];
				if(_netArray[netID]->getPartCount(cellPart) == 1){
					cutChange--;
				}
				else {
					cutChange++;
				}
			}
			// decide to move the cell to the other side
			
			if((cutChange < 0 || rand()<_rFactor) && balcondition() == 2){
				int newPart = !cellPart;
				_cellArray[cellID]->move();
				for (int j = 0; j < nets.size(); j++){
					int netID = nets[j];
					_netArray[netID]->incPartCount(newPart);
					_netArray[netID]->decPartCount(cellPart);
				}
				_partSize[newPart]++;
				_partSize[cellPart]--;
				_changedCells.push_back(cellID);
			}
		}
		syncCells();
		_changedCells.clear();
		if(balcondition() != 2 && it != _iterations-1 && rand()<1.0/_nproc){
			reshuffle(balcondition());
		}
		random_shuffle(_sortedCells.begin(), _sortedCells.end() );
	}
	
}

void Partitioner::cluster()
{	
	queue<int> q;
	vector <int> visited;
	int current_thread = 0;
	int count = 0;
    
	for (int i = 0; i<_cellNum; i++) {
		if (_cellArray[i]->getvisited() == false) {
			_cellArray[i]->setvisited();
			q.push(i);
			while(!q.empty()) {
				int cellID = q.front();
				_cellArray[cellID]-> setthread(current_thread);
				if(count > _cellNum/_nproc*1.0) current_thread++;
				q.pop();
				vector<int> nets = _cellArray[i]->getNetList();
				for (int j = 0; j < nets.size(); j++){
					int netID = nets[j];
					vector <int> cells = _netArray[netID]->getCellList();
					for (int k = 0; k<2; k++){
						if (cells[k] != cellID && _cellArray[i]->getvisited() == false) {
							q.push(cells[k]);
							break;
						}
					}
				}
			}
		}
		
	}
	for(int i = 0; i<=current_thread; i++){
		addCluster(i);
	}
	
}


void Partitioner::reshuffle(int bal_condition){
	int count = 0.2*_cellNum / _nproc;
	int cellPart = bal_condition;
	for(int i = 0; i<_cellNum; i++) {
		int cellID = _sortedCells[(i*_nproc)%_cellNum];
		if (_cellArray[cellID]->getPart() == cellPart){
			if(count < 0) break;
			vector<int> nets = _cellArray[cellID]->getNetList();
			int cutChange = 0;
			for (int j = 0; j < nets.size(); j++){
				int netID = nets[j];
				if(_netArray[netID]->getPartCount(cellPart) == 1){
					cutChange--;
				}
				else {
					cutChange++;
				}
			}
			if (cutChange > -2-_nproc) {
				int newPart = !cellPart;
				_cellArray[cellID]->move();
				for (int j = 0; j < nets.size(); j++){
					int netID = nets[j];
					_netArray[netID]->incPartCount(newPart);
					_netArray[netID]->decPartCount(cellPart);
				}
				_partSize[newPart]++;
				_partSize[cellPart]--;
				count -= 1;
			}
		}
	}
	
}

void Partitioner::syncCells(){
	int self_changed_num = _changedCells.size();
	int* message = new int[2*self_changed_num + 1];
	message[0] = self_changed_num;
	for (int i = 0; i < self_changed_num; i++) {
		message[i*2+1] = _changedCells[i];
		message[i*2+2] = _cellArray[_changedCells[i]]->getPart();
	}
	vector <int> mycluster = getCluster();
	for (int i = 0; i < _nproc; i++) {
		if (i != _pid || find(mycluster.begin(), mycluster.end(), i) != mycluster.end()) {
			MPI_Request request_send;
			MPI_Isend(message, 2*self_changed_num + 1, MPI_INT, i, 1, MPI_COMM_WORLD, &request_send);
			// fprintf(stderr, "pid %d before send %d\n", pid, message[0]);
			MPI_Request request_receive;
			int* received_message = new int[2*(_cellNum/_nproc) + 1];
			MPI_Irecv(received_message, 2*(_cellNum/_nproc) + 1, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &request_receive);
			MPI_Wait(&request_send, MPI_STATUS_IGNORE);
			// fprintf(stderr, "pid %d after wait send\n", pid);
			MPI_Wait(&request_receive, MPI_STATUS_IGNORE);
			// fprintf(stderr, "pid %d after wait receive\n", pid);
			//fprintf(stderr, " pid %d reads num_modified_cells = %d\n", _pid, received_message[0]);
			int other_changed_num = 0;
			if (request_receive == MPI_SUCCESS) {
				other_changed_num = received_message[0];
				for (int j = 0; j<other_changed_num; j++){
						int cellID = received_message[2*j+1];
						int cellPart = received_message[2*j+2];
						_cellArray[cellID]->setPart(cellPart);
						vector<int> nets = _cellArray[cellID]->getNetList();
						for (int k = 0; k < nets.size(); k++){
							_netArray[nets[k]]->incPartCount(1);
							_netArray[nets[k]]->decPartCount(0);
						}
						_partSize[!cellPart]--;
						_partSize[cellPart]++;
				}
				_isfirst = false;
			}
			if(_pid == 0){
				//cout << i <<" "<< other_changed_num<<endl;
			}
			delete[] received_message;
		}
	}
	delete[] message;
}

void Partitioner::printSummary() const
{
	int cutSize = 0;
	for (int netID = 0; netID<_netNum; netID++){
		vector<int> cells = _netArray[netID]->getCellList();
		if (_cellArray[cells[0]]->getPart() !=_cellArray[cells[1]]->getPart()){cutSize++;}
	}
	int count = 0;
	for (int cellID = 0; cellID<_cellNum; cellID++){
		int myPart = _cellArray[cellID]->getPart();
		if (myPart == 0) count++;
	}
    cout << endl;
    cout << "==================== Summary ====================" << endl;
    cout << " Cutsize: " << cutSize << endl;
    cout << " Total cell number: " << _cellNum << endl;
    cout << " Total net number:  " << _netNum << endl;
    cout << " Cell Number of partition A: " << count << endl;
    cout << " Cell Number of partition B: " << _cellNum-count << endl;
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
	_cutSize = 0;
	for (int i = 0; i<_netNum; i++){
		if(_netArray[i]->getPartCount(0) == 1){
			_cutSize++;
		}
	}
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
