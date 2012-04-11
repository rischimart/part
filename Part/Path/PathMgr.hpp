#ifndef PATHMGR_HPP
#define PATHMGR_HPP

#ifndef PIN_HPP
#include <Pin.hpp>
#endif

/** @addtogroup cycle_removal
 *  @{
 */

/** Forward declaration */
class Netlist;
template <class BPQNode> class GBPQueue;
class GNode;
template <class node> class GDList;

/** Handle path of netlist */
class PathMgr
{
  typedef GNode Vertex;

private:
  Netlist&          _H;               // netlist to be handled
  Pin::DIRECTION*   _origDir;         // keep the original direction of pin

public:
  /// Constructor
  PathMgr(Netlist& H);

  /// Destructor
  ~PathMgr();

  /** Remove cycles of the netlist by temporarly reversing the
      directions of pins. Take O(n) time where n is the number of
      nodes */
  void removeCycles();

  /** I forget what it is */
  void assignLayers();
};

/** @} */

#endif
