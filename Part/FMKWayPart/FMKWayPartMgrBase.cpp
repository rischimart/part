#include "FMKWayPartMgrBase.hpp"
#include <FMException.hpp>
#include <FMParam.hpp>
#include <Netlist.hpp>
#include <GDList.hpp>
#include <math.h>  // for lrint()
#include <iostream>
#include <vector>
#include <boost/array.hpp>
#include <climits>  //added by Zhou
#include <cassert>  //added by Zhou

using std::vector;
using boost::array;

typedef GNode Vertex;

/** @return the index of minimum element of the array. 
    Precondition: num >=1 */
inline unsigned int minIndex(const array<int,64>& array, unsigned int num)
{
  assert(num > 0);
  unsigned int minIndex = 0;
  int minValue = array[0];
  unsigned int nmatch = 0;
  for (unsigned int i=1; i<num; ++i) {
    if (minValue > array[i]) {
      minIndex = i;
      minValue = array[i];
      nmatch = 1;
    }
    // ties are broken randomly
    else if (minValue == array[i] && rand() % ++nmatch == 0) {
      minIndex = i;
      // minValue = array[i];
    }
  }
  return minIndex;
}


/** Constructor */
FMKWayPartMgrBase::FMKWayPartMgrBase(const FMParam& param) :
  FMPartMgrBase(param), 
  _K(param._K), 
  _cm(param._cm),
  _constraintMgr(_K) 
{
  assert(_K >= 2);
  getNetlist().setCostModel(_cm);
}


//xxx /** Constructor */
//xxx FMKWayPartMgrBase::FMKWayPartMgrBase(Netlist& H, unsigned int K, int CM) 
//xxx   : FMPartMgrBase(H), _K(K), _cm(CM), _constraintMgr(K)
//xxx {
//xxx   assert(K >= 2);
//xxx   H.setCostModel(CM);
//xxx 
//xxx   // Skip big unmovable cells by marking them as visited
//xxx   unsigned int i_v;
//xxx   for (i_v=0; i_v<getNumCells(); ++i_v) {
//xxx     Cell& c = _cellList[i_v];
//xxx     // if (c.isFixed() || c.getWeight() > getConstraintMgr().getAllowDiff()) {
//xxx     if (c.isFixed()) c.setVisited();
//xxx     else c.clrVisited();
//xxx   }
//xxx 
//xxx   // Skip high fan-out nets by marking it as FIXED
//xxx   const int count = getNetlist().markHighFanOutNets();
//xxx   if (count > 0 && getVerbosity() > 0) {
//xxx     std::cerr << "Warning: Skipping " << count 
//xxx               << " high fanout nets." << std::endl;
//xxx   }
//xxx 
//xxx   getNetlist().markZeroPinNets();
//xxx }


inline void FMKWayPartMgrBase::setDiffInternal(const array<int, 64>& diffArray)
{
  getConstraintMgr().setDiff(diffArray);
  setMaxDiff(getConstraintMgr().getMaxDiff());
}


void FMKWayPartMgrBase::setDiff(const array<int, 64>&  diff)
{
  for (unsigned int k=0; k<getNumPartitions(); ++k) {
    _diff[k] = diff[k];
  }
  setDiffInternal(diff);
}

void FMKWayPartMgrBase::setDiff2(const array<int, 64>&  diff)
{
  for (unsigned int k=0; k<getNumPartitions(); ++k) {
    _diff[k] = diff[k];
  }
  setMaxDiff(getConstraintMgr().getMaxDiff());
}

/** Obtain initial feasible solution */
void FMKWayPartMgrBase::initPart(vector<unsigned char>& part)
{
  std::fill_n(_diff.begin(), getNumPartitions(), int(0));
  // Handle fixed cells
  for (unsigned int i=0; i<getNumCells(); ++i) {
    const Cell& c = _cellList[i];
    if (c.isFixed()) _diff[part[i]] += c.getWeight();
  }

  initPartByRandom(part);
}


/** Obtain initial feasible solution. In this implementation, we try
    to optimize the balance factor: the cell weights are sorting in
    descending order and then insert them according to the following
    rule: always insert the next item in the set of smaller overall
    weight (breaking ties arbitrarily. It is possible to show that
    this procedure always returns a solution whose performance ratio
    is bounded by 6/5 and this bound is indeed tight. (see G. Ausiello
    et al. Complexity and Approximation: Combinatorail Optimization
    Problems and Their Approximability Properties, Springer.) */
