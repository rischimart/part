#include "Netlist.hpp"
#include <vector>
#include <GDList.hpp>
#include <GNode.hpp>
#include <GBPQueue.hpp>
#include <FMLimits.hpp>
#include <iostream>
#include <list>
#include <cmath> // for lrint()
#include <map>
#include <ext/slist>    // this should be platform dependent
#include <boost/shared_ptr.hpp>
#include <cassert>  //added by Zhou
#include <climits>  //added by Zhou

#include <boost/static_assert.hpp>
//BOOST_STATIC_ASSERT( sizeof(int)   <= 4  );   commented by Zhou
//BOOST_STATIC_ASSERT( sizeof(Cell)  <= 12 );
//BOOST_STATIC_ASSERT( sizeof(Net)   <= 8  );
//BOOST_STATIC_ASSERT( sizeof(Pin)   <= 12 );
//BOOST_STATIC_ASSERT( sizeof(GNode) <= 12 );
/** Make sure the above objects are small */

using std::vector;
using std::cerr;
using std::endl;
using std::pair;
using std::make_pair;
using std::list;
using __gnu_cxx::slist; // this should be platform dependent


// For caching the neighbor information (Note: it is not thread-safe
// to use global variables).
static unsigned int IdVec[32768];
static Cell* CellVec[32768];
//xxx static Net* NetVec[32768];
static const Cell* CellConstVec[32768];
static const Net* NetConstVec[32768];
static list<Net*> NetList; // for constructing contraction sub-graphs


/** Constructor */
Netlist::Netlist() : 
  NetlistBase(), 
  _prune2PinNets(true), 
  _prune3PinNets(true) 
{
}

/** Destructor */
Netlist::~Netlist() 
{ 
  clear(); 
}

/** Reset to an empty Netlist */
void Netlist::clear() 
{ 
  vector<Vertex>().swap(_vertexArray); // see Effective STL Item 17
  vector<Cell*>().swap(_masterList);
  NetlistBase::clear();
} 

/** Reset to an empty master list */
void Netlist::clearMasterList() 
{ 
  vector<Cell*>().swap(_masterList); 
}

/** Initialize the vertexArray. */
void Netlist::initVertexArray()
{
  if (!_vertexArray.empty()) return; // already initialized
  _vertexArray.resize(_numCells);
}


/** @return the number of visited nets which are connected to cell c. */
inline unsigned int countVisitedNeighborNets(const Cell& c)
{
  unsigned int ncount = 0;
  NetConstCursor nc(c);
  while (nc.hasNext()) {
    const Net& e = nc.getNext();
    if (e.isVisited()) ++ncount;
  }
  return ncount;
}


/** 
 * (Experimental) Create net weights by mean of contractibility. 
 * Return the result 
 * to the array netWeights. Net's weight is 0 if it is marked as fixed. 
 * (A net is marked as fixed if it is an interface net or it connects
 * to a fixed cell.). Note that the original algorithm was designed
 * for graph instead of hypergraph. It is unclear if the extension
 * can work.
 */
inline void Netlist::contractibility(vector<unsigned int>& netWeights)
{
  clrVisitedAllCells(); // label all cells unvisited
  clrVisitedAllNets();  // label all nets unvisited
  initVertexArray();

  Vertex* const v0 = &getVertex(0); // array offset
  Cell* const c0 = &getCell(0);  // array offset
  Net* const n0 = &getNet(0);  // array offset

  GBPQueue<Vertex> PQ(0, getMaxDegree());
  for (unsigned int i=0; i<getNumCells(); i++) {
    PQ.pushBack(v0[i], 0); // r(v) = 0 for all vertices
  }

  while (!PQ.isEmpty()) { // there exists unvisited vertex
    Vertex& v = PQ.popFront(); // choose an unvisited vertex x 
    Cell& x = c0[&v - v0];
    NetCursor nc(x); // each net adjcnt to x
    x.setVisited();  // mark x visited
    while (nc.hasNext()) {
      Net& e = nc.getNext();
      if (e.isVisited()) continue;
      e.setVisited(); // mark e visited
      const int weight = e.getWeight();
      unsigned int R = UINT_MAX;
      // unsigned int R = 0;
      CellCursor cc(e); // each cell adjcnt to e
      while (cc.hasNext()) {
        Cell& y = cc.getNext();
        if (y.isVisited()) continue;
        Vertex& vy = v0[&y - c0];
        PQ.increaseKeyLIFOBy(vy, weight); // r(y) = r(y) + w(e)
        const unsigned int r = PQ.getKey(vy);
        if (r < R) R = r; // how to calc. q(e) in hypergraph?
      }
      netWeights[&e - n0] = R; // ??? q(e) = \min_{y} r(y)
    }
  }
}



/** Create net weights by mean of the cluster size. Return the result
    to the array netWeights. Net's weight is 0 if it is marked as fixed. 
    (A net is marked as fixed if it is an interface net or it connects
    to a fixed cell.)
*/
void Netlist::clusterSize(vector<unsigned int>& netWeights) const
{
  std::fill_n(netWeights.begin(), getNumNets(), 0);
  for (unsigned int i=0; i<getNumNets(); i++) {
    const Net& e = getNet(i);
    if (e.isFixed()) { // interface net or high fanout net
      netWeights[i] = UINT_MAX; // does it matter?
      continue;
    }
    CellConstCursor cc(e);
    while (cc.hasNext()) {
      const Cell& c = cc.getNext();
      if (c.isFixed()) continue; // don't count the fixed cell
      netWeights[i] += c.getWeight();
    }
  }
}


/** Create net weights by mean of net pin count. Return the result
    to the array netWeights. Net's weight is UNIT_MAX if it is marked 
    as fixed. */
inline void Netlist::pinCount(vector<unsigned int>& netWeights) const
{
  fill_n(netWeights.begin(), getNumNets(), 0);
  for (unsigned int i=0; i<getNumNets(); i++) {
    const Net& e = getNet(i);
    if (e.isFixed()) {
      netWeights[i] = UINT_MAX;
      continue;
    }
    unsigned int degree = 0;
    CellConstCursor cc(e);
    while (cc.hasNext()) {
      const Cell& c = cc.getNext();
      if (c.isFixed()) continue; // don't count the fixed cell
      ++degree;
    }
    netWeights[i] += degree;
  }
}


/** Create net weights by mean of the number of external pins. Return
    the result to the array netWeights. Net's weight is UINT_MAX if it is
    fixed. The number of external pins is the number of pins of
    clusters that will be formed. */
