#ifndef KWAY_CONSTRAINT_MGR3_HPP
#define KWAY_CONSTRAINT_MGR3_HPP

#include "KWayConstrMgrBase.hpp"
#include <FMMoveInfo.hpp>

/** @addtogroup group5
 *  @{
 */

/** 
 * Multi-Way Constraints Manager (version 3). Mainly to answer question
 * "Is a vertex allowed to move from one partition to another?" 
 * @see BiConstrMgr3.
 */
class KWayConstrMgr3 : public KWayConstrMgrBase
{
public:
  /** Constructor */
  KWayConstrMgr3(unsigned int numParts) : KWayConstrMgrBase(numParts) {}  

  /**
   * @return true if move is OK. In the current implement, only
   * need to check the cell area constraint (cell weight). In the
   * future, we may add other constraints such as #terminal on each
   * partition. 
   */
  bool satisfied(const Cell& cell, 
                 unsigned int fromPart, 
                 unsigned int toPart)
  {
    const int weight = cell.getWeight();
    const int diffToOld = _diff[toPart];
    const int diffTo = diffToOld + weight;
    const int diffFromOld = _diff[fromPart];
    const int diffFrom = diffFromOld - weight;
    /* If the move is decreasing the difference, it is satisfied */
    if (abs(diffFrom - diffTo) < abs(diffFromOld - diffToOld)) return true;
    /* Otherwise, if the move is not within the bounds, it is not satisfied. */
    if (_lowerBound > diffFrom || diffTo > _upperBound) return false; 
    /* Moreover, if the overall is not satisfied at all, the move is
       also considered as unsatisfied. */   
    if (!satisfiedAll()) return false;
    return true;
  }
};

/** @} */

#endif
