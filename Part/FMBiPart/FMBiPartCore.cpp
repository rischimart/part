#include "FMBiPartCore.hpp"
#include "FMBiGainArray.hpp"
#include <Netlist.hpp>
#include <GDList.hpp>
#include <vector>
#include <cassert>  //added by Zhou

using std::vector;

/** Constructor */
FMBiPartCore::FMBiPartCore(const FMParam& param) : 
  FMBiPartComm(param) 
{
  //xxx _netStatus.resize(H.getNumNets());
  //xxx updateNetStatus();
}


// Actually update the solution by re-applying the sequence of
// moves. The sequence is kept by the freeVertexList.
int FMBiPartCore::updateCore(const vector<unsigned char>& part,
                             const FMMoveInfo& moveInfo)
{
  const unsigned int weight = moveInfo._cell->getWeight();
  _diff[moveInfo._toPart] += weight;
  _diff[moveInfo._fromPart] -= weight;
  int Gaink = _gain[moveInfo._i_v];
  FMBiGainArray gainArray(_gain);
  updateMoveCore(part, moveInfo, gainArray);
  return Gaink;
}



/** Initialize the gain structure (c.f. Sarrafzadeh and Wong, page 38) */
void FMBiPartCore::initGainCore(const vector<unsigned char>& part)
{
  std::fill(_gain.begin(), _gain.end(), 0);
  for (unsigned int i_e=0; i_e<getNumNets(); ++i_e) {
    const Net& e = _netList[i_e];
    if (e.isHighFanout()) continue;  // skip net such as high fan-out nets
    initGain(e, part);      // initialize the gains for each net e
  }
}


void FMBiPartCore::updateNetStatus()
{
  //xxx std::fill(_netStatus.begin(), _netStatus.end(), 254);
}



/** When the cell v is moved from fromPart, update the gains and the
    gain structures of its neighbors. (c.f. Sarrafzadeh and Wong, 
    page 39)  */ 
inline void FMBiPartCore::
updateMoveCore(const vector<unsigned char>& part, ///< current partition
               const FMMoveInfo& moveInfo,       ///< Info. of the moving cell
               FMBiGainArray& ga                ///< gain related structre
           )
{
  const Cell& v = *moveInfo._cell;
  const unsigned int fromPart = moveInfo._fromPart;

  NetConstCursor ec(v);
  while (ec.hasNext()) {
    const Net& e = ec.getNext();
    if (e.isHighFanout()) continue;

    // Calculate the number of modules on each partition
    CellConstCursor nc(e);
    assert(nc.count() >= 2);
    if (nc.count() == 2) { // special handle for two-pin nets
      updateMove2PinNet(e, part, fromPart, v, ga);
    } else {
      _num[0] = 0;  // number of modules connected by e that are in A
      _num[1] = 0;  // number of modules connected by e that are in B
      while (nc.hasNext()) {
        const Cell& w = nc.getNext();
        if (&w == &v) continue;
        ++_num[part[&w - _cellList]];
      }
      updateMoveGeneralNet(e, part, fromPart, v, ga);
    }
  }

  ga.updateMove(moveInfo);
}




/** When the cell v is moved from fromPart, update the gains and the
    gain structures of its neighbors. (c.f. Sarrafzadeh and Wong, 
    page 39)  */ 
template <class MoveStruct>
void FMBiPartCore::
updateMove(const vector<unsigned char>& part, ///< current partition solution
           const FMMoveInfo& moveInfo,
           MoveStruct& ms                     ///< gain related structre
           )
{
  Cell& v = *moveInfo._cell;
  const unsigned int fromPart = moveInfo._fromPart;
  //xxx const unsigned int toPart = moveInfo._toPart;

  NetCursor ec(v);
  while (ec.hasNext()) {
    Net& e = ec.getNext();
    if (e.isHighFanout()) continue;

    //xxx const unsigned int i_e = &e - _netList;
    //xxx if (_netStatus[i_e] == 255) continue; // locked net

    // Calculate the number of modules on each partition
    CellConstCursor nc(e);
    assert(nc.count() >= 2);
    if (nc.count() == 2) { // special handle for two-pin nets
      updateMove2PinNet(e, part, fromPart, v, ms);
    } else {
      _num[0] = 0;  // number of modules connected by e that are in A
      _num[1] = 0;  // number of modules connected by e that are in B
      while (nc.hasNext()) {
        const Cell& w = nc.getNext();
        if (&w == &v) continue;
        ++_num[part[&w - _cellList]];
      }
      updateMoveGeneralNet(e, part, fromPart, v, ms);
    }

    //xxx if (_netStatus[i_e] == 254) {  // free net 
    //xxx   _netStatus[i_e] = toPart; // become a loose net
    //xxx   CellCursor cc(e);
    //xxx   while (cc.hasNext()) {
    //xxx     Cell& f = cc.getNext();
    //xxx     if (f.isVisited() || &f == &v) continue;
    //xxx     const unsigned int i_f = &f - _cellList;
    //xxx     if (part[i_f] == _netStatus[i_e]) continue;
    //xxx     ms.updateNeighbor(part[i_f], i_f, 0);
    //xxx   }
    //xxx } else { // loose net
    //xxx   if (_netStatus[i_e] != toPart) {
    //xxx     _netStatus[i_e] = 255; // become a locked net
    //xxx   }
    //xxx }
  }

  ms.updateMove(moveInfo);
}


#include "FMBiGainMgr.hpp"
#include "FMBiGainMgr2.hpp"
#include "FMBiGainMgr3.hpp"
#include "FMBiMoveStruct.hpp"

/** Explicitly instantiation */

//xxx template void FMBiPartCore::
//xxx updateMove(const vector<unsigned char>& part, 
//xxx            const FMMoveInfo& moveInfo,
//xxx            FMBiGainArray& s);

template void FMBiPartCore::
updateMove(const vector<unsigned char>& part, 
           const FMMoveInfo& moveInfo,
           FMBiGainMgr& s);

template void FMBiPartCore::
updateMove(const vector<unsigned char>& part, 
           const FMMoveInfo& moveInfo,
           FMBiGainMgr2& s);

template void FMBiPartCore::
updateMove(const vector<unsigned char>& part, 
           const FMMoveInfo& moveInfo,
           FMBiGainMgr3& s);

//xxx template void FMBiPartCore::
//xxx updateMove(const vector<unsigned char>& part, 
//xxx            const FMMoveInfo& moveInfo,
//xxx            FMBiMoveStruct<FMBiGainMgr, FMBiGainArray>& s);
//xxx 
//xxx template void FMBiPartCore::
//xxx updateMove(const vector<unsigned char>& part, 
//xxx            const FMMoveInfo& moveInfo,
//xxx            FMBiMoveStruct<FMBiGainMgr2, FMBiGainArray>& s);
//xxx 
//xxx template void FMBiPartCore::
//xxx updateMove(const vector<unsigned char>& part, 
//xxx            const FMMoveInfo& moveInfo,
//xxx            FMBiMoveStruct<FMBiGainMgr3, FMBiGainArray>& s);