inline void Netlist::ExternalPins(vector<unsigned int>& netWeights) const
{
  clrVisitedAllCells();
  clrVisitedAllNets();
  fill_n(netWeights.begin(), getNumNets(), 0);

  unsigned int idx;
  unsigned int i;
  for (i=0; i<getNumNets(); i++) {
    const Net& e = getNet(i);
    if (e.isFixed()) {
      netWeights[i] = UINT_MAX;
      continue;
    }
    e.setVisited();
    // Count how many external pins (degree)
    unsigned int degree = 0;
    CellConstCursor cc(e);
    while (cc.hasNext()) {
      const Cell& aCell = cc.getNext();
      if (aCell.isFixed()) continue; // don't count the fixed cell
      aCell.setVisited();
      CellConstVec[degree++] = &aCell; // cache the neighbor cells in CellVec
    }
    assert(degree <= 32768);
    unsigned int degreeNet = 0;
    for (idx=0; idx<degree; ++idx) {
      const Cell& aCell = *CellConstVec[idx];
      NetConstCursor nc(aCell);
      while (nc.hasNext()) {
        const Net& ne = nc.getNext();
        if (ne.isVisited()) continue;
        NetConstVec[degreeNet++] = &ne; // cache the neighbor nets in NetConstVec
        ne.setVisited();
        CellConstCursor cc(ne);
        bool isExternal = false;
        while (cc.hasNext()) {
          const Cell& eachCell = cc.getNext();
          if (eachCell.isFixed()) continue; // don't count the fixed cell
          if (!eachCell.isVisited()) {
            isExternal = true;
            break;
          }
        }      
        if (!isExternal) continue;
        netWeights[i]++;
      }
    }
    assert(degreeNet <= 32768);
    // Unmark visited cells and nets
    for (idx=0; idx<degree; ++idx) CellConstVec[idx]->clrVisited();
    for (idx=0; idx<degreeNet; ++idx) NetConstVec[idx]->clrVisited();
    e.clrVisited();
  }
}






/** Find a set of candidate nets by FC algorithm. The nets
    selected are marked as visited. @return the number of nets selected.
    The algorithm starts with sorting nets in
    non-increasing order according to the net weights. Assume
    that net weights are bounded by a small value so that bucket sorting
    algorithm is used. In [khmetis], the aurthers commented that the
    rate of contractions by this method may be too fast. They imposed
    a constraint that the contraction process will stop if certain
    ratio is reach (1.7 in their case). In MLPart, similar constraint
    is set (1.3 in their case).

    Note: This scheme creates less randomness than minimumNetCover.
*/ 
unsigned int 
Netlist::firstChoice(const vector<unsigned int>& netWeights)
{
  // Setup
  NetList.clear(); // put the selected nets in NetList for contraction step.
  vector<GNode> netArray(getNumNets()); // for bucket sorting
  const GNode* const na0 = &netArray[0];
  unsigned int i=0;

  // unsigned int maxDegree = 0;
  // for (; i<getNumNets(); i++) {
  //   Net& e = getNet(i);
  //   e.clrVisited();
  //   if (e.isFixed()) continue;
  //   if (maxDegree < netWeights[i]) maxDegree = netWeights[i];
  // }

  GBPQueue<GNode> degPQ(0, getMaxDegree()); // tailored for contractibility

  // Sort the nets using bucket sort. Nets with same weight will be
  // put in random order. Question: should we use qsort() or 
  // std::sort() instead?
  for (i=0; i<getNumNets(); i++) {
    if (getNet(i).isFixed()) {
      getNet(i).clrVisited();
      continue;
    }
    degPQ.pushBackFast(netArray[i], netWeights[i]);
  }
  degPQ.resync();

  GDList<GNode> L;
  degPQ.pushTo(L); // convert to a sorted list

  unsigned int numOfSelectedNets = 0;
  GDListCursor<GNode> lc(L);
  while (lc.hasNext()) {
    const GNode& gnet = lc.getNext();
    const unsigned int i_e = &gnet - na0;
    Net& e = getNet(i_e);
    NetList.push_back(&e); // final list is in ascending order
    e.setVisited();
    numOfSelectedNets++;
  }
  return numOfSelectedNets;
}




/** Find an independent set by Greedy algorithm. The nets
    selected are marked as visited. @return the number of nets selected.
    The algorithm starts with sorting nets in
    non-increasing order first according to the net weights. Assume
    that net weights are bounded by a small value so that bucket sorting
    algorithm is used. */ 
unsigned int 
Netlist::maximumIndependentSet(const vector<unsigned int>& netWeights)
{
  // Setup
  NetList.clear(); // put the selected nets in NetList for contraction step.
  vector<GNode> netArray(getNumNets()); // for bucket sorting
  const GNode* const na0 = &netArray[0];

  unsigned int maxDegree = 0;
  unsigned int i=0;
  for (; i<getNumNets(); i++) {
    Net& e = getNet(i);
    e.clrVisited();
    if (e.isFixed()) continue;
    if (maxDegree < netWeights[i]) maxDegree = netWeights[i];
  }

  GBPQueue<GNode> degPQ(0, maxDegree);

  // Sort the nets using bucket sort. Nets with same weight will be
  // put in random order. Question: should we use qsort() or 
  // std::sort() instead?
  for (i=0; i<getNumNets(); i++) {
    if (getNet(i).isFixed()) continue;
    degPQ.pushBackFast(netArray[i], netWeights[i]);
  }
  degPQ.resync();

  GDList<GNode> L;
  degPQ.pushTo(L);
  const Net* const n0 = &getNet(0);
  GDList<GNode> L2;
  while (!L.isEmpty()) {
    GNode& gnet = L.popBack(); // get min value first
    L2.pushBack(gnet);
    const unsigned int i_e = &gnet - na0;
    Net& e = getNet(i_e);
    e.setVisited();
    CellCursor cc(e);
    while (cc.hasNext()) {
      Cell& c = cc.getNext();
      NetCursor nnc(c);
      while (nnc.hasNext()) {
        Net& nnet = nnc.getNext();
        if (nnet.isVisited() || nnet.isFixed()) continue;
        L.detach(netArray[&nnet - n0]); // detach all the neighbor nets ...
        nnet.setVisited();  // ... so that the selected net is independent 
      }
    }
  }

  clrVisitedAllNets();

  unsigned int numOfSelectedNets = 0;
  GDListCursor<GNode> lc(L2);
  while (lc.hasNext()) {
    const GNode& gnet = lc.getNext();
    const unsigned int i_e = &gnet - na0;
    Net& e = getNet(i_e);
    NetList.push_front(&e);
    e.setVisited();
    numOfSelectedNets++;
  }

  assert(maximumIndependentSetOK());
  return numOfSelectedNets;
}


/** Verify if minimumIndependentSet() is OK */
bool Netlist::maximumIndependentSetOK() const
{
  // Check if each cell belongs to at most one selected net
  for (unsigned int i=0; i<getNumCells(); i++) {
    if (countVisitedNeighborNets(getCell(i)) > 1) {
      cerr << "Error (more than one net)" << endl;
      return false;
    }
  }
  return true;
}



/** 
 * Find a minimum net (edge) cover E'', i.e., a subset E'' \in E
 * such that for each cell, at least one of connected nets belongs
 * to E''. A net weight is the sum of cell weight here. The
 * objective is to minimize the total net weights.
 * We explore a similar Primal-Dual algorithm described in [1]
 * that solves the graph weighted vertex cover problem. We generalize
 * the idea of the algorithm for hypergraph. Instead of vertex cover,
 * here we find the hyperedge cover. The overall run time is linear,
 * assuming that the maximum degree is bounded by a small value. The
 * solution by the Primal-Dual algorithm is guaranteed bounded by K
 * times the optimal solution where K is the maximum degree of nets. 
 *
 * Input:     Netlist H
 *            unsigned int Cell::weight
 * Output:    bool Net::isVisited
 * Modified:  bool Cell::isVisited (all covered)
 * @return:    number of nets selected
 *
 * Reference [1]: Program 2.7 in "Complexity and Approximation:
 * Combinatorial Optimization Problems and their Approximability
 * Properties."
 */
