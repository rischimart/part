#ifndef FM_BI_GAIN_MGR2_HPP
#define FM_BI_GAIN_MGR2_HPP

#ifndef FM_MOVE_INFO_HPP
#include "FMMoveInfo.hpp"
#endif

#include <GBPQueue.hpp>
#include <GNode.hpp>
#include <Cell.hpp>
#include <vector>

/** Forward declaration */
class Netlist;
class FMParam;
class BiConstrMgr3;

/** 
 * @addtogroup group3
 * @{
 */

/**
 * FM Bi-partitioning Gain Manager. This class responsible for maintaining
 * the gain bucket structures and helps to select the best vertex. The
 * data structure used for selecting the next vertex to be moved is as
 * follows. The vertex gain is an (short) integer and each vertex has its
 * gain in the range -pmax to +pmax, where pmax is the maximum vertex
 * degree in the hypergraph. Since vertex gain have restricted values,
 * 'bucket' sorting can be used to maintain a sorted list of
 * vertex gains. This is done using an array BUCKET[-pmax, ..., pmax],
 * whose kth entry contains a doubly-linked list of free vertices with
 * gains currently equal to k (GBPQueue). Two such arrays are used,
 * one for each partition. Each array is maintained by moving a vertex
 * to the appropriate bucket whenever its gain changes due to the
 * movement of one of its neighbors. Direct access to each vertex,
 * from a separate field in the VERTEX array (_vertexArray), allows
 * removal of a vertex from its current list and its movement to the
 * head of its new bucket list in constant time. As only free vertices
 * are allowed to move, therefore, only their gains are updated. For
 * each BUCKET array, a _max index is maintained which is used to keep
 * track of the bucket having a vertex of highest gain. This index is
 * updated by decrementing it whenever its bucket is found to be empty
 * and reseetting it to a higher bucket whenever a vertex moves to a
 * bucket above _max. Experimental results on real circuits have shown
 * that gains tend to cluster sharply around the origin and that _max
 * moves very little, making the implementation exceptionally fast and
 * simple. 
 * Reference:
 *  N. Sherwani. Algorithms for VLSI Physical Desgin Automation, 3rd Ed.
 */
class FMBiGainMgr2
{
  typedef GNode Vertex;
  typedef GBPQueue<Vertex> VertexPQ;
  typedef GBPQueueCursor<Vertex> VertexPQCur;

public:
  /** Constructor */
  FMBiGainMgr2(const FMParam& param);

  /** Setup the gain bucket structure */
  void init(const std::vector<unsigned char>& part, const std::vector<short>& gain);

  /** Select the next "best" vertex that satisfies all constraints
      when it is moved. In this implementation, we pick two vertices
      from both partitions with the highest gains. If both has equal
      gain, the one that minimizes the balance different will be chosen. */
  Vertex* select(FMMoveInfo& moveInfo,
                 const std::vector<unsigned char>& , 
                 const BiConstrMgr3& constraintMgr)
  {
    return select(moveInfo, constraintMgr);
  }

  /** @return the next "best" vertex that satisfies all constraints
      when it is moved. Return also move from where to where. */
  Vertex* select(FMMoveInfo& moveInfo,
                 const BiConstrMgr3& constraintMgr);

  /** @return the gain of vertex if it moves */
  int getGain(const FMMoveInfo& moveInfo) const
  {
    const Vertex& v = _vertexArray[moveInfo._i_v];
    return _gainBucket[moveInfo._fromPart].getKey(v);
  }

  /** Update the moved cell in the gain bucket */
  void updateMove(const FMMoveInfo& moveInfo)
  {
    _gainBucket[moveInfo._fromPart].detach(_vertexArray[moveInfo._i_v]);
  }

  /** Update the neighbor of the moved cells due to the move. */
  void updateNeighbor(unsigned int whichPart, 
                      unsigned int i_w, short deltaGain)
  {
    // Only free vertices are allowed to moved, therefore, only their
    // gains are updated.
    if (_cellList[i_w].isLocked()) return;
    _gainBucket[whichPart].modifyKeyLIFOBy(_vertexArray[i_w], deltaGain);   
  }

  /** Push back the cell to gain bucket. Precondition: v is not in bucket. */
  void pushBack(unsigned int whichPart, const Cell& v, short gain)
  {
    _gainBucket[whichPart].pushBack(_vertexArray[&v - _cellList], gain);
  }

  /** Update the gain structure according to the freeVertexList. */
  void updateGain(const std::vector<unsigned char>& part,
                  const std::vector<short>& gain)
  {
    init(part, gain);
  }

protected:
  Netlist&       _H;                  /**< the hypergraph */
  VertexPQ       _gainBucket[2];      /**< two gain buckets */
  VertexPQCur    _gainBucketCur[2];   /**< gain bucket cursor */
  Vertex* const  _vertexArray;        /**< VERTEX array */
  Cell* const    _cellList;           /**< Cell array offset */
};

/** @} */

#endif
