#ifndef NET_H
#define NET_H

#include <vector>
using namespace std;

class Net
{
public:
    // constructor and destructor
    Net(int& id) :
        _id(id) {
        _partCount[0] = 2; _partCount[1] = 0;
    }
    ~Net()  { }

    // basic access methods
    int getID()           const { return _id; }
    int getPartCount(int part) const { return _partCount[part]; }
    vector<int> getCellList()  const { return _cellList; }

    // set functions
    void setID(const int id) { _id = id; }
    void setPartCount(int part, const int count) { _partCount[part] = count; }

    // modify methods
    void incPartCount(int part)     { _partCount[part]++; }
    void decPartCount(int part)     { _partCount[part]--; }
    void addCell(const int cellId)  { _cellList.push_back(cellId); }

private:
    int             _partCount[2];  // Cell number in partition A(0) and B(1)
    int          	_id;            // Name of the net
    vector<int>     _cellList;      // List of cells the net is connected to
};

#endif  // NET_H
