#ifndef CELL_HPP
#define CELL_HPP

#include "Node.hpp"
#include <FMLimits.hpp>
#include <cassert>

/** 
 * @defgroup data_model Data Modeling Group.
 * @ingroup Part
 * Data model for supporting netlist. Include Netlist, Pin, Net,
 * Cell, etc. A netlist is represented by a hypergraph $H$. A cell is
 * represented by a vertex and a net is represented by a hyperedge
 * (or simply called edge). 
 *
 * For partitioning problem, one may
 * approximate a hypergraph (and hence the netlist) by a graph using
 * the standard clique model. That is, the netlist is approximated
 * into edge weighted undirected graph $G$ with vertex set $V$, edge
 * set $E$. Edge weights are computed by the standard clique method
 * that assign $1/(|k|-1)$ of weight to each edge in clique of size
 * $|k|$, where $|k|$ is the number of cells net $k$ connects. Note
 * that if a netlist contains only two-pin nets and three-pin nets,
 * the clique model performs exactly the same as the hypergraph
 * model. The clique model may overestimate the cost of a general
 * net. Practically the clique model seems work well for general
 * netlist with respect to the quality-of-result (especially when
 * using SOED metric).  The reason may be because practically two-pin
 * nets and three-pin nets are dominated in a real circuit (there may
 * be a deeper reason. Try take out all the two-pin nets and
 * three-pin net and see).
 *
 * The clique model can simplify the operations and faster run-time is
 * expected. However, it seems that the model requests significant
 * memory. It may not be suitable for the multilevel partitioning method.
 * @{
 */

/** 
 * A cell class. In order to efficiently traverse the nets from cell, it is
 * better that this class has a direct pointers to Net instead of
 * going through the pin. It may make the FM algorithm a little faster. 
 * However, it prohibits to reuse the netlist from other algorithms
 * that require to follow the signal direction. 
 */
class Cell : public Node
{
  friend class Netlist;
  friend class NetlistBase;

public:
  /** 
   * Node type enum. 
   */
  enum Type { 
    CELL,      /**< a cell    */
    CLUSTER    /**< a cluster */
  };

  /** Constructor */
  Cell() :
    _weight(1), 
    _type(Cell::CELL),
    _isVisited(false),
    _isFixed(false),
    _isLocked(false)
  {
  }

  /** @return the type */
  Type getType() const { return _type; }

  /** Set if this cell is a cluster. */
  void setCluster() { setType(Cell::CLUSTER); }

  /** @return true if this cell in fact is a cluster. */
  bool isCluster() const { return getType() == Cell::CLUSTER; }

  /** @return true if cell is terminal pad. */
  bool isPad() const { return getWeight() == 0; } // Is this OK?

  /** @return true if node is fixed. */
  bool isFixed() const { return _isFixed; }

  /** Set fixed. */
  void setFixed() { _isFixed = true; }

  /** Clear fixed. */
  void clrFixed() { _isFixed = false; }

  /** Toggle fixed. */
  void toggleFixed() { _isFixed ^= 1; }

  /** @return true if node is visited */
  bool isVisited() const { return _isVisited; }

  /** Set visited. */
  void setVisited() const { _isVisited = true; }

  /** Clear visited. */
  void clrVisited() const { _isVisited = false; }

  /** Toggle visited. */
  void toggleVisited() { _isVisited ^= 1; }

  /** @return true if node is locked. */
  bool isLocked() const { return _isLocked; }

  /** Set locked. */
  void lock() { _isLocked = true; }

  /** Clear locked. */
  void unlock() { _isLocked = false; }

  /** @return node weight. */
  int getWeight() const { return _weight; }

  /** Set node weight. Precondition: weight non-negative (>= 0) */
  void setWeight(int weight) 
  { 
    assert(weight >= 0 && weight <= FMLimits::maxCellWeight());
    _weight = weight; 
  }

protected:
  /** Set the type */
  void setType(Type t) { _type = t; }
  
private:
  unsigned int _weight : 28;    /**< node weight (area) default is 1 */
  Type         _type : 1;       /**< type of node */
  mutable bool _isVisited : 1;  /**< mark if visited, default is false */
  bool         _isFixed : 1;    /**< mark if fixed in one partition. */
  bool         _isLocked : 1;   /**< mark if cell is locked */
};


/** @} */
#endif
