#ifndef NODE_HPP
#define NODE_HPP

/** Forward declaration */
class Pin;

/** @addtogroup data_model
 *  @{
 */

/** 
 * A node class. In order to efficiently traverse the nets from cell, it is
 * better that this class has a direct pointers to Net instead of
 * going through the pin. It may make the FM algorithm a little faster. 
 * However, it prohibits to reuse the netlist from other algorithms
 * that require to follow the signal direction. 
 */
class Node 
{
  friend class Netlist;
  friend class NetlistBase;

private:
  Pin* _firstAdjPin;    /**< first adjacent pin */

private:
  /** Reset first adjacency pin */
  void resetFirstAdjPin() { _firstAdjPin = 0; }

public:
  /** Constructor */
  Node() : _firstAdjPin(0) {}

  /** @return the firstAdjacency pin */
  Pin* firstAdjPin() { return _firstAdjPin; }

  /** @return the firstAdjacency pin */
  const Pin* firstAdjPin() const { return _firstAdjPin; }
};

/** @} */ // end of group1
#endif