unsigned int Netlist::minimumNetCover(const vector<unsigned int>& netWeights)
{
  initVertexArray();

  const Net* const n0 = &getNet(0); // offset of array
  Cell* const c0 = &getCell(0); // offset of array
  Vertex* const va0 = &getVertex(0);

  NetList.clear();
  //xxx unsigned int i;

  GDList<GNode> L;    // the list records the cells that not yet covered.
  initWeightList(L);

  vector<unsigned int> ciList(netWeights); 
  unsigned int numNetsSelected = 0;       // count how many nets are selected

  clrVisitedAllNets();

  // This while loop perform the hyperedge cover problem by the
  // primal-dual algorithm. In this version, vertices are selected
  // randomly. 
  while (!L.isEmpty()) {
    GNode& v = L.popBack();
    const unsigned int i_v = &v - va0;
    Cell& c = getCell(i_v);
    c.setVisited();

    // Look for the net that is not yet selected with minimun ci
    Net* minNet = 0;
    unsigned int degree = 0;
    unsigned int minWeight = UINT_MAX;
    NetCursor nc(c);
    while (nc.hasNext()) {
      Net& e = nc.getNext();
      // if (e.isVisited()) continue;
      if (e.isFixed()) continue;
      const unsigned int eid = &e - n0;
      IdVec[degree++] = eid;            // cache the neighbor nets
      unsigned int weight = ciList[eid];
      if (minWeight > weight) {
        minNet = &e;
        minWeight = weight;
      }
    }
    assert(degree <= 32768);

    if (minNet == 0) continue; // a cell with no net!
    if (!minNet->isVisited()) {
      minNet->setVisited();
      numNetsSelected++;
      NetList.push_back(minNet);

      // Remove the covered cells from the list ???
      CellCursor cc(*minNet);
      while (cc.hasNext()) {
        Cell& v = cc.getNext();
        if (v.isVisited()) continue;
        v.setVisited();
        const unsigned int i_v = &v - c0;
        L.detach(va0[i_v]);
      }
    }

    for (unsigned int idx=0; idx < degree; idx++) {
      ciList[IdVec[idx]] -= minWeight;
    }
  }

  assert(minimumNetCoverOK());  
  return numNetsSelected;
}




unsigned int Netlist::minimumNetCover3(const vector<unsigned int>& netWeights)
{
  initVertexArray();

  const Net* const n0 = &getNet(0); // offset of array
  Cell* const c0 = &getCell(0); // offset of array
  Vertex* const v0 = &getVertex(0); // array offset

  NetList.clear();

  vector<Vertex*> vaRand(getNumCells());
  for (unsigned int i=0; i<getNumCells(); i++) {
    vaRand[i] = v0 + i;     // for randomizing the order of vertices later
  }
  std::random_shuffle(vaRand.begin(), vaRand.end());

  unsigned int numNetsSelected = 0;       // count how many nets are selected
  GBPQueue<Vertex> PQ(0, getMaxDegree());
  for (unsigned int i=0; i<getNumCells(); i++) {
    const unsigned int i_v = vaRand[i] - v0;
    Cell& c = c0[i_v];
    if (c.isFixed()) {
      c.setVisited();
      continue;
    }
    c.clrVisited();
    PQ.pushBack(*vaRand[i], 0); // r(v) = 0 for all vertices
  }

  clrVisitedAllNets(); // label all net unvisited
  vector<unsigned int> ciList(netWeights); 
  vector<bool> isScanned(getNumNets(), false);
  
  // This while loop perform the hyperedge cover problem by the
  // primal-dual algorithm. In this version, vertices are selected
  // according to the Maximum Adjacency (MA) ordering.
  while (!PQ.isEmpty()) {
    Vertex& v = PQ.popFront();
    const unsigned int i_v = &v - v0;
    Cell& c = getCell(i_v);
    c.setVisited();

    // Look for the net that is not yet selected with minimun ci
    Net* minNet = 0;
    unsigned int degree = 0;
    unsigned int minWeight = UINT_MAX;
    NetCursor nc(c);
    while (nc.hasNext()) {
      Net& e = nc.getNext();
      // if (e.isVisited()) continue;
      if (e.isFixed()) continue;
      const unsigned int eid = &e - n0;
      IdVec[degree++] = eid;            // cache the neighbor nets
      unsigned int weight = ciList[eid];
      if (minWeight > weight) {
        minNet = &e;
        minWeight = weight;
      }

      if (isScanned[eid]) continue;
      isScanned[eid] = true;
      const int eweight = e.getWeight();
      CellCursor cc(e); // each cell adjcnt to e
      while (cc.hasNext()) {
        Cell& y = cc.getNext();
        if (y.isVisited()) continue;
        Vertex& vy = v0[&y - c0];
        PQ.increaseKeyLIFOBy(vy, eweight); // r(y) = r(y) + w(e)
      }
    }
    assert(degree <= 32768);

    if (minNet == 0) continue; // a cell with no net!
    if (!minNet->isVisited()) {
      minNet->setVisited();
      numNetsSelected++;
      NetList.push_back(minNet);

      // Remove the covered cells from the list ???
      CellCursor cc(*minNet);
      while (cc.hasNext()) {
        Cell& v = cc.getNext();
        if (v.isVisited()) continue;
        v.setVisited();
        const unsigned int i_v = &v - c0;
        PQ.detach(v0[i_v]);
      }
    }

    for (unsigned int idx=0; idx < degree; idx++) {
      ciList[IdVec[idx]] -= minWeight;
    }
  }

  assert(minimumNetCoverOK());  
  return numNetsSelected;
}



unsigned int Netlist::minimumNetCover2(const vector<unsigned int>& netWeights)
{
  const Net* const n0 = &getNet(0); // offset of array
  Cell* const c0 = &getCell(0); // offset of array

  NetList.clear();
  unsigned int i;
  vector<GDNode> va(getNumCells());
  GDNode* const va0 = &va[0];
  vector<GDNode*> vaRand(getNumCells());
  for (unsigned int i=0; i<getNumCells(); i++) {
    vaRand[i] = &va[i];     // for randomizing the order of vertices
  }
  std::random_shuffle(vaRand.begin(), vaRand.end());
  vector<unsigned int> ciList(netWeights); 
  unsigned int numNetsSelected = 0;       // count how many nets are selected

  GDList<GDNode> L;    // the list records the cells that not yet covered.
  for (i=0; i<getNumCells(); i++) {
    const unsigned int i_v = vaRand[i] - va0;
    Cell& c = c0[i_v];
    //xxx Cell& c = getCell(i);
    if (c.isFixed()) {
      c.setVisited();
      continue;
    }
    c.clrVisited();
    L.pushBack(*vaRand[i]);
    //xxx L.pushFront(va[i]);  // get non-cluster cells first
  }

  clrVisitedAllNets();

  // This while loop perform the hyperedge cover problem by the
  // primal-dual algorithm. In this version, vertices are selected
  // randomly. 
  while (!L.isEmpty()) {
    GDNode& v = L.popFront();
    const unsigned int i_v = &v - va0;
    Cell& c = getCell(i_v);
    c.setVisited();

    // Look for the net that is not yet selected with minimun ci
    Net* minNet = 0;
    unsigned int degree = 0;
    unsigned int minWeight = UINT_MAX;
    NetCursor nc(c);
    while (nc.hasNext()) {
      Net& e = nc.getNext();
      // if (e.isVisited()) continue;
      if (e.isFixed()) continue;
      const unsigned int eid = &e - n0;
      IdVec[degree++] = eid;            // cache the neighbor nets
      unsigned int weight = ciList[eid];
      if (minWeight > weight) {
        minNet = &e;
        minWeight = weight;
      }
    }
    assert(degree <= 32768);

    if (minNet == 0) continue; // a cell with no net!
    if (!minNet->isVisited()) {
      minNet->setVisited();
      numNetsSelected++;
      NetList.push_back(minNet);

      // Remove the covered cells from the list ???
      CellCursor cc(*minNet);
      while (cc.hasNext()) {
        Cell& v = cc.getNext();
        if (v.isVisited()) continue;
        v.setVisited();
        const unsigned int i_v = &v - c0;
        L.detach(va0[i_v]);
      }
    }

    for (unsigned int idx=0; idx < degree; idx++) {
      ciList[IdVec[idx]] -= minWeight;
    }
  }

  assert(minimumNetCoverOK());  
  return numNetsSelected;
}




