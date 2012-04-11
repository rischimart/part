#include "FMKWayPartCore4.hpp"
#include "FMKWayGainArray.hpp"
#include <Netlist.hpp>
#include <FMParam.hpp>
#include <GBPQueue.hpp>
#include <vector>
#include <cassert>  //added by Zhou

using std::vector;

/** Constructor */
FMKWayPartCore4::FMKWayPartCore4(const FMParam& param) :
  FMKWayPartComm(param)
{
  for (unsigned int k=0; k<getNumPartitions(); k++) {
    _numA[k].resize(getNumNets());
  }

  //xxx if (getCostModel() == 0) { // HEdge
  //xxx   _netStatus.resize(H.getNumNets());
  //xxx   updateNetStatus();
  //xxx }
}


/** Setup the initial gain. Also setup the gain buckets data
    structure. Take O(n) time. */
void FMKWayPartCore4::initGainCore(const vector<unsigned char>& part)
{
  for (unsigned int k=0; k<getNumPartitions(); ++k) {
    std::fill_n(_gain[k].begin(), getNumCells(), 0);
  }

  switch (getCostModel()) {
  case 0: initGainCoreHEdge(part); break;
  case 1: initGainCoreKMinus1(part); break;
  case 2: initGainCoreSOED(part); break;
  case 3: initGainCoreTSV(part); break;     //added by Zhou
  }
}


/** Setting the initial gain for each partition. Here, we use the
    HyperEdge metric cost model. */
inline void FMKWayPartCore4::
initGainCoreHEdge(const vector<unsigned char>& part)
{
  for (unsigned int i_e=0; i_e<getNumNets(); ++i_e) {
    const Net& e = _netList[i_e];
    if (e.isHighFanout()) continue;
    initGainHEdge(e, part);
    for (unsigned int k=0; k<getNumPartitions(); ++k) {
      _numA[k][i_e] = _num[k];
    }
  }
}


/** Setting the initial gain for each partition. Here, we use the
    (K-1) metric cost model. */
inline void FMKWayPartCore4::
initGainCoreKMinus1(const vector<unsigned char>& part)
{
  for (unsigned int i_e=0; i_e<getNumNets(); ++i_e) {
    const Net& e = _netList[i_e];
    if (e.isHighFanout()) continue;
    initGainKMinus1(e, part);
    for (unsigned int k=0; k<getNumPartitions(); ++k) {
      _numA[k][i_e] = _num[k];
    }
  }
}


/** Setting the initial gain for each partition. Here, we use the
    SOED metric cost model. */
inline void FMKWayPartCore4::
initGainCoreSOED(const vector<unsigned char>& part)
{
  for (unsigned int i_e=0; i_e<getNumNets(); ++i_e) {
    const Net& e = _netList[i_e];
    if (e.isHighFanout()) continue;
    initGainSOED(e, part);
    for (unsigned int k=0; k<getNumPartitions(); ++k) {
      _numA[k][i_e] = _num[k];
    }
  }
}


/** Setting the initial gain for each partition. Here, we use the
    TSV metric cost model. */
inline void FMKWayPartCore4::
initGainCoreTSV(const vector<unsigned char>& part)
{
  for (unsigned int i_e=0; i_e<getNumNets(); ++i_e) {
    const Net& e = _netList[i_e];
    if (e.isHighFanout()) continue;
    initGainTSV(e, part);
    for (unsigned int k=0; k<getNumPartitions(); ++k) {
      _numA[k][i_e] = _num[k];
    }
  }
}


/** Update the gain structure given the moving vertex */
template <class MoveStruct>
void FMKWayPartCore4::
updateMove(const vector<unsigned char>& part,
	   const FMMoveInfo& moveInfo,
           MoveStruct& ms)
{
  std::fill_n(_deltaGainV, getNumPartitions(), 0);
  switch (getCostModel()) {
  case 0: updateMoveHEdge(part, moveInfo, ms); break;
  case 1: updateMoveKMinus1(part, moveInfo, ms); break;
  case 2: updateMoveSOED(part, moveInfo, ms); break;
  case 3: updateMoveTSV(part, moveInfo, ms); break;
  }

  ms.updateMove(moveInfo, _deltaGainV);
}


/** Update the structure according to the move. The gains are measured
    in hyperedge metric. Note that I can think
    of other possible ways to perform the same function. However, don't
    know which way can be the fastest one. */
template <class MoveStruct>
inline void FMKWayPartCore4::
updateMoveHEdge(const vector<unsigned char>& part,
		const FMMoveInfo& moveInfo,
		MoveStruct& ms)
{
  const Cell& v = *moveInfo._cell;
  const unsigned int fromPart = moveInfo._fromPart;
  const unsigned int toPart = moveInfo._toPart;

  NetConstCursor ec(v);
  while (ec.hasNext()) {
    const Net& e = ec.getNext();
    if (e.isHighFanout()) continue;

    const unsigned int i_e = &e - _netList;
    //xxx if (_netStatus[i_e] == 255) {
    //xxx   --_numA[fromPart][i_e];  // number of modules in A
    //xxx   ++_numA[toPart][i_e];  // number of modules in B
    //xxx   continue; // locked net
    //xxx }
    CellConstCursor nc(e);
    assert(nc.count() >= 2);
    if (nc.count() == 2) { // special handle for two-pin nets
      updateMove2PinNet4(e, part, moveInfo, e.getWeight(), ms);
    } else {
      for (unsigned int k=0; k<getNumPartitions(); ++k) {
        _num[k] = _numA[k][i_e];
      }
      updateMoveGeneralNetHEdge4(e, part, moveInfo, ms);
      --_numA[fromPart][i_e];  // number of modules in A
      ++_numA[toPart][i_e];  // number of modules in B
    }

    //xxx if (_netStatus[i_e] == 254) {  // free net
    //xxx   _netStatus[i_e] = toPart; // become a loose net
    //xxx } else { // loose net
    //xxx   if (_netStatus[i_e] != toPart) {
    //xxx     _netStatus[i_e] = 255; // become a locked net
    //xxx   }
    //xxx }
  }
}



