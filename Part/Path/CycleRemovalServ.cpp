#include "CycleRemovalServ.hpp"
#include <GBPQueue.hpp>
#include <GDList.hpp>
#include <Netlist.hpp>
#include <string.h>
#include <limits.h>
#include <iostream>
#include <climits>  //added by Zhou
#include <cassert>  //added by Zhou

using std::vector;

/** 
 * Choose a sink c, remove it from the sink queue, and prepend it to
 * S_r. Note: isolated vertices are removed from the sink and
 * prepended to S_r at this stage.  
 */
inline void CycleRemovalServ::handleSink()
{
  Vertex& vertex = _sinkQueue.popFront();
  _Sr.pushFront(vertex);
  _numHandledCells++;
  //xxx Cell& c = *(Cell*) vertex.getInfo();
  const unsigned int i_v = &vertex - _vertexArray;
  Cell& c = _H.getCell(i_v);
  c.setVisited();

  NetCursor nc(c);
  while (nc.hasNext()) {
    Net& net = nc.getNext();
    handleNetForSink(net);
  }
}


/** 
 * Choose a source c, remove it from the source queue, and append it to
 * S_l.
 */
inline void CycleRemovalServ::handleSrc()
{
  Vertex& vertex = _srcQueue.popFront();
  _Sl.pushBack(vertex);
  _numHandledCells++;
  //xxx Cell& c = *(Cell*) vertex.getInfo();
  const unsigned int i_v = &vertex - _vertexArray;
  Cell& c = _H.getCell(i_v);
  c.setVisited();

  NetCursor nc(c);
  while (nc.hasNext()) {
    Net& net = nc.getNext();
    handleNetForSrc(net);
  }
}



/**
 * Choose a vertex v such that the difference outDeg(v) - inDeg(v) is
 * maximum, remove it from the queue, and append it to S_l.
 */
inline void CycleRemovalServ::handleNextBest()
{
  Vertex& v = _bucketPQ->popFront();
  _Sl.pushBack(v);
  _numHandledCells++;
  //xxx Cell& c = *(Cell*) v.getInfo();
  const unsigned int i_v = &v - _vertexArray;
  Cell& c = _H.getCell(i_v);
  c.setVisited();

  PinForCellCursor pc(c);
  while (pc.hasNext()) {
    Pin& pin = pc.getNext();
    Net& net = pin.getNet();
    switch (pin.getDirection()) {
    case Pin::OUTPUT: 
      handleNetForSrc(net);
      break;
    case Pin::INPUT: 
      handleNetForSink(net);
      break;  
    default: break;
    }
  }
}



inline void CycleRemovalServ::handleNetForSrc(Net& net)
{
  unsigned int netIndex = _H.getId(net);
  const Cell* const c0 = &_H.getCell(0);
  _inDegreeNet[netIndex]--;
  if (_inDegreeNet[netIndex] == 0) {
    CellCursor cc(net);
    while (cc.hasNext()) {
      Cell& c = cc.getNext();
      if (!c.isVisited()) {
	int cid = &c - c0;
	_inDegreeCell[cid]--;
	if (_inDegreeCell[cid] == 0) {
	  _bucketPQ->detach(_vertexArray[cid]);
	  _srcQueue.pushBack(_vertexArray[cid]);
	}
	else {
	  _bucketPQ->increaseKeyBy(_vertexArray[cid], 1);
	} 
      }
    }
  }
}


inline void CycleRemovalServ::handleNetForSink(Net& net)
{
  unsigned int netIndex = _H.getId(net);
  const Cell* const c0 = &_H.getCell(0);
  _outDegreeNet[netIndex]--;
  if (_outDegreeNet[netIndex] == 0) {
    CellCursor cc(net);
    while (cc.hasNext()) {
      Cell& c = cc.getNext();
      if (!c.isVisited()) { // really need to check?
	int cid = &c - c0;
	_outDegreeCell[cid]--;
	if (_outDegreeCell[cid] == 0) {
	  _bucketPQ->detach(_vertexArray[cid]);
	  _sinkQueue.pushFront(_vertexArray[cid]);
	}
	else {
	  _bucketPQ->decreaseKeyBy(_vertexArray[cid],1);
	} 
      }
    }
  }
}