/** @return true if the cell c is isolated */
inline bool isIsolated(const Cell& c)
{
  NetConstCursor nc(c);
  while (nc.hasNext()) {
    const Net& e = nc.getNext();
    if (!e.isFixed()) return false;
  }
  return true;
}


/** Verify if minimumNetCover() is OK */
bool Netlist::minimumNetCoverOK() const
{
  // Check if each cell belongs to some nets
  for (unsigned int i=0; i<getNumCells(); i++) {
    const Cell& c = getCell(i);
    if (c.isFixed()) continue;
    if (isIsolated(c)) continue;
    if (countVisitedNeighborNets(c) == 0) {
      cerr << "Error (cell not covered) A" << endl;
      return false;
    }
  }
  return true;
}




// Select nets in random order. 
unsigned int Netlist::randomChoice()
{
  // Setup
  NetList.clear(); // put the selected nets in NetList for contraction step.
  vector<Net*> netArray(getNumNets()); // for bucket sorting
  for (unsigned int i=0; i<getNumNets(); ++i) {
    netArray[i] = &getNet(i);
  }

  // I found that the gnu-linux STL uses lrand48() instead of standard
  // rand() for implementing "random_shuffle()". Even I used seed48()
  // to set the seed, it seems not working unless -g compiler option
  // is set. How to deal with this?
  std::random_shuffle(netArray.begin(), netArray.end());

  unsigned int numOfSelectedNets = 0;
  for (unsigned int i=0; i<getNumNets(); ++i) {
    Net* ePtr = netArray[i];
    if (ePtr->isFixed()) {
      ePtr->clrVisited();
      continue;
    }
    NetList.push_front(ePtr); 
    ePtr->setVisited();
    numOfSelectedNets++;
  }
  return numOfSelectedNets;
}

  

/** Project the result to the coarser level (i.e. current level). Note
    that this function is valid after calling the contraction
    functions such as contractByMMC(), AND before calling the
    contractPhase2() function. */
void Netlist::projectUp(const Netlist& masterNl,
                        vector<unsigned char>& partOut, 
                        const vector<unsigned char>& partIn) const
{
  const Cell* const mc0 = &masterNl.getCell(0); // offset of array
  for (int i=getNumCells()-1; i>=0; i--) {
    const Cell& c = getCell(i);
    if (c.isCluster() && !c.isFixed()) {
      const Net& e = c.firstAdjPin()->getNet();
      CellConstCursor cc(e);
      const Cell* aCellPtr = 0;
      while (1) {
        aCellPtr = &cc.getNext();
        if (masterNl._masterList[aCellPtr - mc0] == &c) {
          break;
	}
      }
      partOut[i] = partIn[aCellPtr - mc0];
    } else {
      const Cell& aCell = c.firstAdjPin()->getCell();
      partOut[i] = partIn[&aCell - mc0];
    }
  }
}


/** Project the vertex value from the coarser hypergraph. 
    The current netlist is a finer hypergraph */
void Netlist::projectDown(const Netlist& sG,
                          const vector<unsigned char>& partIn, 
                          vector<unsigned char>& partOut) const
{
  const Cell* const sc0 = &sG.getCell(0); // offset of array
  for (unsigned int i=0; i<getNumCells(); i++) {
    const Cell& c = getCell(i);
    if (c.isFixed()) continue;
    partOut[i] = partIn[_masterList[i] - sc0];
  }
}



/** Construct a coarsen graph based on Maximal-Matching
    Clustering. Note that it will not contract interface nets (marked as
    FIXED). Please call contractPhase2() after calling this function
    for complete contraction. */ 
Netlist::SPN Netlist::contractByMMC(const vector<unsigned char>& part)
{
  const unsigned int m = getNumNets();
  vector<unsigned int> netWeights(m);
  // pinCount(netWeights);
  clusterSize(netWeights);
  // ExternalPins(netWeights);
  const unsigned int numNetsSelected = minimumNetCover(netWeights);
  return contractCore(numNetsSelected, part);
}


/** Construct a coarsen graph based on Maximal-Matching
    Clustering. Note that it will not contract interface nets (marked as
    FIXED). Please call contractPhase2() after calling this function
    for complete contraction. */ 
Netlist::SPN Netlist::contractByMMC2(const vector<unsigned char>& part)
{
  const unsigned int m = getNumNets();
  vector<unsigned int> netWeights(m);
  // pinCount(netWeights);
  clusterSize(netWeights);
  // ExternalPins(netWeights);
  const unsigned int numNetsSelected = minimumNetCover2(netWeights);
  return contractCore(numNetsSelected, part);
}



/** Construct a coarsen graph based on Independent-Set
    Clustering. Note that it will not contract interface nets (marked
    as FIXED). Please call contractPhase2() after calling this function
    for complete contraction. */
Netlist::SPN Netlist::contractByISC(const vector<unsigned char>& part)
{
  const unsigned int m = getNumNets();
  vector<unsigned int> netWeights(m);
  // pinCount(netWeights);
  clusterSize(netWeights);
  // ExternalPins(netWeights);
  const unsigned int numNetsSelected = maximumIndependentSet(netWeights);
  return contractCore(numNetsSelected, part);
}


/** Construct a coarsen graph based on First-Choice Clustering. Note
    that it will not contract interface nets (marked as FIXED). Please
    call contractPhase2() after calling this function for complete
    contraction. */
Netlist::SPN Netlist::contractByFC(const vector<unsigned char>& part)
{
  const unsigned int m = getNumNets();
  vector<unsigned int> netWeights(m);

  // pinCount(netWeights);
  // clusterSize(netWeights);
  // ExternalPins(netWeights);
  contractibility(netWeights);

  const unsigned int numNetsSelected = firstChoice(netWeights);
  return contractCore(numNetsSelected, part);
}


/** Construct a coarsen graph based on random Clustering. Note that it
    will not contract interface nets (marked as FIXED). Please call
    contractPhase2() after calling this function for complete contraction. */
Netlist::SPN Netlist::contractByRandom(const vector<unsigned char>& part)
{
  const unsigned int numNetsSelected = randomChoice();
  return contractCore(numNetsSelected, part);
}