inline void FMKWayPartMgrBase::initPartBySort(vector<unsigned char>& part)
{
  GDList<Vertex> L;    // a list of unfixed vertics
  getNetlist().initWeightList(L);   // bucket sort in non-decending order
  GDListCursor<Vertex> lc(L);
  const Vertex* const v0 = &getNetlist().getVertex(0);
  
  int weight = INT_MAX;
  while (lc.hasNext()) {
    const unsigned int i_v = &lc.getNext() - v0;
    const Cell& c = _cellList[i_v];
    assert(weight >= c.getWeight()); // assert non-increasing order
    weight = c.getWeight();
    // Find the partition of smallest overall weight
    int smallestPart = minIndex(_diff, getNumPartitions());

    // Put the cell in the partition of smallest overall weight
    part[i_v] = smallestPart;
    _diff[smallestPart] += weight;
  }

  setDiffInternal(_diff);

  // Check if the initial solution meet the balance requirement
  if (!getConstraintMgr().satisfiedAll()) {
    throw FMException(FMException::FM_CANNOT_INIT);
  }
}


/** Initialize partition by moving all the vertices in one partition
    (partition 0). This function takes linear time. */
inline void FMKWayPartMgrBase::initPartByVILE(vector<unsigned char>& part)
{
  for (unsigned int i=0; i<getNumCells(); i++) {
    const Cell& c = _cellList[i];
    if (c.isFixed()) continue;
    // Put the cell in the partition 0
    part[i] = 0;
    _diff[0] += c.getWeight();
  }
  setDiffInternal(_diff);
}


/** Initialize each partition randomly. This function takes in linear
    time. Note that the initial partition may not satisfy the balance
    condition. */  
inline void FMKWayPartMgrBase::initPartByRandom(vector<unsigned char>& part)
{
  for (unsigned int i=0; i<getNumCells(); i++) {
    const Cell& c = _cellList[i];
    if (c.isFixed()) continue;
    // Put the cell in the partition of smallest overall weight
    part[i] = rand()%getNumPartitions();
    _diff[part[i]] += c.getWeight();
  }
  setDiffInternal(_diff);
}



inline void FMKWayPartMgrBase::calcDiff(const vector<unsigned char>& part,
                                        array<int,64>& diff) const
{
  std::fill_n(diff.begin(), getNumPartitions(), int(0));
  for (unsigned int i_v=0; i_v<getNumCells(); ++i_v) {
    const Cell& c = _cellList[i_v];
    diff[part[i_v]] += c.getWeight();
  }
}


bool FMKWayPartMgrBase::checkBalanceOk(const vector<unsigned char>& part)
{
  boost::array<int,64> diffArray;
  calcDiff(part, diffArray);
  setBF();
  setDiffInternal(diffArray);
  return getConstraintMgr().satisfiedAll();
}


void FMKWayPartMgrBase::initDiff(const vector<unsigned char>& part)
{
  calcDiff(part, _diff);
  setDiffInternal(_diff);
  // if (!getConstraintMgr().satisfiedAll()) {
  //   throw FMException(FMException::FM_INVALID_INIT);
  // }
}


void FMKWayPartMgrBase::printDiff() const
{
  std::cout << "Diff: ";
  for (unsigned int k=0; k<getNumPartitions(); ++k) {
    std::cout << _diff[k] << " ";
  }
  std::cout << std::endl;
}


/** @return the cut cost of partition */
unsigned int FMKWayPartMgrBase::
cutCost(const vector<unsigned char>& part) const
{
  return getNetlist().cutCost(part, getNumPartitions());
}
 

inline void FMKWayPartMgrBase::setBF() 
{ 
  //xxx if (getBoundType() == 0) {
  //xxx   const double avgWeight = getTotalWeight() / (double) getNumPartitions();
  //xxx   const double gap = getBalanceTol() * avgWeight;
  //xxx   getConstraintMgr().setLowerBound(lrint(avgWeight - gap));
  //xxx   getConstraintMgr().setUpperBound(lrint(avgWeight + gap));
  //xxx }
  //xxx else if (getBoundType() == 1) {
  //xxx   const double pwr = log(getNumPartitions()) / log(2.0);
  //xxx   const double lower = pow(0.5 - getBalanceTol(), pwr);
  //xxx   const double upper = pow(0.5 + getBalanceTol(), pwr);
  //xxx   getConstraintMgr().setLowerBound(lrint(lower * getTotalWeight()));
  //xxx   getConstraintMgr().setUpperBound(lrint(upper * getTotalWeight()));
  //xxx }
  getConstraintMgr().setUpperBound(getUpperBound());
  getConstraintMgr().setLowerBound(getLowerBound());
}


void FMKWayPartMgrBase::initBase()
{
  setBF();
  initNetlist();
}
