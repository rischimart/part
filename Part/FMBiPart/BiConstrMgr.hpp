#ifndef BI_CONSTR_MGR_HPP
#define BI_CONSTR_MGR_HPP

#include "BiConstrMgrBase.hpp"

/** 
 * @defgroup group3 Bi-partitioner Core Engine 
 * @ingroup partitioning_group
 *
 * This module contains the implementation of Fiduccia-Mattheyses
 * bipartitioning algorithm and its variants. Bi-partitioning problem
 * is a special case of $k$-way partitioning problem where $k$ is
 * equal to 2. When a vertex moves from 0, it implies that the vertex
 * will move to 1. Similarly, when a vertex move to 1, it implies that
 * the vertex moves from 0. Implementation can take the advantage of
 * these facts in order to make the program runs faster than the
 * general k-way implementation. Also, different cost models such as
 * (k-1) metric, SOED metric and hyperedge metric are the same in the
 * bi-partitioning problem.
 *
 * @{
 */

/** 
 * Bi-partitioning Constraints Manager. Mainly to answer question
 * "Is a vertex allowed to move from one partition to another partition?" 
 */
class BiConstrMgr : public BiConstrMgrBase
{
public:
  /**
   * @brief Check if the move satisfies the balance condition.
   * @return true if move is OK. 
   * @param cell the cell.
   * @param fromPart partition that the cell will move from.
   * @param toPart partition that the cell will move to.
   * In the current implement, only
   * need to check the cell area constraint (cell weight). In the
   * future, we may add other constraints such as #terminal on each
   * partition. 
   */
  bool satisfied(const Cell& cell, 
                 unsigned int fromPart, 
                 unsigned int toPart) const
  {
    return _diff[toPart] + weight <= _upperBound 
           && _diff[fromPart] - weight >= _lowerBound;
  }
};

/** @} */ // end of group3
#endif
