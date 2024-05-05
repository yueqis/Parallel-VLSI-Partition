#ifndef CELL_H
#define CELL_H

#include <vector>
using namespace std;

class Cell
{
public:
    // Constructor and destructor
    Cell(int part, int id) :
        _pinNum(0), _part(part), _id(id), _isvisited(false) {
    }
    ~Cell() { }

    // Basic access methods
    int getPinNum() const   { return _pinNum; }
    int getPart() const    { return _part; }
    int getFirstNet() const { return _netList[0]; }
    vector<int> getNetList() const  { return _netList; }
	int getID() const		{return _id;}
	bool getvisited() const {return _isvisited;}
	int getthread() const	{return _threadID;}

    // Set functions
    void setPart(const int part)   { _part = part; }

    // Modify methods
    void move()         { _part = !_part; }
    void incPinNum()    { ++_pinNum; }
    void decPinNum()    { --_pinNum; }
    void addNet(const int netId) { _netList.push_back(netId); }
	void setvisited()   {_isvisited = true;}
	void setthread(int pid)   {_threadID = pid;}
    

private:
	int				_id;
    int             _pinNum;    // number of pins the cell are connected to
    int             _part;      // partition the cell belongs to (0-A, 1-B)
    vector<int>     _netList;   // list of nets the cell is connected to
	bool			_isvisited; // flag for BFS
	int				_threadID;  // thread for this cell
};

#endif  // CELL_H
