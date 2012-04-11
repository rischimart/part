#include "FMPartTmpl.hpp"
#include <FMMoveInfo.hpp>
#include <FMParam.hpp>
#include <Netlist.hpp>
#include <iostream>
#include <vector>
#include <cassert>  //added by Zhou

using std::vector;
using std::cout; using std::endl; // for debug
/** One problem of object oriented programming is that it tends to
    create many small files and they might be scattered in different
    directories. It creates difficulty for traditional
    programmers to follow codes. If you are lucky to be working in
    an Integrated Developement Environment (IDE), you probably already
    has object browsers or other tools to help. If you are just using
    EMacs (like me), try to use "etag" and "ebrowse".
    [ The old practice tends to put as many code as possible in a
    single file because one can easily use the "search" feature of editor
    (such as vi) to browse the code. Also, printing out source codes
    was common in the old day. It may be another reason why people
    liked to do that. ]
 */


/** Constructor */
template <class FMPartCore, class GainTmpl>
FMPartTmpl<FMPartCore, GainTmpl>::FMPartTmpl(const FMParam& param) :
  FMPartCore(param),
  _gainMgr(param),
  _numOfMoves(0)
{
  // for (unsigned int p=0; p<10; ++p) _stat[p] = 0;
}


/** Destructor */
template <class FMPartCore, class GainTmpl>
FMPartTmpl<FMPartCore, GainTmpl>::~FMPartTmpl()
{
  // std::cout << " Stat1 : ";
  // for (unsigned int p=0; p<10; ++p) {
  //   std::cout << _stat[p] << " ";
  // }
  // std::cout << std::endl;
}



template <class FMPartCore, class GainTmpl>
inline void FMPartTmpl<FMPartCore, GainTmpl>::
init(const vector<unsigned char>& part)
{
  FMPartCore::initGainCore(part);
  _gainMgr.init(part, FMPartCore::_gain);
  _tpart = part;
}



template <class FMPartCore, class GainTmpl>
inline bool FMPartTmpl<FMPartCore, GainTmpl>::isSatisfied()
{
  if (FMPartCore::_allowIllegal) return true;
  return FMPartCore::getConstraintMgr().satisfiedAll();
}


