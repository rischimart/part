#ifndef FM_PART_TMPL4_HPP
#define FM_PART_TMPL4_HPP

#include <GDList.hpp>
#include <vector>

/** Forward declaration */
class FMParam;
class Netlist;
class GNode;

/** @addtogroup partitioning_group
 *  @{
 */

/** 
 * Iterative Refinement Partitioning Template Class. This version uses
 * early stop policy. In the refinement phase of multi-level
 * partitioning framework, it is unlikely that there are many moves in
 * a single FM pass. By the experiments of benchmark testcases, 2x
 * speed up are found than the traditional method.
 *
 * @see FMPartTmpl
 *
 */
template <class FMPartCore, class GainTmpl>
class FMPartTmpl4 : public FMPartCore
{
  typedef GNode Vertex;

public:
  /** Constructor */
  FMPartTmpl4(const FMParam& param);

  /** Destructor */
  virtual ~FMPartTmpl4();

  /** A single FM run. @return the solution part and the cut cost. */
  virtual unsigned int doPartitionOne(std::vector<unsigned char>& part);

  /** For multilevel code only */
  unsigned int doPartitionOne4(std::vector<unsigned char>& part);

private:
  /** @return the number of moves */
  unsigned int getNumMoves() const { return _numOfMoves; }

  /** 
   * Setup the initial data structures such that the neighborhood
   * search can be conducted efficiently. Also setup data structures
   * for keeping track the sequence of moves. 
   */
  inline void init(const std::vector<unsigned char>& part);

  /** @return true if the move is considered as a legal move */
  inline bool isSatisfied();

  /** 
   * Find the next solution by applying a sequence of moves. The
   * sequence of moves are keeped in the list freeVertexList. The
   * solution is however not updated immediately until the function
   * update() is called. @return the total gain of the "best" next
   * solution. 
   */
  inline int findNext();

  /** 
   * Actually update the solution by re-applying the sequence of
   * moves. Also update the internal data structure by either
   * re-applying the sequence of moves or by copying, depending on
   * which way is more economic. 
   */
  inline void update(std::vector<unsigned char>& part);

  /** A single FM run. @return the solution part and the cut cost. */
  inline unsigned int doPartitionInternal(std::vector<unsigned char>& part);

private:
  GainTmpl       _gainMgr;            /**< gain manager */
  GDList<Vertex> _freeVertexList;     /**< keep track vertices that are moved */
  unsigned int   _numOfMoves;         /**< number of moves */
  std::vector<unsigned char> _tpart;  /**< temporary solution for sequences of changes */
  unsigned int   _stat[10];
};

/** @} */

#endif 
