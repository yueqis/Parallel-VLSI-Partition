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
#include <mpi.h>
#include <time.h>
using namespace std;

vector<vector<int>> adj_list;
vector<vector<int>> adj_list_coarse;
vector<tuple<int,int>> matching_edge;
vector<int> map_coarse;
vector<bool> part0;
vector<bool> part1;
vector<bool> part0_coarse;
vector<bool> part1_coarse;
int numCell;
int numEdge;
int numCell_coarse;
int numEdge_coarse;

void Partitioner::parseInput(fstream& inFile)
{
    string str;
    inFile >> str;
    numCell = stod(str);
	inFile >> str;
    numEdge = stod(str);

	// create a adjacency list with numCell-many vectors
	adj_list = vector<vector<int>> (numCell);
	for(int i = 0; i < numEdge; i++) {
		inFile >> str;
		int cellID1 = stoi(str);
		inFile >> str;
		int cellID2 = stoi(str);
		cellID1 --;
		cellID2 --;
		adj_list[cellID1].push_back(cellID2);
		adj_list[cellID2].push_back(cellID1);
	}

	// create an initial partition, put even nodes in partition 0 and put odd nodes in partition 1
	for (int i = 0; i < numCell; i++) {
		if (i % 2 == 0) {
			part0.push_back(true);
			part1.push_back(false);
		}
		else {
			part0.push_back(false);
			part1.push_back(true);
		}
	}
	cout << "finished parsing input!" << endl;
    return;
}

void Partitioner::coarse()
{
	vector<int> matched (numCell, 0);
	for (int i = 0; i < numCell; i++) {
		if (matched[i] == 0) {
			matched[i] = 1;
			for (int j = 0; j < adj_list[i].size(); j++) {
				int j_cellID = adj_list[i][j];
				if (matched[j_cellID] == 0) {
					matched[j_cellID] = 1;
					matching_edge.push_back(make_tuple(i, j_cellID));
					break;
				}
			}

		}
	}
	int matching_edge_size = matching_edge.size();
	numCell_coarse = numCell - matching_edge_size;
	adj_list_coarse = vector<vector<int>> (numCell_coarse);

	// mapping uncoarse to coarse
	map_coarse = vector<int> (numCell);
	for (int i = 0; i < matching_edge_size; i++) {
		int cellID1 = std::get<0>(matching_edge[i]);
		int cellID2 = std::get<1>(matching_edge[i]);
		map_coarse[cellID1] = i;
		map_coarse[cellID2] = i;
	}
	int idx = matching_edge_size;
	for (int i = 0; i < numCell; i++) {
		bool add = true;
		for (int j = 0; j < matching_edge_size; j++) {
			int cellID1 = get<0>(matching_edge[j]);
			int cellID2 = get<1>(matching_edge[j]);
			if (i == cellID1 || i == cellID2) {
				add = false;
				continue;}
		}
		if (add) {
			map_coarse[i] = idx;
			idx++;
		}
	}

	// add neighbors of grouped vertices to adj_list_coarse
	for (int i = 0; i < matching_edge_size; i++) {
		int cellID1 = std::get<0>(matching_edge[i]);
		int cellID2 = std::get<1>(matching_edge[i]);
		int cellID1_coarse = map_coarse[cellID1];
		int cellID2_coarse = map_coarse[cellID2];
		for (int j = 0; j < adj_list[cellID1].size(); j++) {
			int j_cellID_coarse = map_coarse[adj_list[cellID1][j]];
			bool add = true;
			if (j_cellID_coarse == cellID2_coarse) {add = false;} // don't add if its in this cell's group
			// else if ((j_cellID_coarse % _nproc) != _pid) {add = false;} // don't add if this pid is not in charge of this cell
			else {
				for (int k = 0; k < adj_list_coarse[i].size(); k++) {
					// don't add if already added previously
					if (j_cellID_coarse == adj_list_coarse[i][k]) {add = false; break;}
				}
			}
			if (add == true) {adj_list_coarse[i].push_back(j_cellID_coarse);}
		}
		for (int j = 0; j < adj_list[cellID2].size(); j++) {
			int j_cellID_coarse = map_coarse[adj_list[cellID2][j]];
			bool add = true;
			if (j_cellID_coarse == cellID2_coarse) {add = false;} // don't add if its in this cell's group
			// else if ((j_cellID_coarse % _nproc) != _pid) {add = false;} // don't add if this pid is not in charge of this cell
			else {
				for (int k = 0; k < adj_list_coarse[i].size(); k++) {
					// don't add if already added previously
					if (j_cellID_coarse == adj_list_coarse[i][k]) {add = false; break;}
				}
			}
			if (add == true) {adj_list_coarse[i].push_back(j_cellID_coarse);}
		}
	}

	// put vertices that are not matched to any other vertex in to adj_list_coarse
	for (int i = 0; i < numCell; i++) {
		// if ((map_coarse[i] % _nproc) == _pid && map_coarse[i] >= matching_edge_size) {
		if (map_coarse[i] >= matching_edge_size) {
			for (int j = 0; j < adj_list[i].size(); j++) {
				int j_coarse = map_coarse[adj_list[i][j]];
				bool add = true;
				// if ((j_coarse % _nproc) != _pid) {add = false;}
				if (false) {add = true;}
				else {
					for (int k = 0; k < adj_list_coarse[map_coarse[i]].size(); k++) {
						// don't add if already added previously
						if (j_coarse == adj_list_coarse[map_coarse[i]][k]) {add = false;}
					}
				}
				if (add == true) {
					adj_list_coarse[map_coarse[i]].push_back(j_coarse);
				}
			}
		}
	}
	// assign all even numbers to partition 0 and odd numbers to partition 1
	for (int i = 0; i < numCell_coarse; i++) {
		if (i % 2 == 0) {
			part0_coarse.push_back(true);
			part1_coarse.push_back(false);
		}
		else {
			part0_coarse.push_back(false);
			part1_coarse.push_back(true);
		}
	}

	// for (int i = 0; i < numCell; i++) {
	// 	cout << "i: " << i << "; group: " << map_coarse[i] << endl;
	// 	for (int j = 0; j < adj_list_coarse[map_coarse[i]].size(); j++) {
	// 		cout << "j: " << adj_list_coarse[map_coarse[i]][j] << endl;
	// 	}
	// }
	// for (int i = 0; i < numCell_coarse; i++) {
	// 	for (int j = 0; j < adj_list_coarse[i].size(); j++) {
	// 		cout << "i: " << i << "; j: " << adj_list_coarse[i][j] << endl;
	// 	}
	// }
	return;
}

