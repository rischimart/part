#ifndef FM_KWAY_PART_MGR_BASE_HPP
#define FM_KWAY_PART_MGR_BASE_HPP

#include "KWayConstrMgr3.hpp"
#include <FMPartMgrBase.hpp>
#include <boost/array.hpp>
#include <vector>

class FMParam;

/** @addtogroup group5
 *  @{
 */

/** 
 * FM K-Way Partitioning Manager Utility. Provide service such that
 * calculate cut cost, constraint checking etc. This class could also
 * be the base class of the multilevel k-way partitioning manager.
 */
class FMKWayPartMgrBase : public FMPartMgrBase
{
public:
  /** @return the number of partitions. */
  unsigned int getNumPartitions() const { return _K; }

  /** @return the cost model. */
  int getCostModel() const { return _cm; }

  /** (For Multilevel code only) Set diff. */
  void setDiff(const boost::array<int,64>& diff);

  void setDiff2(const boost::array<int,64>& diff);

  /** (For Multilevel code only) @return the diff structure. */
  const boost::array<int,64>& getDiff() const { return _diff; }

  /** (For Multilevel code only) @return the diff structure. */
  boost::array<int,64>& getDiff() { return _diff; }

  /** @return initial difference between the partitions. Take O(n) time. */
  void initDiff(const std::vector<unsigned char>& part);

  /** @return true if the balance condition of the solution is
      OK. Take O(n) time. */
  bool checkBalanceOk(const std::vector<unsigned char>& part);

  /** @return the cut cost according to the defined cost model. Take
      O(n) time. */
  unsigned int cutCost(const std::vector<unsigned char>& part) const;

  virtual void printDiff() const;

protected:
  static const unsigned int _M = 64;

  //xxx /** Constructor */
  //xxx FMKWayPartMgrBase(Netlist& H, unsigned int K=3, int CM=1);

  /** Constructor */
  FMKWayPartMgrBase(const FMParam& param);

  /** Destructor */
  ~FMKWayPartMgrBase() {}

  /** @return the constraint Manger */
  KWayConstrMgr3& getConstraintMgr() { return _constraintMgr; }

  /** @return the constraint Manger */
  const KWayConstrMgr3& getConstraintMgr() const { return _constraintMgr; }

  void initPart(std::vector<unsigned char>& part);

  /** Initialization after the creation of this class, but before the
      iteration (i.e. doPartitionOne()) */
  virtual void initBase();

private:
  /** Obtain initial feasible solution. In this implementation, we try
      to optimize the balance factor: the cell weights are sorting in
      descending order and alternately assgin to each partition. It is
      deterministic. (HMetis and MLPart use random
      initialization. Meta-heuristic is applied: E.g. try 100 runs
      with different initialization and choose the best run.) */
  inline void initPartBySort(std::vector<unsigned char>& part);

  /** Obtain initial feasible solution by VILE method. Take O(n) time. */
  inline void initPartByVILE(std::vector<unsigned char>& part);

  /** Obtain initial feasible solution by random method. Take O(n) time. */
  inline void initPartByRandom(std::vector<unsigned char>& part);

  /** Return the different size between parts. Take O(n) time. */
  inline void calcDiff(const std::vector<unsigned char>& part, 
                       boost::array<int,_M>& diff) const;

  inline void setDiffInternal(const boost::array<int,_M>& diff);

  inline void setBF(); 

protected:
  unsigned int      _K;             /**< number of partitions */
  int               _cm;            /**< Cost Model */
  boost::array<int,_M> _diff;       /**< diff cell weight */
  KWayConstrMgr3    _constraintMgr; /**< manage the balance constraints */
};

/** @} */
#endif 
