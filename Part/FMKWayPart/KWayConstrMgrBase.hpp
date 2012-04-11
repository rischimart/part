#ifndef KWAY_CONSTRAINT_MGR_BASE_HPP
#define KWAY_CONSTRAINT_MGR_BASE_HPP

#include <Cell.hpp>
#include <FMMoveInfo.hpp>
#include <boost/array.hpp>
#include <iostream> //for debug
/** @addtogroup group5
 *  @{
 */

/**
 * Multi-Way Constraints Management Base Class. Mainly to answer question
 * "Is a vertex allowed to move from one partition to another?"
 */
class KWayConstrMgrBase
{
protected:
  static const unsigned int _M = 64;

  /** Constructor */
  KWayConstrMgrBase(unsigned int K) : _K(K), _knownSatisfiedAll(false) {}

  /** Destructor */
  ~KWayConstrMgrBase() {}

  /** @return the number of partitions. */
  unsigned int getNumPartitions() const { return _K; }

public:
  /** @return the maximum difference */
  int getMaxDiff() const;

  /** @return allowed difference */
  int getAllowDiff() const { return _upperBound - _lowerBound; }

  /** @return the allowed upper bound */
  int getUpperBound() const { return _upperBound; }

  /** @return the allowed lower bound */
  int getLowerBound() const { return _lowerBound; }

  /** Set the allowed upper bound */
  void setUpperBound(int upperBound) { _upperBound = upperBound; }

  /** Set the allowed lower bound */
  void setLowerBound(int lowerBound) { _lowerBound = lowerBound; }

  /** @return the difference between the partitions */
  boost::array<int,64>& getDiff() { return _diff; }

  /** @return the difference between the partitions */
  const boost::array<int,64>& getDiff() const { return _diff; }

  /** Set the difference between the partitions */
  void setDiff(const boost::array<int,64>& diff)
  {
    setDiff2(diff);
    _knownSatisfiedAll = false;
  }

  /** Set the difference between the partitions */
  void setDiff2(const boost::array<int,64>& diff)
  {
    for (unsigned int k=0; k<getNumPartitions(); ++k) _diff[k] = diff[k];
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
  bool satisfiedAll(const boost::array<int,64>& diff)
  {
    /* In the current implementation, once the partitions are
       legalized, they will remain legal for all later moves. */
    if (_knownSatisfiedAll) return true;

    for (unsigned int k=0; k<getNumPartitions(); ++k) {
      if (diff[k] < _lowerBound || diff[k] > _upperBound) return false;
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

  bool toOk(unsigned int k) const { std::cout << _diff[k] << " " << _upperBound << std::endl;return _diff[k] /*+ 30*/  < _upperBound; }

  bool fromOk(unsigned int k) const { return _diff[k] /*- 30*/ > _lowerBound; }

protected:
  boost::array<int,64> _diff;  ///< temporary diff during one FM pass
  unsigned int _K;             ///< number of partitions
  int _upperBound;             ///< allowed upperbound
  int _lowerBound;             ///< allowed lowerbound
  bool _knownSatisfiedAll;     ///< whether the partitions are legalized
};

/** @} */

#endif