/** Mark cells unfixed according to the proximity level. */
void Netlist::setProxCells(int level)
{
  clrVisitedAllCells();
  clrVisitedAllNets();
  unsigned int i=0;
  for (; i<getNumCells(); i++) {
    Cell& c = getCell(i);
    if (c.isFixed()) continue;
    c.setVisited();
    setProxCellsRecur(c, level);
  }
  for (i=0; i<getNumCells(); i++) {
    Cell& c = getCell(i);
    if (c.isVisited()) c.clrFixed();
  }
}


void Netlist::setProxCellsRecur(Cell& c, int level)
{
  if (level == 0) return;
  NetCursor nc(c);
  while (nc.hasNext()) {
    Net& e = nc.getNext();
    if (e.isVisited()) continue;
    e.setVisited();
    CellCursor cc(e);
    while (cc.hasNext()) {
      Cell& v = cc.getNext();
      if (v.isVisited()) continue;
      v.setVisited();
      setProxCellsRecur(v, level-1);
    }
  }
}

inline void Netlist::setFixedCells()
{
  // Fixed Cell will not be selected
  unsigned int i=0;
  for (; i<getNumNets(); i++) {
    Net& e = getNet(i);
    if (e.isFixed()) continue;  // skip if it is an interface net
    // Mark this net as FIXED if it connects only to fixed cells.
    e.setFixed();
    CellCursor cc(e);
    while (cc.hasNext()) {
      Cell& c = cc.getNext();
      if (!c.isFixed()) {
        e.clrFixed();
        break;
      }
    }
  }
}



/** Mark a net as fixed if it is a high-fanout net. Otherwise, mark it as 
    unfixed. @return the number of such nets. */
int Netlist::markHighFanOutNets()
{
  int res = 0;
  for (unsigned int i=0; i<getNumNets(); i++) {
    Net& e = getNet(i);
    if (!e.isHighFanout()) e.clrFixed();
    else {
      e.setFixed();
      res++;
    }
  }
  return res;
}


/** Mark a net as fixed if it does not connect to any unvisited cells */
void Netlist::markZeroPinNets()
{
  for (unsigned int i=0; i<getNumNets(); i++) {
    Net& e = getNet(i);
    if (e.isFixed()) continue;
    CellConstCursor cc(e);
    bool zero = true;
    while (cc.hasNext()) {
      const Cell& c =  cc.getNext();
      if (!c.isVisited()) {
        zero = false;
        break;
      }
    }
    if (zero) e.setFixed();
  }
}


/** Mark a net as fixed if it is an interface net. Otherwise, mark it as 
    unfixed. */
void Netlist::setFixedNets(const vector<unsigned char>& part)
{
  const Cell* const c0 = &getCell(0); // offset of array
  // Only cells in same partition can be grouped
  for (unsigned int i=0; i<getNumNets(); i++) {
    Net& e = getNet(i);
    e.clrFixed();
    CellCursor cc(e);
    assert(cc.hasNext());
    Cell& c = cc.getNext();
    const unsigned char firstPart = part[&c - c0];
    while (cc.hasNext()) {
      Cell& aCell = cc.getNext();
      if (part[&aCell - c0] != firstPart) {
        e.setFixed();
        break;
      }
    }    
  }
}



/** Main Contraction. Todo: make the function shorter */
Netlist::SPN Netlist::contractCore(unsigned int numNetsSelected,
                                   const vector<unsigned char>& part)
{
  if (numNetsSelected < 3) return Netlist::SPN(); // return null pointer

  //??? // How many internal pins and isolated cells?
  //??? unsigned int numOfInternalPins = 0;
  //??? unsigned int numOfIsolatedCells = 0;
  //??? 
  //??? clrVisitedAllCells();
  //??? 
  //??? unsigned int i=0;
  //??? for (; i<getNumNets(); i++) {
  //???   Net& e = getNet(i);
  //???   if (e.isVisited()) {
  //???     CellCursor cc(e);
  //???     while (cc.hasNext()) {
  //???       Cell& c = cc.getNext();
  //???       c.setVisited();
  //???       numOfInternalPins++;
  //???     }
  //???   }
  //??? }
  //??? for (i=0; i<getNumCells(); i++) {
  //???   const Cell& c = getCell(i);
  //???   if (!c.isVisited()) numOfIsolatedCells++;
  //??? }

  // Allocate memory for subgraph  
  Netlist::SPN sGPtr = Netlist::SPN(new Netlist);
  Netlist& sG = *sGPtr;
  //??? sG._numCells = numNetsSelected + numOfIsolatedCells; // approximately
  //??? sG._numNets = getNumNets() - numNetsSelected; // approximately
  sG._numCells = getNumCells();
  sG._numNets = getNumNets();

  //$$$ sG._masterList.resize(sG.getNumCells());
  sG._cellList.resize(sG.getNumCells());
  _masterList.resize(getNumCells());
  constructCells(sG, part);

  sG._netList.resize(sG.getNumNets() + 1); //### plus one dummy net at the end
  constructNets(sG);
  return sGPtr;
}


/** Complete the contraction process */
void Netlist::contractPhase2(Netlist::SPN sGPtr)
{
  vector<unsigned short> degList(sGPtr->getNumCells());
  sGPtr->constructPins(*this, degList);
  sGPtr->constructDegree(degList);
  sGPtr->_totalWeight = getTotalWeight(); // toal weights are the same
  sGPtr->_costModel = getCostModel();
  sGPtr->_hasFixedCells = _hasFixedCells;
  assert(sGPtr->checkOk());
}


// Used by subGraph
inline void Netlist::constructDegree(const vector<unsigned short>& degList)
{
  unsigned int maxDegree = 0;  // Calculate the maximun degree 
  for (unsigned int i=0; i<getNumCells(); i++) {
    const Cell& sc = getCell(i);
    if (sc.isFixed()) continue; // not count in fixed cells
    if (maxDegree < degList[i]) {
      maxDegree = degList[i];
    }
  }
  _maxDegree = maxDegree;
}



/** Construct cells from selected nets and then other nets */
inline void Netlist::constructCells(Netlist& sG, 
                                    const vector<unsigned char>& part)
{
  clrVisitedAllCells();

  unsigned int cellCount = 0;
  if (hasFixedCells()) {
    constructClustersFromFixedCells(sG, part, cellCount);
  }
  constructClustersFromOtherNets(sG, cellCount);
  constructIsolatedCells(sG, cellCount);

  assert(sG.getNumCells() >= cellCount);
  sG._numCells = cellCount;
  sG._padOffset = sG.getNumCells() - 1; // no pad
  sG._cellList.resize(cellCount);
  //$$$ sG._masterList.resize(cellCount);
}



/** Construct clusters for Fixed Cells. Fixed cells in the same
    partition are grouped into one single cluster. They will be marked
    as "visited" after the grouping. */
