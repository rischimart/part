#ifndef FM_BI_PART_MGR_BASE_HPP
#define FM_BI_PART_MGR_BASE_HPP

#include "BiConstrMgr3.hpp"
#include "FMPartMgrBase.hpp"
#include <vector>

class FMParam;

/** @addtogroup group3
 *  @{
 */

/** 
 * FM Bi-partitioning Manager Utility. Provide service such that
 * calculate cut cost, constraint checking etc. This class could also
 * be the base class of the multilevel bi-partitioning manager.
 */
class FMBiPartMgrBase : public FMPartMgrBase
{
public:
  /** @return the number of partitions (always 2 for this version). */
  unsigned int getNumPartitions() const { return 2; }

  /** @return the cost model (always 0 for this version). */
  unsigned int getCostModel() const { return 0; }

  /** (For Multilevel code only) Set diff. */
  void setDiff(const int* diff);

  /** (For Multilevel code only) Set diff. */
  void setDiff2(const int* diff);

  /** (For Multilevel code only) @return the diff structure. */
  const int* getDiff() const { return _diff; }

  /** (For Multilevel code only) @return the diff structure. */
  int* getDiff() { return _diff; }

  /** @return initial difference between the partitions. Take O(n) time. */
  void initDiff(const std::vector<unsigned char>& part);

  /** @return true if the balance condition of the solution is
      OK. Take O(n) time. */
  bool checkBalanceOk(const std::vector<unsigned char>& part);

  /** @return the cutCost. Take O(n) time. */
  unsigned int cutCost(const std::vector<unsigned char>& part) const;

  virtual void printDiff() const;

protected:
  /** Constructor */
  FMBiPartMgrBase(const FMParam& param);

  /** Destructor */
  virtual ~FMBiPartMgrBase() {}

  /** @return the constraint Manger */
  BiConstrMgr3& getConstraintMgr() { return _constraintMgr; }

  /** @return the constraint Manger */
  const BiConstrMgr3& getConstraintMgr() const { return _constraintMgr; }

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

  /** Return the different size of two parts */
  inline void calcDiff(const std::vector<unsigned char>& part, int* diff) const;

  inline void setDiffInternal(const int* diff);

  inline void setBF(); 

protected:
  int           _diff[2];         /**< diff cell weight btw two parts */
  BiConstrMgr3  _constraintMgr;   /**< manage the balance constraints */
};

/** @} */
#endif 
