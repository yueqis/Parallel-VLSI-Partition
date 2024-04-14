#ifndef CELL_H
#define CELL_H

#include <vector>
using namespace std;

class Cell
{
public:
    // Constructor and destructor
    Cell(bool part, int id) :
        _pinNum(0), _part(part), _id(id) {
    }
    ~Cell() { }

    // Basic access methods
    int getPinNum() const   { return _pinNum; }
    bool getPart() const    { return _part; }
    int getFirstNet() const { return _netList[0]; }
    vector<int> getNetList() const  { return _netList; }
	int getID() const		{return _id;}

    // Set functions
    void setPart(const bool part)   { _part = part; }

    // Modify methods
    void move()         { _part = !_part; }
    void incPinNum()    { ++_pinNum; }
    void decPinNum()    { --_pinNum; }
    void addNet(const int netId) { _netList.push_back(netId); }
    

private:
	int				_id;
    int             _pinNum;    // number of pins the cell are connected to
    bool            _part;      // partition the cell belongs to (0-A, 1-B)
    vector<int>     _netList;   // list of nets the cell is connected to
};

#endif  // CELL_H