/** Update the structure according to the move. The gains are measured
    in (K-1) metric. Note that I can think
    of other possible ways to perform the same function. However, don't
    know which way can be the fastest one. */
template <class MoveStruct>
inline void FMKWayPartCore4::
updateMoveKMinus1(const vector<unsigned char>& part,
		  const FMMoveInfo& moveInfo,
		  MoveStruct& ms)
{
  const Cell& v = *moveInfo._cell;
  const unsigned int fromPart = moveInfo._fromPart;
  const unsigned int toPart = moveInfo._toPart;

  NetConstCursor ec(v);
  while (ec.hasNext()) {
    const Net& e = ec.getNext();
    if (e.isHighFanout()) continue;
    CellConstCursor nc(e);
    assert(nc.count() >= 2);
    if (nc.count() == 2) { // special handle for two-pin nets
      updateMove2PinNet4(e, part, moveInfo, e.getWeight(), ms);
    } else {
      const unsigned int i_e = &e - _netList;
      _num[fromPart] = --_numA[fromPart][i_e];  // number of modules in A
      _num[toPart] = _numA[toPart][i_e]++;  // number of modules in B
      updateMoveGeneralNetKMinus14(e, part, moveInfo, ms);
    }
  }
}





/** Update the structure according to the move. The gains are measured
    in SOED metric. Note that I can think
    of other possible ways to perform the same function. However, don't
    know which way can be the fastest one. */
template <class MoveStruct>
inline void FMKWayPartCore4::
updateMoveSOED(const vector<unsigned char>& part,
	       const FMMoveInfo& moveInfo,
               MoveStruct& ms)
{
  const Cell& v = *moveInfo._cell;
  const unsigned int fromPart = moveInfo._fromPart;
  const unsigned int toPart = moveInfo._toPart;

  NetConstCursor ec(v);
  while (ec.hasNext()) {
    const Net& e = ec.getNext();
    if (e.isHighFanout()) continue;
    CellConstCursor nc(e);
    assert(nc.count() >= 2);
    if (nc.count() == 2) { // special handle for two-pin nets
      updateMove2PinNet4(e, part, moveInfo, 2*e.getWeight(), ms);
    } else {
      const unsigned int i_e = &e - _netList;
      for (unsigned int k=0; k<getNumPartitions(); ++k) {
        _num[k] = _numA[k][i_e];
      }
      updateMoveGeneralNetSOED4(e, part, moveInfo, ms);
      --_numA[fromPart][i_e];  // number of modules in A
      ++_numA[toPart][i_e];  // number of modules in B
    }
  }
}



/** Update the structure according to the move. The gains are measured
    in TSV metric.  */
template <class MoveStruct>
inline void FMKWayPartCore4::
updateMoveTSV(const vector<unsigned char>& part,
		  const FMMoveInfo& moveInfo,
		  MoveStruct& ms)
{
  const Cell& v = *moveInfo._cell;
  const unsigned int fromPart = moveInfo._fromPart;
  const unsigned int toPart = moveInfo._toPart;

  NetConstCursor ec(v);
  while (ec.hasNext()) {
    const Net& e = ec.getNext();
    if (e.isHighFanout()) continue;
    CellConstCursor nc(e);
    assert(nc.count() >= 2);
    const unsigned int i_e = &e - _netList;

    _num[fromPart] = --_numA[fromPart][i_e];  // number of modules in A
    _num[toPart] = _numA[toPart][i_e]++;  // number of modules in B
    updateMoveGeneralNetTSV4(e, part, moveInfo, ms);

  }
}
void FMKWayPartCore4::updateNetStatus()
{
  //xxx if (getCostModel() == 0) {
  //xxx   std::fill(_netStatus.begin(), _netStatus.end(), 254);
  //xxx }
}



#include "FMKWayGainMgr2.hpp"
#include "FMKWayGainMgr3.hpp"
#include "FMKWayGainMgr4.hpp"
#include "FMKWayMoveStruct.hpp"

/** Explicitly instantiation */
template void FMKWayPartCore4::
updateMove(const vector<unsigned char>& part,
	   const FMMoveInfo& moveInfo,
	   FMKWayMoveStruct<FMKWayGainMgr2, FMKWayGainArray>& s);

template void FMKWayPartCore4::
updateMove(const vector<unsigned char>& part,
	   const FMMoveInfo& moveInfo,
	   FMKWayMoveStruct<FMKWayGainMgr3, FMKWayGainArray>& s);

template void FMKWayPartCore4::
updateMove(const vector<unsigned char>& part,
	   const FMMoveInfo& moveInfo,
	   FMKWayMoveStruct<FMKWayGainMgr4, FMKWayGainArray>& s);



