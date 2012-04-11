#ifndef NET_LIST_HPP
#define NET_LIST_HPP

#ifndef NET_LIST_BASE_HPP
#include "NetlistBase.hpp"
#endif

#ifndef G_NODE_HPP
#include <GNode.hpp>
#endif

#include <boost/shared_ptr.hpp>
#include <vector>

template <class node> class GDList;

/** 
 * @addtogroup data_model
 * @{
 */

/** A netlist class.*/
class Netlist : public NetlistBase
{
public:
  typedef GNode Vertex;
  typedef boost::shared_ptr<Netlist> SPN;

  /** Constructor */
  Netlist();

  /** Destructor */
  ~Netlist();

  /** Reset to an empty Netlist */
  void clear();

  /** Reset to an empty master list */
  void clearMasterList();

  /** Initialize the vertexArray. */
  void initVertexArray();

  /** @return master by index. Precondition: index within array bound. */
  Cell& getMaster(unsigned int id) { return *_masterList[id]; }
 
  /** @return master by index. Precondition: index with array bound. */
  const Cell& getMaster(unsigned int id) const { return *_masterList[id]; }
   
  /** @return vertex by index. Precondition: index with array bound. */
  Vertex& getVertex(unsigned int id) { return _vertexArray[id]; }
 
  /** @return vertex by index. Precondition: index with array bound. */
  const Vertex& getVertex(unsigned int id) const { return _vertexArray[id]; }
 
  /** @return Vertex array pointer */
  Vertex* getVertexArray() { return &_vertexArray[0]; }


  /** 
   * Find a maximal matching of nets (edges) \f$E'\f$, i.e., a subset
   * \f$E' \in E\f$ 
   * such that no two nets in \f$E'\f$ shares a common cell (endpoint) and
   * every net in \f$E - E'\f$ shares at least a common cell with some
   * net in \f$E'\f$.A cell that does not connect to any nets in
   * \f$E'\f$ is called isolated cell.   
   * A net weight is the sum of cell weight here. The objective is to
   * minimize the total net weights.
   * 
   * First, we explore a similar Primal-Dual algorithm described in [1]
   * that solves the graph weighted vertex cover problem. We generalize
   * the idea of the algorithm for hypergraph. Instead of vertex cover,
   * here we find the hyperedge cover , i.e., a subset \f$E'' \in E\f$
   * such that,
   * for each cell, at least one of connected nets belongs to \f$E''\f$.
   * Then based on the solution of hyperedge cover, we perform the
   * post processing such that no two nets in \f$E'\f$ share a common cell by
   * removing nets or moving nets. The process can be performed locally.
   *
   * The overall run time is linear, assuming that the maximum degree is
   * bounded by a small value.
   *
   * @param   netWeights   net weights 
   * @return  number of nets selected
   *
   * Reference [1]: Program 2.7 in "Complexity and Approximation:
   * Combinatorial Optimization Problems and their Approximability
   * Properties.
   */
  unsigned int maximalMatchingNets(const std::vector<unsigned int>& netWeights);

  /** 
   * Verify if maximalMatchingNets() is OK. Note that isVisited flags
   * may be marked/unmarked after calling this function. 
   */
  bool maximalMatchingNetsOK();

  /** 
   * Find a minimum net (edge) cover \f$E''\f$, i.e., a subset \f$E'' \in E\f$
   * such that for each cell, at least one of connected nets belongs
   * to \f$E''\f$. A net weight is the sum of cell weight here. The
   * objective is to minimize the total net weights.
   * We explore a similar Primal-Dual algorithm described in [1]
   * that solves the graph weighted vertex cover problem. We generalize
   * the idea of the algorithm for hypergraph. Instead of vertex cover,
   * here we find the hyperedge cover. The overall run time is linear,
   * assuming that the maximum degree is bounded by a small value. The
   * solution by the Primal-Dual algorithm is guaranteed bounded by \f$K\f$
   * times the optimal solution where \f$K\f$ is the maximum degree of nets. 
   *
   * @param   netWeights   net weights 
   * @return  number of nets selected
   *
   * Reference [1]: Program 2.7 in "Complexity and Approximation:
   * Combinatorial Optimization Problems and their Approximability
   * Properties.
   */
  unsigned int minimumNetCover(const std::vector<unsigned int>& netWeights);

  /** Verify if minimumNetCover() is OK. Take O(n+m) time. */
  bool minimumNetCoverOK() const;

  /** Find a minimum net (edge) cover. This version uses Maximum
      Adjacency (MA) ordering instead of random ordering. 
      @see maximumNetCover */ 
  unsigned int minimumNetCover2(const std::vector<unsigned int>& netWeights);
  unsigned int minimumNetCover3(const std::vector<unsigned int>& netWeights);

  /** Maximize Independent Set (Nets) by greedy algorithm. 
      Take O(N logN) time. */
  unsigned int maximumIndependentSet(const std::vector<unsigned int>& netWeight);
  
  /** Verify if maximumIndependentSet() is OK. Take O(n) time. */
  bool maximumIndependentSetOK() const;

  unsigned int firstChoice(const std::vector<unsigned int>& netWeights);

  unsigned int randomChoice();

  /** Construct a coarsen graph based on the minimal matching clustering */  
  SPN contractByMMC() { return contractByMMC(std::vector<unsigned char>()); }

  /** Construct a coarsen graph based on the minimal matching clustering */  
  SPN contractByMMC2() { return contractByMMC(std::vector<unsigned char>()); }

  /** Construct a coarsen graph based on the independent set clustering */
  SPN contractByISC() { return contractByISC(std::vector<unsigned char>()); }

  /** Construct a coarsen graph based on the First-Choice clustering */
  SPN contractByFC() { return contractByFC(std::vector<unsigned char>()); }

