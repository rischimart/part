#include "FMPartTmpl4.hpp"
#include <FMMoveInfo.hpp>
#include <FMParam.hpp>
#include <Netlist.hpp>
#include <iostream>
#include <vector>
#include <cassert>  //added by Zhou

using std::vector;
using std::cout;
using std::endl;
// sdfadfds

/** Constructor */
template <class FMPartCore, class GainTmpl>
FMPartTmpl4<FMPartCore, GainTmpl>::FMPartTmpl4(const FMParam& param) :
  FMPartCore(param),
  _gainMgr(param),
  _numOfMoves(0)
{
  // for (unsigned int p=0; p<10; ++p) _stat[p] = 0;
}


/** Destructor */
template <class FMPartCore, class GainTmpl>
FMPartTmpl4<FMPartCore, GainTmpl>::~FMPartTmpl4()
{
  // std::cout << " Stat4 : ";
  // for (unsigned int p=0; p<10; ++p) {
  //   std::cout << _stat[p] << " ";
  // }
  // std::cout << std::endl;
}



template <class FMPartCore, class GainTmpl>
inline void FMPartTmpl4<FMPartCore, GainTmpl>::
init(const vector<unsigned char>& part)
{
  FMPartCore::initGainCore(part);
  _gainMgr.init(part, FMPartCore::_gain);
  _tpart = part;
}


template <class FMPartCore, class GainTmpl>
inline bool FMPartTmpl4<FMPartCore, GainTmpl>::isSatisfied()
{
  if (FMPartCore::_allowIllegal) return true;
  return FMPartCore::getConstraintMgr().satisfiedAll();
}


// Reference: Introduction to VLIS Physical Design
template <class FMPartCore, class GainTmpl>
inline int FMPartTmpl4<FMPartCore, GainTmpl>::
findNext()
{
  int Gaink = 0;
  int Gain_i = 0;

  _numOfMoves = 0;
  _freeVertexList.clear();

  const unsigned int maxNumMoves
    = FMPartCore::getNumCells() * FMPartCore::_pvalue / FMPartCore::_qvalue;
  unsigned int totalNumMoves = 0;
  while (1) {
    FMMoveInfo moveInfo;
    cout << "before Vertex* in FMPartTmpl4::findNext" << endl;
    Vertex* vertex = _gainMgr.select(moveInfo, _tpart, FMPartCore::getConstraintMgr());

    if (vertex == 0) break;
    //cout << "after Vertex* in FMPartTmpl4::findNext" << endl;
    ++totalNumMoves;
    Cell& v = *moveInfo._cell;
    v.lock();   // lock the cell
    Gain_i += FMPartCore::getGain(moveInfo);  // get the gain from gain array
    //cout << Gain_i << " in FMPartTmpl4::findNext" << endl;
    const bool isSatisfiedAll = isSatisfied();
    FMPartCore::getConstraintMgr().updateMove(moveInfo);
    updateMove4(_tpart, moveInfo, _gainMgr);
    const unsigned int i_v = moveInfo._i_v;
    _tpart[i_v] = moveInfo._toPart;
    vertex->setKey(i_v);    // do it after the detachment
    _freeVertexList.pushBack(*vertex);

    if (Gaink < Gain_i || !isSatisfiedAll) {
      Gaink = Gain_i;                   // keep track the maximun gain
      // setMaxDiff(getConstraintMgr().getMaxDiff());
      _numOfMoves = totalNumMoves;
      if (isSatisfiedAll) break;      // stop immediately (early stop)
    }
    if (totalNumMoves >= maxNumMoves && isSatisfiedAll) break;
  }

  /** This assertion validates the implementation by checking if all
      cells are moved in bi-partitioning, the total gain should
      return to zero (all cells are swapped to the other side). */
  assert(totalNumMoves < FMPartCore::getNumCells()
         || FMPartCore::getNumPartitions() != 2 || Gain_i == 0);

  // if (_numOfMoves != 0) {
  //   ++_stat[_numOfMoves * 10 / getNumCells()];
  // }

  // Unlock the cells for next pass or next multiple run
  GDListConstCursor<Vertex> lc(_freeVertexList);
  while (lc.hasNext()) {
    const Vertex& vertex = lc.getNext();
    const unsigned int i_v = vertex.getKey();
    FMPartCore::_cellList[i_v].unlock(); // unlock the cell
  }

  FMPartCore::updateNetStatus(); // unlock the nets for next pass or next multiple run

  return Gaink;
}


