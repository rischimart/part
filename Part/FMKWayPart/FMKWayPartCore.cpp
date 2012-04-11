#include "FMKWayPartCore.hpp"
#include "FMKWayGainArray.hpp"
#include <Netlist.hpp>
#include <GBPQueue.hpp>
#include <vector>
#include <cassert>  //added by Zhou
#include <iostream> // for debug
using std::vector;
using std::cout; using std::endl;  //for debug
/** Constructor */
FMKWayPartCore::FMKWayPartCore(const FMParam& param) :
  FMKWayPartComm(param)
{
  //xxx if (getCostModel() == 0) { // HEdge
  //xxx   _netStatus.resize(H.getNumNets());
  //xxx   updateNetStatus();
  //xxx }
}



// Actually update the solution by re-applying the sequence of moves.
int FMKWayPartCore::updateCore(const vector<unsigned char>& part,
                               const FMMoveInfo& moveInfo)
{
  const Cell* vPtr = moveInfo._cell;
  const unsigned int weight = vPtr->getWeight();
  _diff[moveInfo._toPart] += weight;
  _diff[moveInfo._fromPart] -= weight;
  const int Gaink = _gain[moveInfo._toPart][moveInfo._i_v];
  FMKWayGainArray gainArray(_gain, getNumPartitions());
  updateMoveCore(part, moveInfo, gainArray);
  return Gaink;
}


/** Setup the initial gain. Also setup the gain buckets data
    structure. Take O(n) time. */
void FMKWayPartCore::initGainCore(const vector<unsigned char>& part)
{
  for (unsigned int k=0; k<getNumPartitions(); ++k) {
    std::fill_n(_gain[k].begin(), getNumCells(), 0);
  }

  switch (getCostModel()) {
  case 0: initGainCoreHEdge(part); break;
  case 1: initGainCoreKMinus1(part); break;
  case 2: initGainCoreSOED(part); break;
  case 3: initGainCoreTSV(part); break;      //added by Zhou
  }
}


/** Setting the initial gain for each partition. Here, we use the
    HyperEdge metric cost model. Take O(n) time. */
inline void FMKWayPartCore::
initGainCoreHEdge(const vector<unsigned char>& part)
{
  for (unsigned int i_e=0; i_e<getNumNets(); ++i_e) {
    const Net& e = _netList[i_e];
    if (e.isHighFanout()) continue;
    //xxx _netStatus[i_e] = 254;    // free net
    initGainHEdge(e, part);
  }
}


/** Setting the initial gain for each partition. Here, we use the
    (K-1) metric cost model. Take O(n) time. */
inline void FMKWayPartCore::
initGainCoreKMinus1(const vector<unsigned char>& part)
{
  for (unsigned int i_e=0; i_e<getNumNets(); ++i_e) {
    const Net& e = _netList[i_e];
    if (e.isHighFanout()) continue;
    initGainKMinus1(e, part);
  }
}


/** Setting the initial gain for each partition. Here, we use the
    SOED metric cost model. Take O(n) time. */
inline void FMKWayPartCore::
initGainCoreSOED(const vector<unsigned char>& part)
{
  for (unsigned int i_e=0; i_e<getNumNets(); ++i_e) {
    const Net& e = _netList[i_e];
    if (e.isHighFanout()) continue;
    initGainSOED(e, part);
  }
}

/** Setting the initial gain for each partition. Here, we use the
    TSV metric cost model. Take O(n) time. */ //added by Zhou
inline void FMKWayPartCore::
initGainCoreTSV(const vector<unsigned char>& part)
{
  for (unsigned int i_e=0; i_e<getNumNets(); ++i_e) {
    const Net& e = _netList[i_e];
    if (e.isHighFanout()) continue;
    initGainTSV(e, part);
  }
}


/** Update the gain structure given the moving vertex */
inline void FMKWayPartCore::
updateMoveCore(const vector<unsigned char>& part,
	   const FMMoveInfo& moveInfo,
           FMKWayGainArray& ga)
{
  std::fill_n(_deltaGainV, getNumPartitions(), 0);
  switch (getCostModel()) {
  case 0: updateMoveHEdgeCore(part, moveInfo, ga); break;
  case 1: updateMoveKMinus1Core(part, moveInfo, ga); break;
  case 2: updateMoveSOEDCore(part, moveInfo, ga); break;
  case 3: updateMoveTSVCore(part, moveInfo, ga); break;
  }

  ga.updateMove(moveInfo, _deltaGainV);
}


/** Update the gain structure given the moving vertex */
template <class MoveStruct>
void FMKWayPartCore::
updateMove(const vector<unsigned char>& part,
	   const FMMoveInfo& moveInfo,
           MoveStruct& ms)
{
  //xxx std::fill_n(_deltaGainV, getNumPartitions(), 0);
  switch (getCostModel()) {
  case 0: updateMoveHEdge(part, moveInfo, ms); break;
  case 1: updateMoveKMinus1(part, moveInfo, ms); break;
  case 2: updateMoveSOED(part, moveInfo, ms); break;
  case 3: updateMoveTSV(part, moveInfo, ms); break;
  }

  ms.updateMove(moveInfo, 0 /* dummy */);
}



