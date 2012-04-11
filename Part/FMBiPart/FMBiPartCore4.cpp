#include "FMBiPartCore4.hpp"
#include <Netlist.hpp>
#include <GDList.hpp>
#include <vector>
#include <cassert>  //added by Zhou

using std::vector;

//xxx /** Constructor. Number of partition is fixed to 2. Cost Model is
//xxx     fixed to be HEdge (0) */
//xxx FMBiPartCore4::FMBiPartCore4(Netlist& H, unsigned int , int ) : 
//xxx   FMBiPartComm(H)
//xxx { 
//xxx   _numA[0].resize(getNumNets());
//xxx   _numA[1].resize(getNumNets());
//xxx   //xxx _netStatus.resize(H.getNumNets()); //$$$
//xxx   //xxx updateNetStatus(); //$$$
//xxx }

/** Constructor */
FMBiPartCore4::FMBiPartCore4(const FMParam& param) : 
  FMBiPartComm(param)
{ 
  _numA[0].resize(getNumNets());
  _numA[1].resize(getNumNets());
  //xxx _netStatus.resize(H.getNumNets()); //$$$
  //xxx updateNetStatus(); //$$$
}


//$$$ update
void FMBiPartCore4::updateNetStatus()
{
  //xxx std::fill(_netStatus.begin(), _netStatus.end(), 254);
}


/** Initialize the gain structure (c.f. Sarrafzadeh and Wong, page 38) */
void FMBiPartCore4::initGainCore(const vector<unsigned char>& part)
{
  std::fill(_gain.begin(), _gain.end(), 0);
  for (unsigned int i_e=0; i_e<getNumNets(); ++i_e) {
    const Net& e = _netList[i_e];
    if (e.isHighFanout()) continue;  // skip high fan-out nets
    initGain(e, part);      // initialize the gains for each net e
    _numA[0][i_e] = _num[0]; // store these numbers so that don't ...
    _numA[1][i_e] = _num[1]; // ... need to re-calculate next time.
  }
}



/** When the cell v is moved from fromPart, update the gains and the
    gain structures of its neighbors. (c.f. Sarrafzadeh and Wong, 
    page 39)  */ 
template <class MoveStruct>
void FMBiPartCore4::
updateMove(const vector<unsigned char>& part, ///< current partition solution
           const FMMoveInfo& moveInfo,
           MoveStruct& ms                     ///< gain related structre
           )
{
  const Cell& v = *moveInfo._cell;
  const unsigned int fromPart = moveInfo._fromPart;
  const unsigned int toPart = moveInfo._toPart;

  NetConstCursor ec(v);
  while (ec.hasNext()) {
    const Net& e = ec.getNext();
    if (e.isHighFanout()) continue;
    const unsigned int i_e = &e - _netList;
    //xxx if (_netStatus[i_e] == 255) continue;  // locked net 
    // Calculate the number of modules on each partition
    CellConstCursor nc(e);
    assert(nc.count() >= 2);
    if (nc.count() == 2) { // special handle for two-pin nets
      updateMove2PinNet(e, part, fromPart, v, ms);
    } else {
      _num[fromPart] = --_numA[fromPart][i_e];  // number of modules in A
      _num[toPart] = _numA[toPart][i_e]++;  // number of modules in B
      updateMoveGeneralNet(e, part, fromPart, v, ms);
    }
  }

  ms.updateMove(moveInfo);
}



/** Update the gain structure given the moving vertex 
    (special for refinement code) */
template <class GainTmpl> void FMBiPartCore4::
updateMove4(const vector<unsigned char>& part, 
                 const FMMoveInfo& moveInfo,
                 GainTmpl& gainMgr)
{
  FMBiGainArray gainArray(_gain);
  FMBiMoveStruct<GainTmpl, FMBiGainArray> gs(gainMgr, gainArray);
  updateMove(part, moveInfo, gs);

  //xxx //$$$ Loose Net Removal algorithm
  //xxx Cell& v = *moveInfo._cell;
  //xxx const unsigned int toPart = moveInfo._toPart;
  //xxx NetCursor nc(v);
  //xxx while (nc.hasNext()) {
  //xxx   Net& e = nc.getNext();
  //xxx   const unsigned int i_e = &e - _netList;
  //xxx   if (_netStatus[i_e] == 254) {  // free net 
  //xxx     _netStatus[i_e] = toPart; // become a loose net
  //xxx     CellCursor cc(e);
  //xxx     while (cc.hasNext()) {
  //xxx       Cell& f = cc.getNext();
  //xxx       if (f.isVisited()) continue;
  //xxx       const unsigned int i_f = &f - _cellList;
  //xxx       if (part[i_f] == _netStatus[i_e]) continue;
  //xxx       gainMgr.updateNeighbor(part[i_f], i_f, 0);
  //xxx     }
  //xxx   } else { // loose net
  //xxx     if (_netStatus[i_e] != toPart) {
  //xxx       _netStatus[i_e] = 255; // become a locked net
  //xxx     }
  //xxx   }
  //xxx }
}



#include "FMBiGainMgr.hpp"
#include "FMBiGainMgr2.hpp"
#include "FMBiGainMgr3.hpp"

/** Explicitly instantiation */

template void FMBiPartCore4::
updateMove(const vector<unsigned char>& part, 
           const FMMoveInfo& moveInfo,
           FMBiGainArray& s);

template void FMBiPartCore4::
updateMove(const vector<unsigned char>& part, 
           const FMMoveInfo& moveInfo,
           FMBiGainMgr& s);

template void FMBiPartCore4::
updateMove(const vector<unsigned char>& part, 
           const FMMoveInfo& moveInfo,
           FMBiGainMgr2& s);

template void FMBiPartCore4::
updateMove(const vector<unsigned char>& part, 
           const FMMoveInfo& moveInfo,
           FMBiGainMgr3& s);

template void FMBiPartCore4::
updateMove(const vector<unsigned char>& part, 
           const FMMoveInfo& moveInfo,
           FMBiMoveStruct<FMBiGainMgr, FMBiGainArray>& s);

template void FMBiPartCore4::
updateMove(const vector<unsigned char>& part, 
           const FMMoveInfo& moveInfo,
           FMBiMoveStruct<FMBiGainMgr2, FMBiGainArray>& s);

template void FMBiPartCore4::
updateMove(const vector<unsigned char>& part, 
           const FMMoveInfo& moveInfo,
           FMBiMoveStruct<FMBiGainMgr3, FMBiGainArray>& s);




/** Explicitly instantiation */

template void FMBiPartCore4::
updateMove4(const vector<unsigned char>& part, 
           const FMMoveInfo& moveInfo,
           FMBiGainMgr& s);

template void FMBiPartCore4::
updateMove4(const vector<unsigned char>& part, 
           const FMMoveInfo& moveInfo,
           FMBiGainMgr2& s);

template void FMBiPartCore4::
updateMove4(const vector<unsigned char>& part, 
           const FMMoveInfo& moveInfo,
           FMBiGainMgr3& s);