// Actually update the solution
template <class FMPartCore, class GainTmpl>
inline void FMPartTmpl4<FMPartCore, GainTmpl>::
update(vector<unsigned char>& part)
{
  while (!_freeVertexList.isEmpty()) {
    const Vertex& vertex = _freeVertexList.popFront();
    const unsigned int i_v = vertex.getKey();
    const int toPart = _tpart[i_v];
    part[i_v] = toPart; // move to another side
    FMPartCore::pushBack4(toPart, FMPartCore::_cellList[i_v], _gainMgr);
  }

  FMPartCore::setDiff2(FMPartCore::getConstraintMgr().getDiff());
}



/** A single FM run. @return the solution part and the total gain. */
template <class FMPartCore, class GainTmpl>
unsigned int FMPartTmpl4<FMPartCore, GainTmpl>::
doPartitionOne(vector<unsigned char>& part)
{
  if (FMPartCore::isNoInit()) FMPartCore::initDiff(part);
  else {
    FMPartCore::initPart(part);
    FMPartCore::_initCost
      = FMPartCore::getNetlist().cutCost(part, FMPartCore::getNumPartitions());
  }
  return doPartitionInternal(part);
}


/** A single FM run. @return the solution part and the total gain. */
template <class FMPartCore, class GainTmpl>
unsigned int FMPartTmpl4<FMPartCore, GainTmpl>::
doPartitionOne4(vector<unsigned char>& part)
{
  FMPartCore::initBase();
  return doPartitionInternal(part);
}


/** A single FM run. @return the solution part and the total gain. */
template <class FMPartCore, class GainTmpl>
inline unsigned int FMPartTmpl4<FMPartCore, GainTmpl>::
doPartitionInternal(vector<unsigned char>& part)
{
  init(part);
  int totalGain = 0;

  do {
    int Gaink = findNext();
    cout << Gaink;
    if (getNumMoves() > 0) {
      update(part);
    }
  } while (getNumMoves() > 0);
  cout << FMPartCore::_initCost << endl;
  cout << totalGain << endl;
  cout << FMPartCore::getNetlist().cutCost(part, FMPartCore::getNumPartitions()) << endl;
  assert((int) FMPartCore::_initCost >= totalGain);
  /** Note than if a circuit contains high fan-out nets that are
  skipped during the partition process, the following assertion may
  not hold. */

  assert(FMPartCore::_initCost - totalGain ==
         FMPartCore::getNetlist().cutCost(part, FMPartCore::getNumPartitions()));
  return FMPartCore::_initCost - totalGain;
}


/** Explicit Instantiation */
#include <FMKWayGainMgr2.hpp>
#include <FMKWayGainMgr3.hpp>
#include <FMKWayGainMgr4.hpp>
#include <FMKWayPartCore4.hpp>

/** An explicit instantiation request is a declaration of a
    specialization prefixed by the keyword template (not followed by <) */
template class FMPartTmpl4<FMKWayPartCore4, FMKWayGainMgr2>;
template class FMPartTmpl4<FMKWayPartCore4, FMKWayGainMgr3>;
template class FMPartTmpl4<FMKWayPartCore4, FMKWayGainMgr4>;

#include <FMBiGainMgr3.hpp>
#include <FMBiGainMgr2.hpp>
#include <FMBiGainMgr.hpp>
#include <FMBiPartCore4.hpp>

/** An explicit instantiation request is a declaration of a
    specialization prefixed by the keyword template (not followed by <) */
template class FMPartTmpl4<FMBiPartCore4, FMBiGainMgr>;
template class FMPartTmpl4<FMBiPartCore4, FMBiGainMgr2>;
template class FMPartTmpl4<FMBiPartCore4, FMBiGainMgr3>;

