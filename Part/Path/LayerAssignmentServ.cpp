#include "LayerAssignmentServ.hpp"
#include <GBPQueue.hpp>
#include <GDList.hpp>
#include <Netlist.hpp>
#include <string.h>
#include <iostream>
#include <fstream>
#include <cassert>  //added by Zhou

using namespace std;

LayerAssignmentServ::LayerAssignmentServ(Netlist& H) 
  : CycleRemovalServ(H)
{
  const int n = _H.getNumCells();
  const int m = _H.getNumNets();
  _layerCell = new int[n];
  _layerNet = new int[m];
}


LayerAssignmentServ::~LayerAssignmentServ()
{
  delete [] _layerCell;
  delete [] _layerNet;
}


void LayerAssignmentServ::doAssignment()
{
  const int n = _H.getNumCells();
  const int m = _H.getNumNets();
  memset(_layerCell, 0, sizeof(int)*n);
  memset(_layerNet, 0, sizeof(int)*m);
  const Cell* const c0 = &_H.getCell(0);
  const Net* const n0 = &_H.getNet(0);

  doRemove();

  //xxx for (int i=0; i<n; i++) {
  //xxx   Cell* c = _H.getCell(i);
  //xxx   c->clrVisited();
  //xxx }

  _H.clrVisitedAllCells();

  GDListCursor<Vertex> lc(_Sl);
  while (lc.hasNext()) {
    const Vertex& vertex = lc.getNext();
    //xxx Cell& cellFrom = *(Cell*) vertex.getInfo();
    const unsigned int i_v = &vertex - _vertexArray;
    Cell& cellFrom = _H.getCell(i_v);
    cellFrom.setVisited();
    PinForCellCursor pc(cellFrom);
    while (pc.hasNext()) {
      Pin& pinFrom = pc.getNext();
      if (pinFrom.getDirection() == Pin::INPUT) continue;
      Net& net = pinFrom.getNet();
      if (_layerNet[&net - n0] < _layerCell[&cellFrom - c0] + 1) {
        _layerNet[&net - n0] = _layerCell[&cellFrom - c0] + 1;
      }
      PinForNetCursor pnc(net);
      while (pnc.hasNext()) {
        Pin& pinTo = pnc.getNext();
        if (pinTo.getDirection() == Pin::OUTPUT) continue;
        Cell& cellTo = pinTo.getCell();
        assert(!cellTo.isVisited());
        if (_layerCell[&cellTo - c0] < _layerCell[&cellFrom - c0] + 2) {
          _layerCell[&cellTo - c0] = _layerCell[&cellFrom - c0] + 2;
        }
      }
    }
  }
}



int LayerAssignmentServ::writeDot(const char* fileName) const
{ 
  ofstream out(fileName);

  if (out.fail()) {
    cerr << " Could not open " << fileName; 
    exit(0);
  }

  int n = _H.getNumCells();
  int m = _H.getNumNets();
  int padOffset = n - _H.getNumPads() - 1;

  int i;

  out << "digraph H {\n";
  out << "\trankdir=LR;\n";
  out << "\t{node [shape=circle];\n";
  out << "\t";

  for (i=0; i<m; i++) {
    out << " n" << i << ";" ;
  }

  out << " }\n";

  for (i=0; i<n; i++) {
    const Cell& c = _H.getCell(i);
    out << "\t{rank=" << _layerCell[i] << "; ";
    if (c.isPad()) out << "p" << i - padOffset;
    else out << "a" << i;
    out << ";}\n";
  }

  for (i=0; i<m; i++) {
    out << "\t{rank=" << _layerNet[i] << "; ";
    out << "n" << i;
    out << ";}\n";
  }

  for (i=0; i<n; i++) {
    const Cell& c = _H.getCell(i);

    PinForCellConstCursor pcc(c);
    while (pcc.hasNext()) {
      const Pin& p = pcc.getNext();
      const Net& e = p.getNet();
      out << "\t";
      if (p.getDirection() == Pin::OUTPUT) {
        if (c.isPad()) {
          out << "p" << i - padOffset;
	}
        else {
          out << "a" << i;
        }
        out << " -> ";
        out << "n" << _H.getId(e);
      }
      else {
        out << "n" << _H.getId(e);
        out << " -> ";
        if (c.isPad()) {
          out << "p" << i - padOffset;
	}
        else {
          out << "a" << i;
        }
      }
      out << ";\n";
    }
  }

  out << "}\n";
  return 1;
}

