#ifndef NET_HPP
#define NET_HPP

#include "Node.hpp"
#include <FMLimits.hpp>
#include <cassert>

/** @addtogroup data_model
 *  @{
 */

/** 
 * A net class.
 * Net is represented by hyperedge. In some other implementations, net
 *is first approximated by using the clique model. If a circuit
 *contains mostly two-pin nets, this strategy may greatly simplifies
 *algorithms while it doesn't affect the result too much.
 */
class Net : public Node
{
  friend class Netlist;
  friend class NetlistBase;

public:
  /** Construtor */
  Net() :
    _weight(1), 
    _isVisited(false),
    _isFixed(false) 
  {
  }

  /** @return true if node is fixed */
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

  /** @return node weight. */
  int getWeight() const { return _weight; }

  /** Set node weight. Precondition: weight non-negative (>= 0) */
  void setWeight(int weight) 
  { 
    assert(0 <= weight && weight <= FMLimits::maxNetWeight());
    _weight = weight; 
  }

  /** @return true if this is a high fanout net */
  bool isHighFanout() const
  {
    const unsigned int pc = (this+1)->firstAdjPin() - (this)->firstAdjPin(); 
    return pc > FMLimits::maxFanOutOfNets();
  }

private:
  unsigned short  _weight;        /**< node weight (area) default is 1 */
  mutable unsigned char   _isVisited;  /**< mark if visited, default is false */
  unsigned char   _isFixed;       /**< mark if fixed, default is false */
};

/** @} */
#endif