// Reference: Introduction to VLIS Physical Design
template <class FMPartCore, class GainTmpl>
inline int FMPartTmpl<FMPartCore, GainTmpl>::findNext()
{
  int Gaink = 0;
  int Gain_i = 0;

  _numOfMoves = 0;
  _freeVertexList.clear();

  const unsigned int maxNumMoves
    = FMPartCore::getNumCells() * FMPartCore::_pvalue / FMPartCore::_qvalue;
  unsigned int totalNumMoves=0;
  while (1) {
    FMMoveInfo moveInfo;
    Vertex* vertex = _gainMgr.select(moveInfo, _tpart,
                                     FMPartCore::getConstraintMgr());
    if (vertex == 0) break;
    ++totalNumMoves;
    Cell& v = *moveInfo._cell;
    v.setVisited();               // don't update
    v.lock();                     // lock the cell
    Gain_i += _gainMgr.getGain(moveInfo); // get the gain from gainMgr

    const bool isSatisfiedAll = isSatisfied();
    FMPartCore::getConstraintMgr().updateMove(moveInfo);
    updateMove(_tpart, moveInfo, _gainMgr);
    const unsigned int i_v = moveInfo._i_v;
    _tpart[i_v] = moveInfo._toPart;
    vertex->setKey(i_v); // do it after the detaching
    _freeVertexList.pushBack(*vertex);

    if (Gaink < Gain_i || !isSatisfiedAll) {
      Gaink = Gain_i;                   // keep track the maximun gain
      setMaxDiff(FMPartCore::getConstraintMgr().getMaxDiff());
      _numOfMoves = totalNumMoves;
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
    Cell& v = FMPartCore::_cellList[i_v];
    v.clrVisited();
    v.unlock(); // unlock the cell
  }

  FMPartCore::updateNetStatus(); // unlock the nets for next pass or next multiple run

  return Gaink;
}


// Actually update the solution
template <class FMPartCore, class GainTmpl>
inline int FMPartTmpl<FMPartCore, GainTmpl>::
update(vector<unsigned char>& part)
{
  int Gaink = 0;

  GDListCursor<Vertex> lc(_freeVertexList);
  for (unsigned int i=0; i<getNumMoves(); ++i) {
    Vertex& vertex = lc.getNext();
    const unsigned int i_v = vertex.getKey();
    const unsigned int fromPart = part[i_v];
    const unsigned int toPart = _tpart[i_v];
    FMMoveInfo moveInfo(i_v, FMPartCore::_cellList[i_v], fromPart, toPart);
    Gaink += FMPartCore::updateCore(part, moveInfo);
    part[i_v] = toPart;
  }

  _gainMgr.updateGain(part, FMPartCore::_gain);
  // Initialize/Re-initialize
  _tpart = part;
  FMPartCore::getConstraintMgr().setDiff2(FMPartCore::_diff);

  return Gaink;
}



/** A single FM run. @return the solution part and the total gain. */
template <class FMPartCore, class GainTmpl>
unsigned int FMPartTmpl<FMPartCore, GainTmpl>::
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
unsigned int FMPartTmpl<FMPartCore, GainTmpl>::
doPartitionOne4(vector<unsigned char>& part)
{
  FMPartCore::initBase();
  return doPartitionInternal(part);
}


/** A single FM run. @return the solution part and the total gain. */
template <class FMPartCore, class GainTmpl>
inline unsigned int FMPartTmpl<FMPartCore, GainTmpl>::
doPartitionInternal(vector<unsigned char>& part)
{
  init(part);
  int totalGain = 0;

  do {
    findNext();

    if (getNumMoves() > 0) {
      int GainU = update(part);
      cout << "In FMPartTmpl::doPartitionInternal" << "totalGain = " << totalGain << endl;
      totalGain += GainU;
      cout << "In FMPartTmpl::doPartitionInternal" << "GainU = " << GainU << endl;
      cout << "In FMPartTmpl::doPartitionInternal" << "totalGain = " << totalGain << endl;
      cout << endl;
    }
  } while (getNumMoves() > 0);

  assert((int) FMPartCore::_initCost >= totalGain);
  /** Note than if a circuit contains high fan-out nets that are
  skipped during the partition process, the following assertion may
  not hold. */
  cout << "FMPartTmpl::doPartitionInternal" << endl;
  cout << "FMPartCore::_initCost = " << FMPartCore::_initCost << endl;
  cout << "totalGain = " << totalGain << endl;
  cout << "newcost = " << FMPartCore::getNetlist().cutCost(part, FMPartCore::getNumPartitions()) << endl;
  cout <<"=========================================" << endl;
  assert(FMPartCore::_initCost - totalGain ==
         FMPartCore::getNetlist().cutCost(part, FMPartCore::getNumPartitions()));
  return FMPartCore::_initCost - totalGain;
}


/** Explicit Instantiation */
#include <FMKWayGainMgr2.hpp>
#include <FMKWayGainMgr3.hpp>
#include <FMKWayGainMgr4.hpp>
#include <FMKWayPartCore.hpp>

/** An explicit instantiation request is a declaration of a
    specialization prefixed by the keyword template (not followed by <) */
template class FMPartTmpl<FMKWayPartCore, FMKWayGainMgr2>;
template class FMPartTmpl<FMKWayPartCore, FMKWayGainMgr3>;
template class FMPartTmpl<FMKWayPartCore, FMKWayGainMgr4>;

#include <FMBiGainMgr3.hpp>
#include <FMBiGainMgr2.hpp>
#include <FMBiGainMgr.hpp>
#include <FMBiPartCore.hpp>

/** An explicit instantiation request is a declaration of a
    specialization prefixed by the keyword template (not followed by <) */
template class FMPartTmpl<FMBiPartCore, FMBiGainMgr>;
template class FMPartTmpl<FMBiPartCore, FMBiGainMgr2>;
template class FMPartTmpl<FMBiPartCore, FMBiGainMgr3>;

