#ifndef CYCLE_REMOVAL_SERV_HPP
#define CYCLE_REMOVAL_SERV_HPP

#include <GDList.hpp>
#include <GNode.hpp>
#include <vector>

/**
 * @defgroup cycle_removal Cycle Removal Group.
 * @ingroup Part
 *
 *  This module contains the implementation of Cycle Removal service,
 * i.e., converting a netlist into acyclic hypergraph. Many
 * applications in EDA, such as finding the maximum delay path,
 * request the input netlist be first temporarily converted to a acyclic
 * graph. Typically, we obtain an acyclic digraph by reversing
 * the directions of some pins.
 *
 * (c.f. G. D. Battista., Graph Drawing: Algorithms for the
 * Visualization of Graphs, Pentice Hall, Section 9.4.)
 *
 *  If \f$G = (V, E\f$ is a digraph and \f$R \subset E\f$, then we
 * denote the digraph obtained by reversing all the edge of \f$R\f$ by
 * \f$G_{rev(R)}\f$. To remove cycles, we choose a set \f$R\f$ such
 * that the digraph \f$G_{rev(R)}\f$ is acyclic. The main problem is
 * how to choose \f$R\f$ so that \f$|R|\f$ is small.
 *
 *  A set \f$R\f$ of edges of a digraph \f$G = (V, E)\f$ is a
 * <i>feedback set</i>, if \f$H_{rev(R)}\f$ is acyclic. The feedback
 * set is closely related to the well-known <i>feedback arc set</i>,
 * which is defined as a set of edges whose removal makes the digraph
 * acyclic. The problem of finding a minimum cardinality feedback edge
 * set of a given digraph is equivalent to the well-known feedback arc
 * set problem. Unfortunately this problem is NP-complete. Thus
 * effective heuristics are needed.
 *
 *  Suppose that we choose an ordering \f$S = (v_1, v_2, ..., v_n)\f$
 * of the vertices of a digraph \f$G\f$. We say that \f$S\f$ is a
 * <i>vertex sequence</i> for \f$G\f$. An edge \f$(v_i, v_j)\f$ with
 * \f$i > j\f$ is called an <i>leftward edge</i> (with respect to
 * \f$S\f$). In other words, if the vertices are drawn on a horizontal
 * line in the left to right order as they appear in \f$S\f$, then the
 * leftward edges point to the left. The set of leftward edges for a
 * vertex forms a feedback set. Conversely, if \f$R\f$ is a feedback
 * set, then by computing a topological ordering of te digraph obtained
 * by reversing all edges in \f$R\f$, we obtain a vertex sequence, such
 * that \f$R\f$ contains every leftward edge. Thus the feedback set
 * problems is equivalent to finding a vertex sequence with as few
 * leftward edges as possible.
 *
 * @{
 */

/** Forward declaration */
template <class BPQueue> class GBPQueue;
class Netlist;
class Net;

/** 
 * Service for Remove Cycles of a netlist by reversing the direction
 * of some pins. The algorithm is called Greedy-Cycle-Removal, which
 * computes a vertex sequence inducing a small set of leftward
 * edges. The algorithm runs in linear time and space, and it has a better
 * guarantee on performance than a trivial method. We assume that the
 * input netlist \f$H\f$ is connected; if not, we deal with each
 * connected component individually. The algorithm successively
 * removes vertices from \f$H\f$, and adds each in turn, to one of two
 * lists \f$S_l\f$ and \f$S_r\f$. The vertices are added to either to
 * the end of of \f$S_l\f$ or to the beginning of \f$S_r\f$. When
 * \f$H\f$ has been reduced to an empty digraph by successive
 * removals, the output vertex sequence S is the concatenation of
 * \f$S_l\f$ and \f$S_r\f$. 
 *
 * The algorithm is greedy in its choice of the vertex to be removed
 * from \f$H\f$ and the choice of the list (either \f$S_l\f$ or
 * \f$S_r\f$) to which it is added. All sinks should be added to
 * \f$S_r\f$, and all sources of \f$H\f$ should be added to
 * \f$S_l\f$, since this procedure will prevent their incident edges
 * form becoming leftward edges. Note that an isolated vertex is both
 * a sink and a source. In this implementation, we regard it as a sink
 * and added to \f$S_l\f$. After dealing with all sinks and sources,
 * we choose a vertex \f$u\f$ for which \f$outdeg(u) - indeg(u)\f$ is
 * maximized. Vertex \f$u\f$ is removed and added to \f$S_l\f$. The
 * throught here is that this choice locally maximizes the number of
 * "rightward" edges for a given number of leftward edges.
 *
 */
class CycleRemovalServ
{
  typedef GNode Vertex;

protected:
  Netlist&             _H;                 ///< netlist to be handled
  std::vector<int>     _inDegreeCell;      ///< in-degree of cell
  std::vector<int>     _outDegreeCell;     ///< out-degree of cell
  std::vector<int>     _inDegreeNet;       ///< in-degree of net
  std::vector<int>     _outDegreeNet;      ///< out-degree of net
  Vertex*              _vertexArray;       ///< vertex array for bucket sort
  GDList<Vertex>       _srcQueue;          ///< event queue for source
  GDList<Vertex>       _sinkQueue;         ///< event queue for sink
  GBPQueue<Vertex>*    _bucketPQ;          ///< bounded priority queue
  GDList<Vertex>       _Sl;                ///< vertex sequence (left)
  GDList<Vertex>       _Sr;                ///< vertex sequence (right)
  int                  _numHandledCells;   ///< keep track #cells handled

  /** Initial Setup */
  void setup();

  ///@{ Update neigbours
  inline void handleSrc();
  inline void handleSink();
  inline void handleNextBest();
  inline void handleNetForSrc(Net& net);
  inline void handleNetForSink(Net& net);
  inline void assignFirstOrLastCell();
  inline void assignOtherCells();
  ///@}

  // Unimplemented
  CycleRemovalServ(const CycleRemovalServ&);
  CycleRemovalServ& operator=(const CycleRemovalServ&);
 
public:
  /// Constructor
  CycleRemovalServ(Netlist& H);

  /// Destructor
  ~CycleRemovalServ();

  /// Perform the removal algorithm.
  void doRemove();

  /// Verify only
  bool verifyOK();

  /** Select indepentent nets followed the signal flow */
  unsigned int selectIndependentNets(const std::vector<unsigned int>& netWeights);
};

/** @} */
#endif
