#ifndef BI_CONSTR_MGR3_HPP
#define BI_CONSTR_MGR3_HPP

#include "BiConstrMgrBase.hpp"
#include <cstdlib>  // for abs()

/** @addtogroup group3
 *  @{
 */

/** 
 * Bi-partitioning Constraints Manager (version 3). Mainly to answer question
 * "Is a vertex allowed to move from one partition to another?". In
 * this version, we relax the constraint so that a cell can move if
 * it is not increasing the violation of the constraints. Although the
 * performance in general is better than the traditional approach, the
 * final result may not guarantee to be satisfied the constraints.
 * @see BiConstrMgr
 */
class BiConstrMgr3 : public BiConstrMgrBase
{
public:
  /**
   * @brief Check if the moved is OK.
   * @return true if the move is OK. 
   * @param cell the cell.
   * @param fromPart partition that the cell will move from.
   * @param toPart   partition that the cell will move to.
   * In this version, we relax the constraint so that a cell can move if
   * it is not increasing the violation of the constraints. See the
   * discussion of the VILE in the UCLA's paper. 
   */
  bool satisfied(const Cell& cell, 
                 unsigned int fromPart, 
                 unsigned int toPart) const
  {
    const int weight = cell.getWeight();
    const int diffToOld = _diff[toPart];
    const int diffTo = diffToOld + weight;
    const int diffFromOld = _diff[fromPart];
    const int diffFrom = diffFromOld - weight;
    if (diffTo <= _upperBound && diffFrom >= _lowerBound) return true;
    return abs(diffFrom - diffTo) < abs(diffFromOld - diffToOld);
  }
};

/** @} */

#endif
