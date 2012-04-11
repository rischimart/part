#include "FMBiPartMgrBase.hpp"
#include "FMException.hpp"
#include "FMParam.hpp"
#include <Netlist.hpp>
#include <GDList.hpp>
#include <iostream>
#include <vector>
#include <climits>  //added by Zhou
#include <cassert>  //added by Zhou

using std::vector;

typedef GNode Vertex;

/** Long function is bad. It makes other people
    difficult to understand the code and hard to maintain. I sometimes
    also write long functions. The excuse usually is the gain of
    run-time performance, even the function only contribute a very
    small portion of total run-time. Since C++ provides the inline
    function method, it should not be an excuse anymore. */



/** Constructor */
FMBiPartMgrBase::FMBiPartMgrBase(const FMParam& param) : 
  FMPartMgrBase(param)
{
}


inline void FMBiPartMgrBase::setDiffInternal(const int* diffArray)
{
  getConstraintMgr().setDiff(diffArray);
  setMaxDiff(getConstraintMgr().getMaxDiff());
}


void FMBiPartMgrBase::setDiff(const int* diff)
{ 
  _diff[0] = diff[0];
  _diff[1] = diff[1];
  setDiffInternal(diff);
}

void FMBiPartMgrBase::setDiff2(const int* diff)
{ 
  _diff[0] = diff[0];
  _diff[1] = diff[1];
  setMaxDiff(getConstraintMgr().getMaxDiff());
}

/** Obtain initial feasible solution */
void FMBiPartMgrBase::initPart(vector<unsigned char>& part)
{
  _diff[0] = _diff[1] = 0;
  // Handle fixed cells
  for (unsigned int i=0; i<getNumCells(); ++i) {
    const Cell& c = _cellList[i];
    if (c.isFixed()) _diff[part[i]] += c.getWeight();
  }

  initPartByVILE(part);
}


/** Obtain initial feasible solution. In this implementation, we try
    to optimize the balance factor: the cell weights are sorting in
    descending order and then insert them according to the following
    rule: always insert the next item in the set of smaller overall
    weight (breaking ties arbitrarily. It is possible to show that
    this procedure always returns a solution whose performance ratio
    is bounded by 6/5 and this bound is indeed tight. (see G. Ausiello
    et al. Complexity and Approximation: Combinatorail Optimization
    Problems and Their Approximability Properties, Springer.) 

    This method runs slower than the randomize method and less
    randomness. The randomize method also enlarges the search space in
    multiple runs because it can start even with infeasible initial
    partitions. It is interesting to know that in which condition the
    FM iteration can eventually produce feasible solution (see
    Netlist::isVILEOk()). There is always a risk in randomize
    initialization. However, looks like some users don't mind. In the
    future, I may implement it as an option.
*/
inline void FMBiPartMgrBase::initPartBySort(vector<unsigned char>& part)
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
    int smallestPart = 0;
    if (_diff[1] < _diff[0] || _diff[1] == _diff[0] && rand()&1==0) {
      smallestPart = 1;
    }
    // Put the cell in the partition of smallest overall weight
    part[i_v] = smallestPart;
    _diff[smallestPart] += weight;
  }

  setDiffInternal(_diff);

  // Check if the initial solution meet the balance requirement
  if (getMaxDiff() > (1.0 + 6.0 * getBalanceTol()) * getTotalWeight() / 5.0) {
    /* this procedure always returns a solution whose performance ratio
    is bounded by 6/5 and this bound is indeed tight. (see G. Ausiello
    et al. Complexity and Approximation: Combinatorail Optimization
    Problems and Their Approximability Properties, Springer.) Note
    that the bound does not consider the present of fixed cells, and
    the feasible solution may exist in some rare cases, and the
    FM_CANNOT_INIT should be thrown instead. Note also that the
    theorem does not extend to the multi-way partition. */
    throw FMException(FMException::FM_IMPOSSIBLE_INIT);
  } else if (!getConstraintMgr().satisfiedAll()) {
    throw FMException(FMException::FM_CANNOT_INIT);
  }
}


/** Initialize partition by moving all the vertices in one partition
    (partition 0). This function takes linear time. */
inline void FMBiPartMgrBase::initPartByVILE(vector<unsigned char>& part)
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
inline void FMBiPartMgrBase::initPartByRandom(vector<unsigned char>& part)
{
  for (unsigned int i=0; i<getNumCells(); i++) {
    const Cell& c = _cellList[i];
    if (c.isFixed()) continue;
    // Put the cell in the partition of smallest overall weight
    part[i] = rand()&1;
    _diff[part[i]] += c.getWeight();
  }
  setDiffInternal(_diff);
}



inline void FMBiPartMgrBase::calcDiff(const vector<unsigned char>& part,
                                      int* diff) const
{
  diff[0] = diff[1] = 0;
  for (unsigned int i_v=0; i_v<getNumCells(); ++i_v) {
    const Cell& c = _cellList[i_v];
    diff[part[i_v]] += c.getWeight();
  }
}


bool FMBiPartMgrBase::checkBalanceOk(const vector<unsigned char>& part)
{
  int diffArray[2];
  calcDiff(part, diffArray);
  setBF();
  setDiffInternal(diffArray);
  return getConstraintMgr().satisfiedAll();
}


void FMBiPartMgrBase::initDiff(const vector<unsigned char>& part)
{
  calcDiff(part, _diff);
  setDiffInternal(_diff);
  // if (!getConstraintMgr().satisfiedAll()) {
  //   throw FMException(FMException::FM_INVALID_INIT);
  // }
}


void FMBiPartMgrBase::printDiff() const
{
  std::cout << "Diff: ";
  for (unsigned int k=0; k<2; ++k) {
    std::cout << _diff[k] << " ";
  }
  std::cout << std::endl;
}


/** @return the cut cost of partition */
unsigned int FMBiPartMgrBase::
cutCost(const vector<unsigned char>& part) const
{
  return getNetlist().cutCost(part, getNumPartitions());
}
 

inline void FMBiPartMgrBase::setBF() 
{ 
  //xxx const double avgWeight = getTotalWeight() / 2.0;
  //xxx const double gap = getBalanceTol() * avgWeight;
  //xxx getConstraintMgr().setUpperBound(lrint(avgWeight + gap));
  getConstraintMgr().setUpperBound(getUpperBound());
  getConstraintMgr().setLowerBound(getLowerBound());
}


void FMBiPartMgrBase::initBase()
{
  setBF();
  initNetlist();
}