inline void 
Netlist::constructClustersFromFixedCells(Netlist& subGraph,
                                         const vector<unsigned char>& part,
                                         unsigned int& cellCount)
{
  //xxx const Cell* const mc0 = &subGraph.getCell(0); // offset of array
  vector<Cell*> fcVector(256); // each partition has one fixed cluster cell 
  for (unsigned int i_v=0; i_v<getNumCells(); i_v++) {
    Cell& v = getCell(i_v);
    if (!v.isFixed()) continue;
    Cell* mc = 0;
    if (fcVector[part[i_v]] != 0) { // add "v" to the existing cluster
      mc = fcVector[part[i_v]]; 
      _masterList[i_v] = mc;
      mc->_weight += v._weight;
    } else { // allocate one new cluster
      mc = &subGraph._cellList[cellCount];
      *mc = v; //$$$ watch out! They have the same firstAdjPin
      assert(mc->_firstAdjPin != 0);
      _masterList[i_v] = mc;
      fcVector[part[i_v]] = mc;
      cellCount++;
    }
    assert((int) mc->_weight <= FMLimits::maxCellWeight());
    assert(!v.isVisited());
    v.setVisited();
  }
}


/** Construct clusters from selected (marked as "visited") nets. */
inline void Netlist::constructClustersFromOtherNets(Netlist& subGraph,
                                                    unsigned int& cellCount)
{
  const Cell* const c0 = &getCell(0); // offset of array
  //$$$ const Net* const n0 = &getNet(0); // offset of array

  unsigned int reduceCount = 0;
  const unsigned int maxReduceCount(lrint(getNumCells()*0.3));
  // const unsigned int maxReduceCount = getNumCells();
  list<Net*>::iterator it;
  for (it=NetList.begin(); it !=NetList.end(); ++it) {
    Net& e = *(*it);
    if (reduceCount >= maxReduceCount) { // enough contraction
      e.clrVisited();
      continue;
    }
    unsigned int ccount = 0;
    CellCursor cc(e);
    bool isInternalNet = true;
    while (cc.hasNext()) {
      Cell& aCell = cc.getNext();
      if (aCell.isVisited() || aCell.isFixed()) {
        isInternalNet = false; // this may not be an internal net
        continue;
      }
      CellVec[ccount] = &aCell;
      ccount++;
      //xxx if (ccount >= 5) break;  // not larger than 5 cells in a cluster
    }
    assert(ccount <= 32768);
    if (ccount < 2 || ccount > 100) {
      e.clrVisited();
      //??? subGraph._numNets++;
      continue;
    }
    if (!isInternalNet) {
      e.clrVisited();
      //??? subGraph._numNets++;
    }
      
    // Cluster
    Cell& sc = subGraph._cellList[cellCount];
    sc._type = Cell::CLUSTER;
    //$$$ subGraph._masterList[cellCount] = &e - n0;
    sc._firstAdjPin = e._firstAdjPin; //$$$ watch out!
    assert(sc._firstAdjPin != 0);

    sc._weight = 0;
    for (unsigned int idx=0; idx<ccount; idx++) {
      Cell& aCell = *CellVec[idx]; 
      aCell.setVisited();
      _masterList[&aCell - c0] = &sc;
      sc._weight += aCell._weight;
      ++reduceCount;
      assert((int) sc._weight <= FMLimits::maxCellWeight());
    }
    --reduceCount;
    ++cellCount;
  }

  NetList.clear();
}


/** Construct isolated cells. */
inline void Netlist::constructIsolatedCells(Netlist& subGraph, 
                                            unsigned int& cellCount)
{
  for (unsigned int i=0; i<getNumCells(); i++) {
    Cell& c = getCell(i);
    if (c.isVisited()) continue;
    Cell& sc = subGraph._cellList[cellCount];
    //$$$ subGraph._masterList[cellCount] = i;
    sc._firstAdjPin = c._firstAdjPin; //$$$ watch out!
    sc._type = Cell::CELL; // type is CELL even "c" is a cluster
    sc._weight = c._weight;
    sc._isFixed = c._isFixed; // only in isolated cells
    _masterList[i] = &sc;
    cellCount++;
  }
}


// Prune out the internal net
inline void Netlist::pruneInternalNets(Netlist& )
{
  const Cell* const c0 = &getCell(0); // offset of array
  //xxx Cell* const sc0 = &sG.getCell(0); // offset of array
  for (unsigned int i=0; i<getNumNets(); i++) {
    Net& e = getNet(i);
    if (e.isVisited()) {
      e.setFixed(); // don't create this net
      e.clrVisited(); // clear for later use
      continue;
    }
    if (e.isFixed()) continue;
    CellCursor cc(e);
    Cell& aCell = cc.getNext();
    const Cell* sc = _masterList[&aCell - c0];
    e.setFixed();
    while (cc.hasNext()) {
      Cell& aCell2 = cc.getNext();
      const Cell* sc2 = _masterList[&aCell2 - c0];
      if (sc != sc2) { // e is interface net
        e.clrFixed();
        break;
      }
    }
  }
}


// Prune out the identical nets
inline unsigned int Netlist::pruneIdenticalNets(Netlist& sG)
{
  const Cell* const c0 = &getCell(0); // offset of array
  //xxx Cell* const sc0 = &sG.getCell(0); // offset of array

  // Prune out one-pin nets and identical two-pin nets
  // and identical three-pin nets
  slist<pair<Net*, Cell*> >  L2; // for 2-pin nets
  slist<pair<Net*, pair<Cell*,Cell*> > >  L3; // for 3-pin nets
  unsigned int netCount = 0;
  unsigned int i;

  for (i=0; i<sG.getNumCells(); i++) {
    Cell& sc = sG.getCell(i);
    if (!sc.isCluster() || sc.isFixed()) continue;

    L2.clear();
    L3.clear();
    
    //$$$ Net& me = getNet(sG._masterList[i]);
    Net& me = sc.firstAdjPin()->getNet(); //$$$ watch out
    CellCursor cc(me);
    unsigned int degree = 0;
    while (cc.hasNext()) {
      Cell& c = cc.getNext();
      if (_masterList[&c - c0] != &sc) continue;
      CellVec[degree++] = &c;
      NetCursor nnc(c);
      while (nnc.hasNext()) {
        Net& ne = nnc.getNext();
        if (ne.isVisited() || ne.isFixed()) continue;
        ne.setVisited();
        CellCursor ncc(ne);
        unsigned int pcount = 1;
        Cell* cell2 = 0;
        Cell* cell3 = 0;
        while (ncc.hasNext()) {
          Cell& aCell = ncc.getNext();
          Cell* aSC = _masterList[&aCell - c0];
          if (aSC != &sc) {
            if (cell2 == 0) {
              cell2 = aSC;
              pcount++;
            }
            else if (aSC != cell2) {
              if (cell3 == 0) {
                cell3 = aSC;
                pcount++;
              }
              else if (aSC != cell3) {
                pcount++; // more than three
                break;
              }
            }
          }
        }
        if (pcount > 3) continue; // not handle more than 3 pins

        if (pcount == 2) {
          // Check parallel two-pin net
          slist<pair<Net*, Cell*> >::iterator it;
          for (it=L2.begin(); it !=L2.end(); ++it) {
            Cell* aCell = (*it).second;
            if (aCell == cell2) {
              // A two-pin net that identical to the one in the list
              ne.setFixed();
              (*it).first->_weight += ne._weight;
              assert((*it).first->_weight >= ne._weight); // no overflow
              break;
            }
          }
          if (!ne.isFixed()) {
            Net& se = sG._netList[netCount];
            se = ne;
            netCount++;
            L2.push_front(make_pair(&se, cell2));
            ne.setFixed();
          }
        }
        else if (pcount == 3 && needPrune3PinNets()) { 
          // Check parallel three-pin net
          // Sort in acsending order
          if (cell3 > cell2) { // pointer comparison
            // swap
            Cell* temp = cell3;
            cell3 = cell2;
            cell2 = temp;
          }
    	
          slist<pair<Net*,pair<Cell*,Cell*> > >::iterator it2;
          for (it2 = L3.begin(); it2 != L3.end(); ++it2) {
            pair<Cell*, Cell*> cellPair = (*it2).second;
            if (cellPair.first == cell2 && cellPair.second == cell3) {
              ne.setFixed();
              (*it2).first->_weight += ne._weight;
              assert((*it2).first->_weight >= ne._weight); // no overflow
              break;
            }
          }
          if (!ne.isFixed()) {
            Net& se = sG._netList[netCount];
            se = ne;
            netCount++;
            L3.push_front(make_pair(&se, make_pair(cell2, cell3)));
            ne.setFixed();
          }
        }
        else { // prune out one-pin net and zero-pin net if any
          ne.setFixed();
        }
      }
    }
    assert(degree <= 32768);
    // Unmark
    for (unsigned int idx=0; idx<degree; idx++) {
      Cell& c = *CellVec[idx];
      NetCursor nnc(c);
      while (nnc.hasNext()) {
        nnc.getNext().clrVisited();
      }
    }
  }

  return netCount;
}
/* Remarks: currently, we merge the "parallel" two-pins nets into a
   single net. The weights of nets are added up. The purpose is to
   reduce the size of the coarse graph. However, we didn't take care
   of the pin directions. Two nets are defined to be parallel only if they
   are connected to the same set of cells, even their pins directions
   may be different. It is OK because currently the objective is the
   cut size. If it is not the case, then additional check will be needed.
   In the above implementation, we also merge the "parallel" three-pin
   nets. However, the code for detecting the three-pin nets is quite
   complicated. The performance gain on the other hand is not
   significant for the benchmark circuits. If one who finds hard to
   maintain the code may consider to remove it.
 */    


