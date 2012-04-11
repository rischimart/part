#ifndef KWAY_CONSTRAINT_MGR_HPP
#define KWAY_CONSTRAINT_MGR_HPP

#include "KWayConstrMgrBase.hpp"

/** @addtogroup group5
 *  @{
 */

/** 
 * Multi-Way Constraints Manager. Mainly to answer question
 * "Is a vertex allowed to move from one partition to another?" 
 */
class KWayConstrMgr : public KWayConstrMgrBase
{
public:
  /** Constructor */
  KWayConstrMgr(unsigned int numParts) : KWayConstrMgrBase(numParts) {}  

  /**
   * @return true if move is OK. In the current implement, only
   * need to check the cell area constraint (cell weight). In the
   * future, we may add other constraints such as #terminal on each
   * partition. 
   */
  bool satisfied(const Cell& cell, 
                 unsigned int , 
                 unsigned int toPart) const
  {
    const int weight = cell.getWeight();
    const int diffTo = _diff[toPart] + weight;
    const int diffFrom = _diff[fromPart] - weight;
    return diffFrom >= _lowerBound && diffTo <= _upperBound ;
  }
};

/** @} */

#endif