// cite: https://patterns.eecs.berkeley.edu/?page_id=571#2_Kernighan-Lin_Algorithm
void Partitioner::KL()
{
	cout << "KL" << endl;
	map<int, bool> lock; // whether a vertex shouldn't be moved any more
	while (true) {
		vector<pair<int, pair<int, int>>> gain_swap; // gain of swapping a pair of vertices + swap a pair of vertices
		for (int i = _pid; i < numCell_coarse / 2 * _nproc; i+=_nproc) {
			// Want D[j] = E[j] - I[j] (external cost - internal cost)
			vector<int> D = vector<int> (numCell_coarse);
			// pick some random vertex v1
			for (int v1 = _pid; v1 < numCell_coarse; v1+=_nproc) {
				int Ev = 0;
				int Iv = 0;
				for (int j = 0; j < adj_list_coarse[v1].size(); j++) {
					int v2 = adj_list_coarse[v1][j];
					if (part0_coarse[v1] == part0_coarse[v2]) {
						// v1 and v2 in the same partition -> internal cost
						Iv += 1;
					}
					else {Ev += 1;}
				}
				D[v1] = Ev - Iv;
			}

			vector<pair<int, pair<int, int>>> g;
			for (int v1 = _pid; v1 < numCell_coarse; v1+=_nproc) {
				int cost = 0;
				// for (int v2 = _pid; v2 < numCell_coarse; v2+=_nproc) {
				for (int v2 = 0; v2 < numCell_coarse; v2+=1) {
					if (part0_coarse[v1] != part0_coarse[v2]) {
						for (int j = 0; j < adj_list_coarse[v1].size(); j++) {
							if (adj_list_coarse[v1][j] == v2) {cost = 1;}
						}
						int gain = D[v1] + D[v2] - cost;
						if ((v2 % _nproc) == _pid) {g.push_back({gain, {v1, v2}});}
					}
				}
			}
			sort(g.begin(), g.end(), greater<pair<int, pair<int, int>>> ());
			for (int i = 0; i < g.size(); i++) {
				pair<int, int> pr = g[i].second;
				if(!lock.count(pr.first) && !lock.count(pr.second)) {
					lock[pr.first];
					lock[pr.second];
					gain_swap.push_back({g[i].first, pr});
					break;
				}
			}
		}
		sort(gain_swap.begin(), gain_swap.end(), greater<pair<int, pair<int, int>>> ());
		if (gain_swap.size() <= 0 || gain_swap[0].first <= 0) {
			break;}
		part0_coarse[gain_swap[0].second.first] = !part0_coarse[gain_swap[0].second.first];
		part0_coarse[gain_swap[0].second.second] = !part0_coarse[gain_swap[0].second.second];
		part1_coarse[gain_swap[0].second.first] = !part1_coarse[gain_swap[0].second.first];
		part1_coarse[gain_swap[0].second.second] = !part1_coarse[gain_swap[0].second.second];
	}
	return;
}

