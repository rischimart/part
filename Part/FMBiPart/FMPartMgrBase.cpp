#include "FMPartMgrBase.hpp"
#include "FMException.hpp"
#include "FMLimits.hpp"
#include "FMParam.hpp"
#include <Netlist.hpp>
#include <GNode.hpp>
#include <GDList.hpp>
#include <iostream>
#include <limits.h>
#include <values.h>  // DBL_MAX
#include <cmath> // for lrint(), log(), pow()
#include <vector>
#include <climits>  //added by Zhou
#include <cassert>  //added by Zhou
#include <climits>  //added by Zhou

using std::vector;

FMPartMgrBase::FMPartMgrBase(const FMParam& param) :
  _param(param),
  _H(param._H), 
  _balTol(10.0),
  _verbosity(0),
  _avgCost(DBL_MAX),
  _bestCost(UINT_MAX),
  _maxdiff(INT_MAX), 
  _totalWeight(_H.getTotalWeight()),
  _numCells(_H.getNumCells()),
  _numNets(_H.getNumNets()),
  _boundType(0),
  _lowerBound(0),
  _noInit(false), 
  _initCost(UINT_MAX),
  _cellList(&_H.getCell(0)),
  _netList(&_H.getNet(0)),
  _needSetHighFanoutNets(true),
  _needSetFixedCells(true),
  _pvalue(1),
  _qvalue(1),
  _allowIllegal(false)
{
  // In order to save memory, gains are represented by short integer.
  // Therefore, the maximum degree of the hypergraph (including
  // its contracted subgraphs) cannot exceed 2^15 - 1.
  if (_H.getMaxDegree() > FMLimits::maxDegreeOfCells()) {
    throw FMException(FMException::FM_EXCEED_MAX_DEGREE);
  }
}


/** @return the number of partitions. */
inline unsigned int FMPartMgrBase::getNumPartitions() const 
{ 
  return _param._K; 
}

/** Set the balance tolerance. Precondition: 0.0 < balTol < 1 */ 
void FMPartMgrBase::setBalanceTol(double balTol) 
{ 
  assert(0.0 < balTol && balTol < 1.0);
  _balTol = balTol;

  if (getBoundType() == 0) {
    const double avgWeight = getTotalWeight() / (double) getNumPartitions();
    const double gap = getBalanceTol() * avgWeight;
    setLowerBound(lrint(avgWeight - gap));
    setUpperBound(lrint(avgWeight + gap));
  }
  else if (getBoundType() == 1) {
    const double pwr = log(getNumPartitions()) / log(2.0);
    const double lower = pow(0.5 - getBalanceTol(), pwr);
    const double upper = pow(0.5 + getBalanceTol(), pwr);
    setLowerBound(lrint(lower * getTotalWeight()));
    setUpperBound(lrint(upper * getTotalWeight()));
  }
}


void FMPartMgrBase::initNetlist()
{ 
  //xxx if (_needSetHighFanoutNets) {
  //xxx   // Skip high fan-out nets by marking it as FIXED
  //xxx   const int count = getNetlist().markHighFanOutNets();
  //xxx   if (count > 0 && getVerbosity() > 0) {
  //xxx     std::cerr << "Warning: Skipping " << count 
  //xxx               << " high fanout nets." << std::endl;
  //xxx   }
  //xxx }

  if (_needSetFixedCells) {
    if (getNetlist().hasFixedCells()) {
      // Skip big unmovable cells by marking them as visited
      unsigned int i_v;
      for (i_v=0; i_v<getNumCells(); ++i_v) {
        Cell& c = _cellList[i_v];
        // if (c.isFixed() || c.getWeight()>getConstraintMgr().getAllowDiff()) {
        if (c.isFixed()) c.setVisited();
        else c.clrVisited();
      }    
      //xxx getNetlist().markZeroPinNets();
    } else {
      getNetlist().clrVisitedAllCells();
    }
  }
}



/** Multiple run heuristic. Assume some randomness in initPart() and
    runFMOne(). */
double FMPartMgrBase::doPartition(vector<unsigned char>& part, 
                                  unsigned int starts)
{
  vector<unsigned char> ttpart(part);
  double elapsed = 0.0;
  int bestDiff = INT_MAX;
  _bestCost = UINT_MAX,
  _avgCost = 0.0;

  if (_noInit && _initCost == INT_MAX) {
    _initCost = cutCost(part);
  }

  initBase();

  for (unsigned int i=0; i<starts; i++) {
    clock_t before = clock();
    unsigned int curCost = doPartitionOne(ttpart);
    if (curCost < _bestCost) {
      part = ttpart;
      _bestCost = curCost;
      bestDiff = _maxdiff;
    }
    elapsed += double(clock() - before)/CLOCKS_PER_SEC;
    _avgCost += curCost;
  }

  _maxdiff = bestDiff;
  _avgCost = _avgCost / starts;

  return elapsed;
}
  