/** Construct contraction nets */
inline void Netlist::constructNets(Netlist& sG)
{
  markHighFanOutNets(); // skip the high-fanout nets
  // clrFixedAllNets();
  pruneInternalNets(sG);
  unsigned int netCount = pruneIdenticalNets(sG);

  // Construct net if it is not a pure internal net of a cluster
  for (unsigned int i=0; i<getNumNets(); i++) {
    Net& e = getNet(i);
    if (e.isFixed()) continue;
    Net& se = sG._netList[netCount];
    se = e; // watch out! se._firstAdjPin is set to the same as e's  
    netCount++;
  }

  assert(sG.getNumNets() >= netCount);
  sG._numNets = netCount;
  sG._netList.resize(netCount);
}


// Used by subGraph
inline void Netlist::
constructPins(Netlist& masterGraph, // the master/orginal netlist
              vector<unsigned short>& degList)
{
  _numPins = masterGraph._numPins;
  _pinList.resize(_numPins);

  // Construct pins
  unsigned int pinCount = 0;
  const Cell* const c0 = &masterGraph.getCell(0); // offset of array
  Cell* const mc0 = &getCell(0); // offset of array
  unsigned int i_e, i_v, idx;

  //$$$ Reset Cell first Adjacent pin
  for (i_v=0; i_v<getNumCells(); ++i_v) {
    Cell& sc = getCell(i_v);
    sc.resetFirstAdjPin();
  }

  for (i_e=0; i_e<getNumNets(); ++i_e) {
    Net& se = getNet(i_e);
    CellCursor cc(se.firstAdjPin()->getNet()); // watch out!
    se._firstAdjPin = &_pinList[pinCount]; //###
    unsigned int degree = 0;
    while (cc.hasNext()) {
      Cell& aCell = cc.getNext();
      Cell& sc = *(masterGraph._masterList[&aCell - c0]);
      if (sc.isVisited()) continue; // prevent multiple pins ...
      sc.setVisited();              // ... to the same cell.
      CellVec[degree++] = &sc;
      Pin& p = _pinList[pinCount];
      // p.setDirection(pin.getDirection());
      Netlist::connect(p, sc, se);
      pinCount++;
      if (sc.isFixed()) continue;
      degList[&sc - mc0] += se._weight;
      assert(degList[&sc - mc0] >= se._weight); // no overflow
    }
    assert(degree <= 32768);
    for (idx=0; idx<degree; ++idx) CellVec[idx]->clrVisited();
  }

  assert(_numPins >= pinCount);
  _numPins = pinCount; // some pins allocated but not used!!!
  _pinList.resize(pinCount);

  //### add one dummy net
  _netList[_numNets]._firstAdjPin = &_pinList[pinCount];
}



/**
 * Sort the vertices by weight in descending order. Return result
 * to the list L. If the kinds of weights are small (because the cells
 * are from only a few master copies), then the function runs in
 * nearly linear time. The worst case is O(N logN) time.
 */
void Netlist::initWeightList(GDList<Vertex>& L)
{
  typedef boost::shared_ptr<GDList<Vertex> > SPL;
  typedef std::map<int, SPL, std::greater<int> > WeightCellMap;  
  WeightCellMap aMap;

  // We choose to use a MAP of lists with key equals the cell weights
  // because we assume that many cells have the same weights (mostly they
  // come from the same master copies). It should take nearly linear
  // time for the sorting.

  vector<unsigned int> vaRand(getNumCells());
  for (unsigned int i=0; i<getNumCells(); i++) {
    vaRand[i] = i;     // for randomizing the order of vertices
  }
  std::random_shuffle(vaRand.begin(), vaRand.end());

  for (unsigned int i=0; i<getNumCells(); ++i) {
    const unsigned int i_v = vaRand[i];
    const Cell& v = getCell(i_v);
    if (v.isFixed()) continue;
    SPL& ptr = aMap[v.getWeight()];
    if (ptr == 0) ptr = SPL(new GDList<Vertex>);
    // We add the cells with same weight to a list in random order to
    // increase the randomness
    ptr->pushBack(getVertex(i_v));
  }

  // Sort the cells in descending order according to their weights
  // (area) and put the result to L.
  WeightCellMap::iterator pos;
  for (pos=aMap.begin(); pos!=aMap.end(); ++pos) {
    L.concat(*pos->second);
  }
}


/**
 * Check if FM iteration can guarantee eventually produce legal
 * solution with any (include illegal) initial partitions. The bound can be
 * calculated by using a simple pigeonhole arugument. That is, if we
 * want to gurantee a sequence of single moves can eventually produce
 * legal result, at least sum of certain "small" vertices/cells has to be
 * larger than the total weight divided by the number of
 * partitions. This checking runs in O(N logN) where N is the number
 * of vertices. 
 * (Assume no fixed cells)
 */
bool Netlist::isVILEOk(double balTol, int numParts)
{
  const Vertex* const v0 = &getVertex(0);

  GDList<Vertex> L;    // a list of unfixed vertics
  initWeightList(L);   // bucket sort in non-decending order
  GDListCursor<Vertex> lc(L);

  // Divide the sorted list into two portions  
  const Cell* c = 0;
  double gap = getTotalWeight() * (1.0 + balTol) / (double) numParts;
  double weight = 0;
  while (lc.hasNext()) {
    if (weight >= gap) break; // break from the the sorted list
    const unsigned int cid = &lc.getNext() - v0;
    c = &getCell(cid);
    weight += c->getWeight();
  }

  if (c->getWeight() > getTotalWeight() * balTol) {
    return false;
  }
  return true;
}




