#ifndef FM_PART_TMPL_HPP
#define FM_PART_TMPL_HPP

#include <GDList.hpp>
#include <vector>

/** Forward declaration */
class FMParam;
class Netlist;
class GNode;

/** @defgroup partitioning_group Partitioning Group.
 * @ingroup Part
 * This module contains the implementation of Fiduccia-Mattheyses
 * partitioning algorithm and its variants. 
 *
 * @{
 */

/** 
 * Iterative Improvement Partitioning Template Class. <p>In this
 * partitioning method, the next solution <i>s'</i> considered after
 * solution <i>s</i> is dervied by first applying a sequence of
 * <i>t</i> changes (moves) to <i>s</i> (with <i>t</i> dependent from
 * <i>s</i> and from the specific heuristic method), thus obtaining a
 * sequence of solution <i>s,...,s_t</i> and by successively
 * choosing the best among these solutions. </p>
 * 
 * <p>In order to do that, heuristics refer to a measure of the gain (and
 * balance condition) associated to any sequence of changes performed on
 * the current solution. Moreover, the length of the sequence generated
 * is determined by evaluting a suitably defined <i>stopping rule</i> at
 * each iteration.</p>
 * 
 * <pre>
 * Reference:
 *   G. Ausiello et al., Complexity and Approximation: Combinatorial
 * Optimization Problems and Their Approximability Properties, Section 10.3.2.
 * </pre>
 */
template <class FMPartCore, class GainTmpl>
class FMPartTmpl : public FMPartCore
{
  typedef GNode Vertex;

public:
  /** Constructor. */
  FMPartTmpl(const FMParam& param);

  /** Destructor. */
  virtual ~FMPartTmpl();
  
  /** 
   * A complete single run. Initial solution is computed by this
   * function. If no feasible solution is found, an exception will be
   * thrown.  @return the cut cost.
   */
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
  inline int update(std::vector<unsigned char>& part);

  /** 
   * A complete single run. Initial solution is computed by this
   * function. If no feasible solution is found, an exception will be
   * thrown.  @return the cut cost.
   */
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
