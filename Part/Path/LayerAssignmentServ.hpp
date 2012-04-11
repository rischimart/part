#ifndef LAYER_ASSIGNMENT_SERV_HPP
#define LAYER_ASSIGNMENT_SERV_HPP

#ifndef CYCLE_REMOVAL_SERV_HPP
#include "CycleRemovalServ.hpp"
#endif
#ifndef G_BPQUEUE_HPP
#include <GBPQueue.hpp>
#endif
#ifndef G_DLIST_HPP
#include <GDList.hpp>
#endif

/** @addtogroup cycle_removal
 *  @{
 */

/** Forward declaration */
class Netlist;
class Net;
class Cell;

/** Service for Remove Cycles of a netlist by reversing the direction
    of some pins */
class LayerAssignmentServ : public CycleRemovalServ
{
  typedef GNode Vertex;

private:
  int*            _layerCell;
  int*            _layerNet;

  // Unimplemented
  LayerAssignmentServ(const LayerAssignmentServ&);
  LayerAssignmentServ& operator=(const LayerAssignmentServ&);
 
public:
  /// Constructor
  LayerAssignmentServ(Netlist& H);

  /// Destructor
  ~LayerAssignmentServ();

  /// Perform the layer assignment algorithm
  void doAssignment();

  /// Write to the .dot format
  int writeDot(const char* fileName) const;
};

/** @} */
#endif