// Assign pin direction for first or last cell of a net
inline void CycleRemovalServ::assignFirstOrLastCell()
{
  //xxx const int n = _H.getNumCells();
  GDListCursor<Vertex> lc(_Sl);

  //xxx for (int i=0; i<n; i++) {
  //xxx   Cell* c = _H.getCell(i);
  //xxx   c->clrVisited();
  //xxx }
  _H.clrVisitedAllCells();

  while (lc.hasNext()) {
    const Vertex& vertex = lc.getNext();
    //xxx Cell& cellFrom = *(Cell*) vertex.getInfo();
    const unsigned int i_v = &vertex - _vertexArray;
    Cell& cellFrom = _H.getCell(i_v);
    cellFrom.setVisited();
    PinForCellCursor pc(cellFrom);
    while (pc.hasNext()) {
      Pin& pinFrom = pc.getNext();
      Net& net = pinFrom.getNet();
      PinForNetCursor pnc(net);
      bool lastOne = true;
      bool firstOne = true;
      while (pnc.hasNext()) {
        Pin& pinTo = pnc.getNext();
        Cell& cellTo = pinTo.getCell();
        if (!cellTo.isVisited()) {
          lastOne = false;
        } else {
          if (&cellTo != &cellFrom) {
            firstOne = false;
          }
        }
      }
      if (lastOne) {
        pinFrom.setDirection(Pin::INPUT);
      }
      else if (firstOne) {
        pinFrom.setDirection(Pin::OUTPUT);
      }
    }
  }
}


// Assign pin direction for other cells of a net
inline void CycleRemovalServ::assignOtherCells()
{
  //xxx const int n = _H.getNumCells();
  GDListCursor<Vertex> lc(_Sl);

  //xxx for (int i=0; i<n; i++) {
  //xxx   Cell* c = _H.getCell(i);
  //xxx   c->clrVisited();
  //xxx }
  _H.clrVisitedAllCells();

  while (lc.hasNext()) {
    const Vertex& vertex = lc.getNext();
    //xxx Cell& cellFrom = *(Cell*) vertex.getInfo();
    const unsigned int i_v = &vertex - _vertexArray;
    Cell& cellFrom = _H.getCell(i_v);
    cellFrom.setVisited();
    PinForCellCursor pc(cellFrom);
    while (pc.hasNext()) {
      Pin& pinFrom = pc.getNext();
      Net& net = pinFrom.getNet();
      PinForNetCursor pnc(net);
      while (pnc.hasNext()) {
        Pin& pinTo = pnc.getNext();
        Cell& cellTo = pinTo.getCell();
        if (&cellTo == &cellFrom) continue;
        if (!cellTo.isVisited()) {
          if (pinTo.getDirection() == Pin::OUTPUT) {
            pinFrom.setDirection(Pin::OUTPUT);
          }
        } else {
          if (pinTo.getDirection() == Pin::INPUT) {
            pinFrom.setDirection(Pin::INPUT);
          }
        }
      }
      if (pinFrom.getDirection() != Pin::INPUT 
	  && pinFrom.getDirection() != Pin::OUTPUT) {
        // if stil can not be decided, then make it OUTPUT
        pinFrom.setDirection(Pin::OUTPUT);
      }
    }
  }
}


CycleRemovalServ::CycleRemovalServ(Netlist& H) 
  : _H(H)
{
  const int n = _H.getNumCells();
  const int m = _H.getNumNets();

  // Initialization
  _inDegreeCell.reserve(n);
  _outDegreeCell.reserve(n);
  _inDegreeNet.reserve(m);
  _outDegreeNet.reserve(m);
  _bucketPQ = new GBPQueue<GNode>(-H.getMaxDegree(), H.getMaxDegree());
  _vertexArray = _H.getVertexArray();
}


CycleRemovalServ::~CycleRemovalServ()
{
  delete _bucketPQ;
}


void CycleRemovalServ::doRemove()
{
  const int n = _H.getNumCells();

  setup();

  while (_numHandledCells < n) {
    while (!_sinkQueue.isEmpty()) handleSink();
    while (!_srcQueue.isEmpty()) handleSrc();
    if (!_bucketPQ->isEmpty()) handleNextBest();
  }
  
  // Concatenate S_l with S_r
  _Sl.concat(_Sr);

  assignFirstOrLastCell();
  assignOtherCells();

  assert(verifyOK());
}


bool CycleRemovalServ::verifyOK()
{
  const int n = _H.getNumCells();

  setup();

  while (_numHandledCells < n) {
    int last = _numHandledCells;
    while (!_srcQueue.isEmpty()) handleSrc();
    while (!_sinkQueue.isEmpty()) handleSink();
    if (last == _numHandledCells) return false;
  }
  _Sl.concat(_Sr);
  assert(_Sl.isValid());  
  return true;
}