void Netlist::pairWisePhase1(const vector<unsigned char>& part, 
                             const vector<unsigned int>& groupMap,
                             vector<Netlist*>& subGraphVec)
{
  clrVisitedAllNets();

  const unsigned int numGroups = subGraphVec.size();
  unsigned int i,j;

  const Cell* const c0 = &_cellList[0];

  // Count how many cells and nets
  vector<unsigned int> numCells(numGroups);
  vector<unsigned int> numNets(numGroups);
  vector<unsigned int> numPins(numGroups);
  for (i=0; i<getNumCells(); ++i) {
    const unsigned int group = groupMap[part[i]];
    if (group >= numGroups) continue;
    ++numCells[group];
  }

  for (i=0; i<getNumNets(); ++i) {
    Net& e = getNet(i);
    CellConstCursor ccc(e);
    assert(ccc.hasNext());
    const Cell& c1 = ccc.getNext();
    const unsigned int group1 = groupMap[part[&c1 - c0]];
    if (group1 >= numGroups) {
      e.setVisited();
      continue;
    }
    unsigned int group2 = group1;
    while (ccc.hasNext()) {
      const Cell& c2 = ccc.getNext();
      group2 = groupMap[part[&c2 - c0]];
      if (group1 != group2) break;
    }
    if (group1 == group2) ++numNets[group1];
    else e.setVisited(); // mark the net as an interface net
  }

  if (getCostModel() != 0) { // SOED or K-1 metric
    for (i=0; i<getNumNets(); ++i) {
      const Net& e = getNet(i);
      if (!e.isVisited()) continue;
      vector <unsigned int> numA(numGroups);
      CellConstCursor ccc(e);
      while (ccc.hasNext()) {
        const Cell& c = ccc.getNext();
        const unsigned int group = groupMap[part[&c - c0]];
        if (group >= numGroups) continue;
        ++numA[group];
      }
      for (j=0; j<numGroups; ++j) {
        if (numA[j] > 1) ++numNets[j];
      }
    }
  }
  for (i=0; i<getNumPins(); ++i) {
    const Pin& p = getPin(i);
    const Cell& c = p.getCell();
    const unsigned int group = groupMap[part[&c - c0]];
    if (group >= numGroups) continue;
    ++numPins[group];
  }

  vector<Cell*> sC0(numGroups);
  for (j=0; j<numGroups; ++j) {
    subGraphVec[j] = new Netlist;
    Netlist* sG = subGraphVec[j];
    sG->_cellList.resize(numCells[j]);
    sG->_netList.resize(numNets[j]+1); // plus one dummy net
    sG->_pinList.resize(numPins[j]);
    sG->_masterList.resize(numCells[j]);
    sG->_costModel = getCostModel();
    sG->_totalWeight = 0; // initially
    sC0[j] = &sG->_cellList[0];
  }

  _masterList.resize(getNumCells());

  for (i=0; i<getNumCells(); i++) {
    Cell& c = getCell(i);
    unsigned int group = groupMap[part[i]];
    if (group >= numGroups) continue;
    Netlist* sG = subGraphVec[group];
    Cell& sC = sG->_cellList[sG->_numCells];
    sC = c;
    sC.resetFirstAdjPin();
    sG->_totalWeight += sC.getWeight();
    sG->_masterList[sG->_numCells] = &c;
    _masterList[i] = &sC;
    ++sG->_numCells;
  }

  for (i=0; i<getNumNets(); ++i) {
    Net& e = getNet(i);
    if (e.isVisited()) continue; // skip interface net
    CellConstCursor ccc(e);
    assert(ccc.hasNext());
    const Cell& c = ccc.getNext();
    const unsigned int group = groupMap[part[&c - c0]];
    assert(group < numGroups);
    Netlist* sG = subGraphVec[group];
    Net& sE = sG->_netList[sG->_numNets++];
    sE = e;
    if (getCostModel() == 2) sE._weight *= 2; // SOED
    
    sE._firstAdjPin = &sG->_pinList[sG->_numPins]; //###
    CellConstCursor cc(e);
    while (cc.hasNext()) {
      const Cell& c = cc.getNext();
      Cell& sC = *_masterList[&c - c0];
      Pin& p = sG->_pinList[sG->_numPins++];
      Netlist::connect(p, sC, sE);
    }
  }

  if (getCostModel() != 0) { // handle interface net
    for (i=0; i<getNumNets(); ++i) {
      Net& e = getNet(i);
      if (!e.isVisited()) continue; // skip "internal" net

      vector <unsigned int> numA(numGroups);
      CellConstCursor ccc(e);
      while (ccc.hasNext()) {
        const Cell& c = ccc.getNext();
        const unsigned int group = groupMap[part[&c - c0]];
        if (group >= numGroups) continue;
        ++numA[group];
      }
      for (j=0; j<numGroups; ++j) {
        if (numA[j] > 1) {
          Netlist* sG = subGraphVec[j];
          Net& sE = sG->_netList[sG->_numNets++];
          sE = e;
          sE._firstAdjPin = &sG->_pinList[sG->_numPins]; //###
        }
      }
      
      CellConstCursor cc(e);
      while (cc.hasNext()) {
        const Cell& c = cc.getNext();
        const unsigned int group = groupMap[part[&c - c0]];
        if (group >= numGroups) continue;
        if (numA[group] < 2) continue;
        Netlist* sG = subGraphVec[group];
        Net& sE = sG->_netList[sG->_numNets - 1];
        Cell& sC = *_masterList[&c - c0];
        Pin& p = sG->_pinList[sG->_numPins++];
        Netlist::connect(p, sC, sE);
      }
    }
  }

  // Add one dummy net
  for (j=0; j<numGroups; ++j) {
    Netlist* sG = subGraphVec[j];
    Net& sE = sG->_netList[sG->_numNets];
    sE._firstAdjPin = &sG->_pinList[sG->_numPins];
  }

  // Calc. degList
  for (j=0; j<numGroups; ++j) {
    Netlist* sG = subGraphVec[j];
    const unsigned int n = sG->getNumCells();
    unsigned int maxDegree = 0;
    for (i=0; i<n; ++i) {
      const Cell& sC = sG->getCell(i);
      if (sC.isFixed()) continue;
      unsigned int degree = 0;
      NetConstCursor ncc(sC);
      while (ncc.hasNext()) {
        const Net& e = ncc.getNext();
        degree += e.getWeight();
      }
      if (maxDegree < degree) maxDegree = degree;
    }
    sG->_maxDegree = maxDegree;
    assert(sG->checkOk());
  }
}


//xxx void Netlist::pairWisePhase2(const vector<unsigned char>& part, 
//xxx                              const vector<unsigned int>& groupMap,
//xxx                              vector<Netlist*>& subGraphVec)
//xxx {
//xxx   const unsigned int numGroups = subGraphVec.size();
//xxx   unsigned int i,j;
//xxx 
//xxx   const Cell* const c0 = &_cellList[0];
//xxx 
//xxx }