/** Update the structure according to the move. The gains are measured
    in hyperedge metric. Note that I can think
    of other possible ways to perform the same function. However, don't
    know which way can be the fastest one. */
template <class MoveStruct>
inline void FMKWayPartCore::
updateMoveHEdge(const vector<unsigned char>& part,
		const FMMoveInfo& moveInfo,
		MoveStruct& ms)
{
  //xxx const unsigned int toPart = moveInfo._toPart;

  NetConstCursor ec(*moveInfo._cell);
  while (ec.hasNext()) {
    const Net& e = ec.getNext();
    if (e.isHighFanout()) continue;

    //xxx const unsigned int i_e = &e - _netList;
    //xxx if (_netStatus[i_e] == 255) continue; // locked net
    CellConstCursor nc(e);
    assert(nc.count() >= 2);
    if (nc.count() == 2) { // special handle for two-pin nets
      updateMove2PinNet(e, part, moveInfo, e.getWeight(), ms);
    } else {
      std::fill_n(_num, getNumPartitions(), 0);
      while (nc.hasNext()) {
        const Cell& w = nc.getNext();
        const unsigned int i_w = &w - _cellList;
        ++_num[part[i_w]];
      }
      updateMoveGeneralNetHEdge(e, part, moveInfo, ms);
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
    in hyperedge metric. Note that I can think
    of other possible ways to perform the same function. However, don't
    know which way can be the fastest one. */
inline void FMKWayPartCore::
updateMoveHEdgeCore(const vector<unsigned char>& part,
		    const FMMoveInfo& moveInfo,
		    FMKWayGainArray& ga)
{
  NetConstCursor ec(*moveInfo._cell);
  while (ec.hasNext()) {
    const Net& e = ec.getNext();
    if (e.isHighFanout()) continue;
    CellConstCursor nc(e);
    assert(nc.count() >= 2);
    if (nc.count() == 2) { // special handle for two-pin nets
      updateMove2PinNet4(e, part, moveInfo, e.getWeight(), ga);
    } else {
      std::fill_n(_num, getNumPartitions(), 0);
      while (nc.hasNext()) {
        const Cell& w = nc.getNext();
        const unsigned int i_w = &w - _cellList;
        ++_num[part[i_w]];
      }
      updateMoveGeneralNetHEdge4(e, part, moveInfo, ga);
    }
  }
}



/** Update the structure according to the move. The gains are measured
    in (K-1) metric. Note that I can think
    of other possible ways to perform the same function. However, don't
    know which way can be the fastest one. */
template <class MoveStruct>
inline void FMKWayPartCore::
updateMoveKMinus1(const vector<unsigned char>& part,
		  const FMMoveInfo& moveInfo,
		  MoveStruct& ms)
{
  const Cell& v = *moveInfo._cell;
  NetConstCursor ec(v);
  while (ec.hasNext()) {
    const Net& e = ec.getNext();
    if (e.isHighFanout()) continue;
    CellConstCursor nc(e);
    assert(nc.count() >= 2);
    if (nc.count() == 2) { // special handle for two-pin nets
      updateMove2PinNet(e, part, moveInfo, e.getWeight(), ms);
    } else {
      std::fill_n(_num, getNumPartitions(), 0);
      while (nc.hasNext()) {
        const Cell& w = nc.getNext();
        if (&w == &v) continue;
        const unsigned int i_w = &w - _cellList;
        ++_num[part[i_w]];
      }
      updateMoveGeneralNetKMinus1(e, part, moveInfo, ms);
    }
  }
}



/** Update the structure according to the move. The gains are measured
    in (K-1) metric. Note that I can think
    of other possible ways to perform the same function. However, don't
    know which way can be the fastest one. */
template <class MoveStruct>
inline void FMKWayPartCore::
updateMoveKMinus1Core(const vector<unsigned char>& part,
		      const FMMoveInfo& moveInfo,
		      MoveStruct& ms)
{
  const Cell& v = *moveInfo._cell;
  NetConstCursor ec(v);
  while (ec.hasNext()) {
    const Net& e = ec.getNext();
    if (e.isHighFanout()) continue;
    CellConstCursor nc(e);
    assert(nc.count() >= 2);
    if (nc.count() == 2) { // special handle for two-pin nets
      updateMove2PinNet4(e, part, moveInfo, e.getWeight(), ms);
    } else {
      std::fill_n(_num, getNumPartitions(), 0);
      while (nc.hasNext()) {
        const Cell& w = nc.getNext();
        if (&w == &v) continue;
        const unsigned int i_w = &w - _cellList;
        ++_num[part[i_w]];
      }
      updateMoveGeneralNetKMinus14(e, part, moveInfo, ms);
    }
  }
}





/** Update the structure according to the move. The gains are measured
    in SOED metric. Note that I can think
    of other possible ways to perform the same function. However, don't
    know which way can be the fastest one. */
template <class MoveStruct>
inline void FMKWayPartCore::
updateMoveSOED(const vector<unsigned char>& part,
	       const FMMoveInfo& moveInfo,
               MoveStruct& ms)
{
  NetConstCursor ec(*moveInfo._cell);
  while (ec.hasNext()) {
    const Net& e = ec.getNext();
    if (e.isHighFanout()) continue;
    CellConstCursor nc(e);
    assert(nc.count() >= 2);
    if (nc.count() == 2) { // special handle for two-pin nets
      updateMove2PinNet(e, part, moveInfo, 2*e.getWeight(), ms);
    } else {
      std::fill_n(_num, getNumPartitions(), 0);
      while (nc.hasNext()) {
        const Cell& w = nc.getNext();
        const unsigned int i_w = &w - _cellList;
        ++_num[part[i_w]];
      }
      updateMoveGeneralNetSOED(e, part, moveInfo, ms);
    }
  }
}


/** Update the structure according to the move. The gains are measured
    in SOED metric. Note that I can think
    of other possible ways to perform the same function. However, don't
    know which way can be the fastest one. */
template <class MoveStruct>
inline void FMKWayPartCore::
updateMoveSOEDCore(const vector<unsigned char>& part,
	           const FMMoveInfo& moveInfo,
                   MoveStruct& ms)
{
  NetConstCursor ec(*moveInfo._cell);
  while (ec.hasNext()) {
    const Net& e = ec.getNext();
    if (e.isHighFanout()) continue;
    CellConstCursor nc(e);
    assert(nc.count() >= 2);
    if (nc.count() == 2) { // special handle for two-pin nets
      updateMove2PinNet4(e, part, moveInfo, 2*e.getWeight(), ms);
    } else {
      std::fill_n(_num, getNumPartitions(), 0);
      while (nc.hasNext()) {
        const Cell& w = nc.getNext();
        const unsigned int i_w = &w - _cellList;
        ++_num[part[i_w]];
      }
      updateMoveGeneralNetSOED4(e, part, moveInfo, ms);
    }
  }
}


/** Update the structure according to the move. The gains are measured
    in TSV metric. Note that I can think
    of other possible ways to perform the same function. However, don't
    know which way can be the fastest one. */
template <class MoveStruct>
inline void FMKWayPartCore::
updateMoveTSV(const vector<unsigned char>& part,
		  const FMMoveInfo& moveInfo,
		  MoveStruct& ms)
{
  const Cell& v = *moveInfo._cell;
  NetConstCursor ec(v);
  while (ec.hasNext()) {
    const Net& e = ec.getNext();
    if (e.isHighFanout()) continue;
    CellConstCursor nc(e);
    assert(nc.count() >= 2);
    std::fill_n(_num, getNumPartitions(), 0);
    while (nc.hasNext()) {
        const Cell& w = nc.getNext();
        if (&w == &v) continue;
        const unsigned int i_w = &w - _cellList;
        ++_num[part[i_w]];
    }
    updateMoveGeneralNetTSV(e, part, moveInfo, ms);
  }
}



/** Update the structure according to the move. The gains are measured
    in TSV metric. Note that I can think
    of other possible ways to perform the same function. However, don't
    know which way can be the fastest one. */
template <class MoveStruct>
inline void FMKWayPartCore::
updateMoveTSVCore(const vector<unsigned char>& part,
		      const FMMoveInfo& moveInfo,
		      MoveStruct& ms)
{
  const Cell& v = *moveInfo._cell;
  NetConstCursor ec(v);
  while (ec.hasNext()) {
    const Net& e = ec.getNext();
    if (e.isHighFanout()) continue;
    CellConstCursor nc(e);
    assert(nc.count() >= 2);
    std::fill_n(_num, getNumPartitions(), 0);
    while (nc.hasNext()) {
        const Cell& w = nc.getNext();
        if (&w == &v) continue;
        const unsigned int i_w = &w - _cellList;
        ++_num[part[i_w]];
    }
    updateMoveGeneralNetTSV4(e, part, moveInfo, ms);
  }
}



void FMKWayPartCore::updateNetStatus()
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
//xxx template void FMKWayPartCore::
//xxx updateMove(const vector<unsigned char>& part,
//xxx 	   const FMMoveInfo& moveInfo,
//xxx 	   FMKWayGainArray& s);

template void FMKWayPartCore::
updateMove(const vector<unsigned char>& part,
	   const FMMoveInfo& moveInfo,
	   FMKWayGainMgr2& s);

template void FMKWayPartCore::
updateMove(const vector<unsigned char>& part,
	   const FMMoveInfo& moveInfo,
	   FMKWayGainMgr3& s);

template void FMKWayPartCore::
updateMove(const vector<unsigned char>& part,
	   const FMMoveInfo& moveInfo,
	   FMKWayGainMgr4& s);

