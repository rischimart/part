#include "PathMgr.hpp"
#include "CycleRemovalServ.hpp"
#include <GDList.hpp>
#include <GBPQueue.hpp>
#include <Netlist.hpp>
#include <string.h>

PathMgr::PathMgr(Netlist& H) : _H(H)
{
  const int m = H.getNumPins();
  _origDir = new Pin::DIRECTION[m];

  for (int i=0; i<m; i++) {
    _origDir[i] = H.getPin(i).getDirection();
  }
}

PathMgr::~PathMgr()
{
  delete [] _origDir;
}


/** Remove cycles of netlist by temporarly reversing the directions of
    pins. See "Graph Drawing" Section 9.4 for detail */
void PathMgr::removeCycles()
{
  CycleRemovalServ aCRS(_H);
  aCRS.doRemove();  
}

// Assume the netlist now does not have cycles.
void PathMgr::assignLayers()
{
}
