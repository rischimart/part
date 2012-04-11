#ifndef BI_CONSTRAINT_MGR_BASE_HPP
#define BI_CONSTRAINT_MGR_BASE_HPP

#include <Cell.hpp>
#include <FMMoveInfo.hpp>

/** @addtogroup group3
 *  @{
 */

/** 
 * Bi-partitioning Constraints Management Base Class. Mainly to answer question
 * "Is a vertex allowed to move from one partition to another?" 
 */
class BiConstrMgrBase
{
public:
  /** @return the maximum difference */
  int getMaxDiff() const 
  { 
    return _diff[0] > _diff[1] ? _diff[0] - _diff[1] : _diff[1] - _diff[0];
  }

  /** @return the allowed upper bound */
  int getUpperBound() const { return _upperBound; }

  /** @return the allowed lower bound */
  int getLowerBound() const { return _lowerBound; }

  /** Set the allowed upper bound */
  void setUpperBound(int upperBound) { _upperBound = upperBound; }

  /** Set the allowed lower bound */
  void setLowerBound(int lowerBound) { _lowerBound = lowerBound; }

  /** @return the difference between the partitions */
  int* getDiff() { return _diff; }

  /** @return the difference between the partitions */
  const int* getDiff() const { return _diff; }

  /** Set the difference between the partitions */
  void setDiff(const int* diff) 
  { 
    setDiff2(diff);
    _knownSatisfiedAll = false;
  }

  /** Set the difference between the partitions */
  void setDiff2(const int* diff) 
  { 
    _diff[0] = diff[0];
    _diff[1] = diff[1];
  }

  /** Update the difference structure when v moves */
  void updateMove(const FMMoveInfo& moveInfo)
  {
    const unsigned int weight = moveInfo._cell->getWeight();
    _diff[moveInfo._toPart] += weight;
    _diff[moveInfo._fromPart] -= weight;
  }

  /**
   * @return true if current status is OK. In the current implement, only
   * need to check the cell area constraint (cell weight). In the
   * future, we may add other constraints such as #terminal on each
   * partition. 
   */
  bool satisfiedAll(const int* diff)
  { 
    /* In the current implementation, once the partitions are
       legalized, they will remain legal for all later moves. */
    if (_knownSatisfiedAll) return true;

    for (unsigned int k=0; k<2; ++k) {
      if (diff[k] > _upperBound || diff[k] < _lowerBound) return false;
    }

    _knownSatisfiedAll = true;
    return true;
  }

  /**
   * @return true if current status is OK. In the current implement, only
   * need to check the cell area constraint (cell weight). In the
   * future, we may add other constraints such as #terminal on each
   * partition. 
   */
  bool satisfiedOK()
  { 
    return satisfiedAll();
  }

  /**
   * @return true if current status is OK. In the current implement, only
   * need to check the cell area constraint (cell weight). In the
   * future, we may add other constraints such as #terminal on each
   * partition. 
   */
  bool satisfiedAll() { return satisfiedAll(_diff); }

  void resetKnownSatisfiedAll() { _knownSatisfiedAll = false; }

protected:
  /** Constructor */
  BiConstrMgrBase() : _lowerBound(0), _knownSatisfiedAll(false) {}  

  /** Destructor */
  ~BiConstrMgrBase() {}

protected:
  int _diff[2];            ///< temporary diff during one FM pass
  int _upperBound;         ///< allowed upper bound
  int _lowerBound;         ///< allowed lower bound
  bool _knownSatisfiedAll; ///< whether the partitions are legalized
};

/** @} */

#endif
