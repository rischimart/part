#ifndef FM_PART_MGR_BASE_HPP
#define FM_PART_MGR_BASE_HPP

#include <vector>

/** Forward declaration */
class Netlist;
class FMParam;
class Cell;
class Net;

/** @addtogroup partitioning_group
 *  @{
 */

/** 
 * FM Partitioning Manager Utility Base Class. Provide service such that
 * calculate cut cost, constraint checking etc. This class is for all
 * bi-partitoning and multi-way partitioning managers.
 */
class FMPartMgrBase
{
public:
  /** Destructor */
  virtual ~FMPartMgrBase() {}

  void setPValue(unsigned int pvalue) { _pvalue = pvalue; }

  void setQValue(unsigned int qvalue) { _qvalue = qvalue; }

  /** @return the cut-cost */
  virtual unsigned int cutCost(const std::vector<unsigned char>& part) const = 0;

  /** A single FM run. @return the cutcost. */
  virtual unsigned int doPartitionOne(std::vector<unsigned char>& ) = 0;

  /** Complete FM run. @return the elapsed time */
  double doPartition(std::vector<unsigned char>& part, unsigned int starts=1);

  /** Complete FM run in Qt. @return the elapsed time */
  int doPartitionQt(std::vector<unsigned char>& part, unsigned int starts=1);

  /** @return bound type */
  int getBoundType() const { return _boundType; }

  /** Set bound type */
  void setBoundType(int boundType) { _boundType = boundType; }

  /** @return the balance tolerance. */ 
  double getBalanceTol() const { return _balTol; } 

  /** Set the balance tolerance. Precondition: 0.0 < balTol < 1 */ 
  void setBalanceTol(double balTol);

  /** @return the allowed upper bound */
  int getUpperBound() const { return _upperBound; }

  /** @return the allowed lower bound */
  int getLowerBound() const { return _lowerBound; }

  /** Set the allowed upper bound */
  void setUpperBound(int upperBound) { _upperBound = upperBound; }

  /** Set the allowed lower bound */
  void setLowerBound(int lowerBound) { _lowerBound = lowerBound; }

  /** @return maxdiff */
  int getMaxDiff() const { return _maxdiff; }

  /** @return init cost */
  unsigned int getInitCost() const { return _initCost; }

  /** @return best cost recorded. Note than if a circuit contains high
      fan-out nets that are skipped during the partition process, the
      value of this variable may not be accurate. In this case, use
      Netlist::cutCost() to get the accurate information. This
      variable is used in the multiple run and the multilevel method. */
  unsigned int getBestCost() const { return _bestCost; }

  /** @return average cost. Note than if a circuit contains high
      fan-out nets that are skipped during the partition process, the
      value of this variable may not be accurate. In this case, use
      Netlist::cutCost() to get the accurate information. This
      variable is used in the multiple run and the multilevel method. */
  double getAvgCost() const { return _avgCost; }

  /** @return the difference btw two partitions in percentage of the
      total weight */
  double diff() const { return 100.0 * (double) _maxdiff /getTotalWeight(); }

  /** @return true if noInit mode is on */
  bool isNoInit() const { return _noInit; }

  /** Do not initialize partition. In "noInit" mode, initial solution
      is supplied by the callers. This mode is useful for ECO
      (Engineering Change Order) flow. It is also used in the
      refinement phase of the multilevel method. */
  void setNoInit(unsigned int cost) { _noInit = true; _initCost = cost; }

  /** @return verbosity level, quiet(-1), normal(0), more messages(> 0) */
  int getVerbosity() const { return _verbosity; }

  /** Set verbosity level, quiet(-1), normal(0), more messages(> 0) */
  void setVerbosity(int verbosity) { _verbosity = verbosity; }

  /** @return fixed parameters */
  const FMParam& getParam() const { return _param; }

  void noNeedSetHighFanoutNets() { _needSetHighFanoutNets = false; }

  void noNeedSetFixedCells() { _needSetFixedCells = false; }

  /** Allow illegal solution */
  void setAllowIllegal(bool allowIllegal) { _allowIllegal = allowIllegal; }

  /** Allow illegal solution */
  bool allowIllegal() const { return _allowIllegal; }

protected:
  /** Constructor */
  FMPartMgrBase(const FMParam& param);

  /** @return netlist */
  Netlist& getNetlist() { return _H; }

  /** @return netlist */
  const Netlist& getNetlist() const { return _H; }

  void initNetlist();

  /** Initialization before multiple runs */
  virtual void initBase() {}

  /** (For Multilevel code only) Set maxdiff */
  void setMaxDiff(int maxdiff) { _maxdiff = maxdiff; }

  /** @return the number of Cells */
  unsigned int getNumCells() const { return _numCells; }

  /** @return the number of Nets */
  unsigned int getNumNets() const { return _numNets; }

  /** @return the total weight of cells */
  unsigned int getTotalWeight() const { return _totalWeight; }

private:
  /** @return the number of partitions. */
  inline unsigned int getNumPartitions() const;

private:
  const FMParam& _param;            /**< fixed parameters */
  Netlist&       _H;                /**< netlist to be partition */
  double         _balTol;           /**< balance tolerance */
  int            _verbosity;        /**< Verbosity level, 0=default. */
  double         _avgCost;          /**< Average cut cost. */
  unsigned int   _bestCost;         /**< best cost */
  int            _maxdiff;	    /**< maximum different */
  unsigned int   _totalWeight;      /**< total cell weight */
  unsigned int   _numCells;         /**< number of cells */
  unsigned int   _numNets;          /**< number of nets */
  int            _boundType;        /**< method to set the tolerant */ 
  int            _upperBound;       /**< allowed upper bound */
  int            _lowerBound;       /**< allowed lower bound */
				    
protected:			    
  bool           _noInit;           /**< No internal init. false=default */
  unsigned int   _initCost;         /**< init cost */
  Cell* const    _cellList;         /**< cell array offset */
  Net* const     _netList;          /**< net array offset */
  bool           _needSetHighFanoutNets;
  bool           _needSetFixedCells;
  unsigned int   _pvalue;
  unsigned int   _qvalue;
  bool           _allowIllegal;     /**< whether illegal sol'n is allowed */
};

/** @} */

/** Note that virtual function may impose performance penalty (both
    run-time and memory). However, it can simplify the interface if it
    is used wisely. */
#endif 
