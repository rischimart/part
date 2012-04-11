#ifndef ML_PART_TMPL_HPP
#define ML_PART_TMPL_HPP

#include <boost/shared_ptr.hpp>
#include <Netlist.hpp>
#include <vector>
#include <stack>

class FMParam;

/** @addtogroup partitioning_group
 *  @{
 */

/** 
 * Multi-level Partitioning Template Class. The algorithm used in this
 * class can support the memory limited feature. The algorithm
 * generates sub-graphs only when memory is avaliable. Since most UNIX
 * operating systems support virtual memory, if users want to generate
 * sub-graphs for available physical memory, they may need to limit the
 * memory manually. In bash, use command 'ulimit' to do that. (Note
 * for developers: the code relies on the exception mechanism to make
 * the feature work. To avoid memory leak due to the exception, all
 * the memory allocations should be done in constructors of classes
 * and the corresponding memory deallocation should be provided in the
 * corresponding destructors. Or one should use smart pointers (such
 * as boost::shared_ptr) to automatically release the memory.
 * 
 * The performance of multilevel method may not be as good as the flat
 * methods in ECO (Engineering Change Order) flow. Especially when a
 * circuit contains many isolated unfixed cells.
 */
template <class PartMgrBase, 
          class TopPartitioner, 
          class RefinementMgr>
class MlPartTmpl : public PartMgrBase
{
  typedef Netlist::SPN 
    (Netlist::* contractMethodPtr) (const std::vector<unsigned char>& );

public:
  /** Constructor */
  MlPartTmpl(const FMParam& param);

  /** Destructor */
  ~MlPartTmpl();

  /** 
   * Single Multi-level run. Multilevel method consists of three
   * phases, namely Coarsening Phase, Initial Partition Phase (at top
   * level) and Refinement Phase. In practice, more sophisticated
   * strategies could be used here, such as V-cycle and W-cycle
   * (terminlogies borrowed from Multigrid Method in solving
   * differential equations (c.f. P. Wesseling, An Introduction to
   * Multigrid Methos, John Wiley & Sons, 1992) 
   */
  unsigned int doPartitionOne(std::vector<unsigned char>& part);

  /** Set contraction method. @see Netlist. */
  void setContractMethod(contractMethodPtr ptr) { _contractPtr = ptr; }

  /** Use VCycle method */
  void setVCycle() { _isVCycle = true; }

  void setLocalityRatio(unsigned int lr) { _localityRatio = lr; }

private:
  /** Coarsening Phase. Successively generate a sequence of coarse
      hypergraphs. The process repeats until the number of cells
      (modules) is less than or equal to the maximum pin count (degree),
      or legal initial solution cannot be generated. Contraction methods
      defined in Netlist such as Greedy and MMC could be used. */
  inline Netlist::SPN coarseningPhase(Netlist& H, 
                                      const std::vector<unsigned char>& part);

  /** Initial Partition Phase. This phase is applied to the coarsest
      (top level) hypergraph. In current implementation, the initial
      partitioning solution is simply obtained from the best of three
      runs of the FM algorithm. */ 
  void topLevelInitPhase(Netlist& H,
		                std::vector<unsigned char>& part,
                                unsigned int& cost,
			        int starts, int level);

  /** Uncoarsening and Refinement Phase. The solutions are projected to
      the finer hypergraphs and are then refined by the standard FM
      algorithm.*/
  void refinementPhase(Netlist& H, 
                              std::vector<unsigned char>& part,
                              unsigned int& cost, int level);

  /** Apply the phases recursively. */
  void doRecur(Netlist& H, 
               std::vector<unsigned char>& part, 
               unsigned int& cost, 
               int level);


protected:
  virtual void initBase() {}

private:
  contractMethodPtr   _contractPtr;   ///< contraction method
  bool _isVCycle;                     ///< V-Cycle
  bool _hasFixedCells;                ///< has Fixed Cells
  unsigned int _localityRatio;        ///< for locality estimation
  double _relaxRatio;
};

/** @} */
#endif 