void Partitioner::uncoarse() {
	cout << "uncoarse" << endl;
	int* message = new int[numCell_coarse];
	for (int i = 0; i < numCell_coarse; i++) {
		message[i] = part0_coarse[i];
	}
	if (_pid != 0) {
		// send the partitions to pid 0
		MPI_Request request_send;
		MPI_Isend(message, numCell_coarse, MPI_INT, 0, 1, MPI_COMM_WORLD, &request_send);
		MPI_Wait(&request_send, MPI_STATUS_IGNORE);
	}
	if (_pid == 0) {
		// obtain the partitions from other threads
		for (int from_pid = 1; from_pid < _nproc; from_pid++) {
			MPI_Request request_receive;
			int* received_message = new int[numCell_coarse];
			MPI_Irecv(received_message, numCell_coarse, MPI_INT, from_pid, MPI_ANY_TAG, MPI_COMM_WORLD, &request_receive);
			MPI_Wait(&request_receive, MPI_STATUS_IGNORE);
			if (request_receive == MPI_SUCCESS) {
				for (int j = from_pid; j < numCell_coarse; j+=_nproc) {
					if (received_message[j] == 0) {
						part0_coarse[j] = false;
						part1_coarse[j] = true;
					}
					else {
						part0_coarse[j] = true;
						part1_coarse[j] = false;
					}
				}
			}
		}
		// uncoarse
		for (int i = 0; i < part0.size(); i++) {
			int coarse_idx = map_coarse[i];
			part0[i] = part0_coarse[coarse_idx];
			part1[i] = part1_coarse[coarse_idx];
		}
		// for (int i = 0; i < part0_coarse.size(); i++) {
		// 	cout << i << ": " << part0_coarse[i] << endl;
		// }
		cout << "partition A: ";
		for (int i = 0; i < part0.size(); i++) {
			if (part0[i] == true) {
				cout << i << " ";
			}
		}
		cout << endl;
	}
	return;
}

// cite: https://people.csail.mit.edu/jshun/6886-s18/lectures/lecture13-1.pdf
void Partitioner::partition()
{
	coarse();
	KL();
	uncoarse();
	return;
}

void Partitioner::printSummary() const
{
	int cutSize = 0;
	for (int i = 0; i < numCell; i++) {
		for (int j = 0; j < adj_list[i].size(); j++) {
			int j_cell = adj_list[i][j];
			if (part0[i] != part0[j_cell]) {
				cutSize += 1;
			}
		}
	}
	cutSize = cutSize / 2;
	int count0 = 0;
	for (int i = 0; i < numCell; i++) {
		if (part0[i] == true) {count0++;}
	}
	int count1 = numCell - count0;
    cout << endl;
    cout << "==================== Summary ====================" << endl;
    cout << " Cutsize: " << cutSize << endl;
    cout << " Total cell number: " << numCell << endl;
    cout << " Total net number:  " << numEdge << endl;
    cout << " Cell Number of partition A: " << count0 << endl;
    cout << " Cell Number of partition B: " << count1 << endl;
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
