#include "FMPWPartTmpl.hpp"
#include <FMPartTmpl.hpp>
#include <FMBiPartCore.hpp>
#include <FMPartTmpl4.hpp>
#include <FMKWayPartCore4.hpp>
#include <FMKWayGainMgr2.hpp>
#include <FMParam.hpp>
#include <Netlist.hpp>
#include <iostream>
#include <cassert>  //added by Zhou

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


/** Constructor. */
template <class GainTmpl>
FMPWPartTmpl<GainTmpl>::
FMPWPartTmpl(const FMParam& param) :
  FMKWayPartMgrBase(param),
  _moveTo(_K),
  _groupMap(_K),
  _groupInvMap(_K/2), 
  _numGroups(_K/2),
  _sGVec(_numGroups)
{
}


template <class GainTmpl>
inline void FMPWPartTmpl<GainTmpl>::
initGrouping2()
{
  vector<unsigned int> randVector(getNumPartitions());
  for (unsigned int k=0; k<getNumPartitions(); ++k) {
    _moveTo[k] = k;
    _groupMap[k] = 255;
    randVector[k] = k;
  }
  std::random_shuffle(randVector.begin(), randVector.end());
  for (unsigned int j=0; j<_numGroups; ++j) {
    const unsigned int i = _numGroups + j;
    _moveTo[randVector[j]] = randVector[i];
    _moveTo[randVector[i]] = randVector[j];
    _groupMap[randVector[j]] = j;
    _groupMap[randVector[i]] = j;
    _groupInvMap[j] = randVector[j];
  }
}


/** A single FM run. @return the solution part and the total gain. */
template <class GainTmpl>
unsigned int FMPWPartTmpl<GainTmpl>::
doPartitionOne(vector<unsigned char>& part) 
{
  if (isNoInit()) initDiff(part);
  else {
    initPart(part);
    _initCost = getNetlist().cutCost(part, getNumPartitions());
    //xxx std::cout << "InitCost: " << _initCost << '\n';
  }
  return doPartitionInternal(part);
}


/** A single FM run. @return the solution part and the total gain. */
template <class GainTmpl>
unsigned int FMPWPartTmpl<GainTmpl>::
doPartitionOne4(vector<unsigned char>& part) 
{
  initBase();
  return doPartitionInternal(part);
}



/** A single FM run. @return the solution part and the total gain. */
template <class GainTmpl>
inline unsigned int FMPWPartTmpl<GainTmpl>::
doPartitionInternal(vector<unsigned char>& part) 
{
  typedef FMPartTmpl<FMBiPartCore, GainTmpl> PWPartMgrType; 
  typedef FMPartTmpl4<FMKWayPartCore4, FMKWayGainMgr2> RefineType; 

  unsigned int cost1 = _initCost;
  unsigned int cost = _initCost;

  RefineType rfMgr(getParam());
  rfMgr.setBalanceTol(getBalanceTol());
  rfMgr.setPValue(1);
  rfMgr.setQValue(10);
  rfMgr.setVerbosity(getVerbosity());
  rfMgr.setBoundType(getBoundType());
  rfMgr.noNeedSetHighFanoutNets();

  while (1) {
    initGrouping2();
    getNetlist().pairWisePhase1(part, _groupMap, _sGVec);
    //xxx int cost3 = cost;
    cost = cost1;

    for (unsigned int j=0; j<_numGroups; ++j) {
      const FMParam param(*_sGVec[j]);
      PWPartMgrType PWMgr(param);
      //xxx PWMgr.setBalanceTol(getBalanceTol());
      PWMgr.setUpperBound(getUpperBound());
      PWMgr.setLowerBound(getLowerBound());
      // PWMgr.setPValue(_pvalue);
      // PWMgr.setQValue(_qvalue);
      PWMgr.setVerbosity(getVerbosity());
      //xxx PWMgr.setBoundType(getBoundType());
      PWMgr.noNeedSetHighFanoutNets();
      PWMgr.setNoInit(cost);
      vector<unsigned char> pw_part;
      projectUp(part, pw_part, j);
      // cost = PWMgr.doPartitionOne(pw_part);
      boost::array<int, 64>& diff = getDiff();
      const unsigned int part0 = _groupInvMap[j];
      const unsigned int part1 = _moveTo[part0];
      int diff2[2];
      diff2[0] = diff[part0];
      diff2[1] = diff[part1];
      PWMgr.setDiff(diff2);
      cost = PWMgr.doPartitionOne4(pw_part);
      // PWMgr.doPartition(pw_part, 1);
      // cost = PWMgr.getBestCost();
      projectDown(part, pw_part, j);
      int* diff3 = PWMgr.getDiff();
      diff[part0] = diff3[0];
      diff[part1] = diff3[1];
      delete _sGVec[j];
    }
    // printDiff();
    // std::cout << "cost = " << cost << std::endl;

    assert(cost == getNetlist().cutCost(part, getNumPartitions()));
    //xxx initDiff(part);  

    rfMgr.setNoInit(cost); // only refine the solution
    rfMgr.setDiff(getDiff());
    cost1 = rfMgr.doPartitionOne4(part);
    // rfMgr.doPartition(part, 1);
    // cost1 = rfMgr.getBestCost();
    assert(cost1 == getNetlist().cutCost(part, getNumPartitions()));
    setDiff(rfMgr.getDiff());
    // printDiff();
    // std::cout << "cost1 = " << cost1 << std::endl;

    if (cost1 >= cost) break;
  }


  return cost1;
}


template <class GainTmpl>
inline void FMPWPartTmpl<GainTmpl>::
projectUp(const vector<unsigned char>& part,
          vector<unsigned char>& pw_part,
          unsigned int group) const
{
  const Cell* const c0 = &getNetlist().getCell(0);
  const Netlist& sG = *_sGVec[group];
  const unsigned int sn = sG.getNumCells();
  const unsigned int part0 = _groupInvMap[group];
  pw_part.resize(sn);
  unsigned int i;
  for (i=0; i<sn; ++i) {
    const Cell& c = sG.getMaster(i);
    if (part[&c - c0] == part0) pw_part[i] = 0;
    else pw_part[i] = 1;
  }
}

template <class GainTmpl>
inline void FMPWPartTmpl<GainTmpl>::
projectDown(vector<unsigned char>& part,
            const vector<unsigned char>& pw_part,
            unsigned int group) const
{
  const Cell* const c0 = &getNetlist().getCell(0);
  const Netlist& sG = *_sGVec[group];
  const unsigned int sn = sG.getNumCells();
  const unsigned int part0 = _groupInvMap[group];
  unsigned int i;
  for (i=0; i<sn; ++i) {
    const Cell& c = sG.getMaster(i);
    if (pw_part[i] == 0) part[&c - c0] = part0;
    else part[&c - c0] = _moveTo[part0];
  }
}


/** Explicit Instantiation */
#include "FMBiGainMgr2.hpp"

/** An explicit instantiation request is a declaration of a
    specialization prefixed by the keyword template (not followed by <) */
template class FMPWPartTmpl<FMBiGainMgr2>;