  /** Construct a coarsen graph based on the Random clustering */
  SPN contractByRandom() { return contractByRandom(std::vector<unsigned char>()); }

  /** Construct a coarsen graph based on the minimal matching clustering */  
  SPN contractByMMC(const std::vector<unsigned char>& part);

  /** Construct a coarsen graph based on the minimal matching clustering */  
  SPN contractByMMC2(const std::vector<unsigned char>& part);

  /** Construct a coarsen graph based on the independent set clustering */
  SPN contractByISC(const std::vector<unsigned char>& part);

  /** Construct a coarsen graph based on the First-Choice clustering */
  SPN contractByFC(const std::vector<unsigned char>& part);

  /** Construct a coarsen graph based on the Random clustering */
  SPN contractByRandom(const std::vector<unsigned char>& part);

  /** @return contraction netlist based on the result of net matching */
  SPN contractCore(unsigned int numNetsSelected, 
                   const std::vector<unsigned char>& part);

  void contractPhase2(SPN subGraph);

  /** Verify if contractCore() is OK */
  bool contractCoreOK(const Netlist& subGraph) const;

  /** Create net weights by mean of the cluster size. Return the result
      to the array netWeights */
  void clusterSize(std::vector<unsigned int>& netWeights) const;

  /** Project the result to the finer level */
  void projectUp(const Netlist& masterNl, 
                 std::vector<unsigned char>& partOut, 
                 const std::vector<unsigned char>& partIn) const;

  /** Project the result to the coarser level */
  void projectDown(const Netlist& clusterNl, 
                   const std::vector<unsigned char>& partIn, 
                   std::vector<unsigned char>& partOut) const;


  /** Mark high fanout nets as FIXED. Otherwise mark as UNFIXED. Take
      O(m) time. */
  int markHighFanOutNets();

  /** Mark nets as FIXED if they do not connected to any unvisited
      cells. Take O(m) time. */
  void markZeroPinNets();

  /** Mark nets as FIXED if they are in different partitions.
      Otherwise, mark as UNFIXED. Take O(m) time. */
  void markInterfaceNets(const std::vector<unsigned char>& part);

  /** Mark nets as FIXED if they are in different partitions */
  void setFixedNets(const std::vector<unsigned char>& part);

  /** Set "Proximity level" */
  void setProxCells(int level);

  /** Sort the cells in decending order according to their 
      weight and return the result to L. */
  void initWeightList(GDList<Vertex>& L);
  
  /** @return true if this netlist can eventually guarantee provide legal
      partition by FM iteration */
  bool isVILEOk(double bf, int numParts);

  void pairWisePhase1(const std::vector<unsigned char>& part, 
                      const std::vector<unsigned int>& groupMap,
                      std::vector<Netlist*>& subGraphVec);

  void pairWisePhase2(const std::vector<unsigned char>& part, 
                      const std::vector<unsigned int>& groupMap,
                      std::vector<Netlist*>& subGraphVec);

private:
  /** Construct contraction pins */
  inline void constructDegree(const std::vector<unsigned short>& degList);

  /** Construct cluster for fixed cells */
  inline void 
  constructClustersFromFixedCells(Netlist& subGraph,
				  const std::vector<unsigned char>& part,
				  unsigned int& cellCount);

  /** Construct clusters from selected nets. @return number of clusters
      created. */
  inline void constructClustersFromSelectedNets(Netlist& subGraph,
                                                unsigned int& cellCount);

  /** Construct clusters from other nets if possible. @return number of
      clusters created. */
  inline void constructClustersFromOtherNets(Netlist& subGraph,
                                             unsigned int& cellCount);

  /** Construct isolated cells. */
  inline void constructIsolatedCells(Netlist& subGraph,
                                     unsigned int& cellCount);

  /** Construct cells from selected nets and then other nets */
  inline void constructCells(Netlist& subGraph, 
                             const std::vector<unsigned char>& part);

  /** Prune out internal nets */
  inline void pruneInternalNets(Netlist& sG);

  /** Prune out identical nets */
  inline unsigned int pruneIdenticalNets(Netlist& sG);

  /** Construct contraction nets */
  inline void constructNets(Netlist& subGraph);

  /** Construct contraction pins for nets */
  inline void constructPinsForNets(Netlist& subGraph, 
				   std::vector<unsigned short>& degList,
				   unsigned int& pinCount);

  /** Construct contraction pins */
  inline void constructPins(Netlist& subGraph, 
                            std::vector<unsigned short>& degList);

  /** Construct net weights by mean of contractibility */
  inline void contractibility(std::vector<unsigned int>& netWeights);

  /** Construct net weights by mean of its pin count */
  inline void pinCount(std::vector<unsigned int>& netWeight) const;

  /** Construct net weights by mean of its external pin after contraction */
  inline void ExternalPins(std::vector<unsigned int>& netWeight) const;

  /** Set all the neighbor nets of fixed cells be fixed */
  inline void setFixedCells();

  /** Recursive function to set the proximity level */
  void setProxCellsRecur(Cell& c, int level);

  /** @return true if need to prune out two-pin nets */
  bool needPrune2PinNets() const { return _prune2PinNets; }

  /** @return true if need to prune out three-pin nets */
  bool needPrune3PinNets() const { return _prune3PinNets; }


private:
  std::vector<Vertex>  _vertexArray;   /**< Vertex array  */
  std::vector<Cell*>   _masterList;    /**< list of index to the master cells */
  bool  _prune2PinNets;                /**< whether prune out two-pin nets */
  bool  _prune3PinNets;                /**< whether prune out three-pin nets */

  /** Note: In order to save memory, the _vertexArray and _masterList
      are not reserved memory in the constructor. Call
      initVertexArray() before it is used. */
};


/** @} */ // end of group1

#endif