void CycleRemovalServ::setup()
{
  const int n = _H.getNumCells();
  const int m = _H.getNumNets();

  _inDegreeCell.assign(n, 0);
  _outDegreeCell.assign(n, 0);
  _inDegreeNet.assign(m, 0);
  _outDegreeNet.assign(m, 0);

  _numHandledCells = 0;

  int i;

  for (i=0; i<n; i++) {
    Cell& c = _H.getCell(i);
    c.clrVisited();
    PinForCellCursor pc(c);
    while (pc.hasNext()) {
      Pin& aPin = pc.getNext();
      switch (aPin.getDirection()) {
      case Pin::INPUT:  _inDegreeCell[i]++; break;
      case Pin::OUTPUT: _outDegreeCell[i]++; break;
      default: break;
      }
    }
  }
  
  for (i=0; i<m; i++) {
    PinForNetCursor pc(_H.getNet(i));
    while (pc.hasNext()) {
      Pin& aPin = pc.getNext();
      switch (aPin.getDirection()) {
      case Pin::INPUT:  _outDegreeNet[i]++; break;
      case Pin::OUTPUT: _inDegreeNet[i]++; break;
      default: break;
      }
    }
  }

  // Initialize both S_l and S_r to be empty lists
  _Sl.clear();
  _Sr.clear();

  _srcQueue.clear();
  _sinkQueue.clear();

  for (i=0; i<n; i++) {
    if (_outDegreeCell[i] == 0) { // sink and isolated vertices
      _sinkQueue.pushRandom(_vertexArray[i]);
    } 
    else if (_inDegreeCell[i] == 0) {
      _srcQueue.pushRandom(_vertexArray[i]);
    } 
    else {
      _bucketPQ->pushBackFast(_vertexArray[i], 
                                _outDegreeCell[i] - _inDegreeCell[i]);
    }
  }

  _bucketPQ->resync();
}



/** Select an independent set of nets based on signal flow (experimental). */ 
unsigned int CycleRemovalServ::
selectIndependentNets(const vector<unsigned int>& netWeights)
{
  const int m = _H.getNumNets();
  int numOfNetsSelected = 0;
  GNode* netArray = new GNode[m];
  const Net* const n0 = &_H.getNet(0);
  const Cell* const c0 = &_H.getCell(0);

  int i;
  for (i=0; i<m; i++) {
    Net& e = _H.getNet(i);
    e.clrVisited();
    //xxx netArray[i].setInfo((void*) &e);
  }

  _H.clrVisitedAllCells();

  // Alternately get the sink and source  
  GDList<GNode> L2;
  while (!_Sl.isEmpty()) {
    Vertex& v = _Sl.popBack();
    //xxx Cell& c = *(Cell*) v.getInfo();
    const unsigned int i_v = &v - _vertexArray;
    Cell& c = _H.getCell(i_v);
    c.setVisited();
    NetCursor nc(c);
    Net* minNet = 0;
    int minWeight = INT_MAX;
    while (nc.hasNext()) {
      Net& e = nc.getNext();
      if (e.isVisited()) continue; // Need to check??
      int weight = netWeights[&e-n0];
      if (minWeight > weight) {
        minNet = &e;
        minWeight = weight;
      }
    }
    if (minNet == 0) continue;
    L2.pushBack(netArray[minNet - n0]);
    numOfNetsSelected++;
    CellCursor cc(*minNet);
    while (cc.hasNext()) {
      Cell& aCell = cc.getNext();
      if (!aCell.isVisited()) {
        _Sl.detach(_vertexArray[&aCell - c0]);
        aCell.setVisited();
      }
      NetCursor nnc(aCell);
      while (nnc.hasNext()) {
        Net& aNet = nnc.getNext();
        aNet.setVisited();
      }
    }
  }

  _H.clrVisitedAllNets();

  GDListCursor<GNode> lc(L2);
  while (lc.hasNext()) {
    const GNode& gnet = lc.getNext();
    //xxx Net& e = *(Net*) gnet.getInfo();
    const unsigned int i_e = &gnet - netArray;
    Net& e = _H.getNet(i_e);
    e.setVisited();
  }

  delete [] netArray;

  return numOfNetsSelected;
}



//xxx /** Construct a coarsen graph based on signal flow (Experimental). */
//xxx Netlist* CycleRemovalServ::contract()
//xxx {
//xxx   const unsigned int m = _H.getNumNets();
//xxx   // Find the net weights
//xxx   std::vector<unsigned int> netWeights(m);
//xxx   _H.clusterSize(netWeights);
//xxx   //xxx _H.pinCount(netWeights);
//xxx   unsigned int numNetsSelected = selectIndependentNets(netWeights);
//xxx   return _H.contractCore(numNetsSelected);
//xxx }
