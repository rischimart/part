#ifndef FM_BI_GAIN_MGR_HPP
#define FM_BI_GAIN_MGR_HPP

#ifndef G_BPQUEUE_HPP
#include <GBPQueue.hpp>
#endif

#ifndef G_NODE_HPP
#include <GNode.hpp>
#endif

#ifndef FM_MOVE_INFO_HPP
#include "FMMoveInfo.hpp"
#endif

#ifndef CELL_HPP
#include <Cell.hpp>
#endif

#include <vector>

/** Forward declaration */
class Netlist;
class FMParam;
class BiConstrMgr3;

/** @addtogroup group3
 *  @{
 */

/**
 * FM Bi-partitioning Gain Manager (version 1). This class responsible
 * for maintaining 
 * the gain bucket structures and helps to select the best vertex. In
 * this version, only one gain bucket is used. @see FMBiGainMgr2.
 */
class FMBiGainMgr
{
  typedef GNode Vertex;
  typedef GBPQueue<Vertex>         VertexPQ;
  typedef GBPQueueCursor<Vertex>   VertexPQCur;

public:
  /** Constructor */
  FMBiGainMgr(const FMParam& param);

  /** Setup the gain bucket structure */
  void init(const std::vector<unsigned char>& , const std::vector<short>& gain)
  {
    return init(gain);
  }

  /** Setup the gain bucket structure */
  void init(const std::vector<short>& gain);

  /** Select the next "best" vertex that satisfies all constraints
      when it is moved. */
  Vertex* select(FMMoveInfo& moveInfo,
                 const std::vector<unsigned char>& part, 
                 const BiConstrMgr3& constraintMgr);

  /** @return the gain of vertex if it moves */
  int getGain(const FMMoveInfo& moveInfo) const
  {
    return _gainBucket.getKey(_vertexArray[moveInfo._i_v]);
  }

  void updateMove(const FMMoveInfo& moveInfo)
  {
    _gainBucket.detach(_vertexArray[moveInfo._i_v]);
  }

  /** Update the gain information by deltaGain amount */
  void updateNeighbor(unsigned int, unsigned int i_w, 
                      short deltaGain)
  {
    // Only free vertices are allowed to moved, therefore, only their
    // gains are updated.
    //&&& if (_cellList[i_w].isVisited()) return;
    if (_cellList[i_w].isLocked()) return;
    _gainBucket.modifyKeyLIFOBy(_vertexArray[i_w], deltaGain);
  }

  /** Push back the cell to gain bucket. Precondition: v is not in bucket. */
  void pushBack(unsigned int , const Cell& v, short gain)
  {
    _gainBucket.pushBack(_vertexArray[&v - _cellList], gain);
  }

  /** Update the gain structure according to the freeVertexList. */
  void updateGain(const std::vector<unsigned char>& ,
                  const std::vector<short>& gain)
  {
    init(gain);
  }

private:
  Netlist&       _H;                  /**< netlist hypergraph */
  VertexPQ       _gainBucket;         /**< gain bucket for each partition */
  VertexPQCur    _gainBucketCur;      /**< gain bucket cursor */
  Vertex* const  _vertexArray;        /**< VERTEX array */
  Cell* const    _cellList;
};

#endif
