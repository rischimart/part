#ifndef FM_KWAY_GAIN_MGR4_HPP
#define FM_KWAY_GAIN_MGR4_HPP

#include <GBPQueue.hpp>
#include <GNode.hpp>
#include <Cell.hpp>
#include <FMMoveInfo.hpp>
#include <vector>

/** Forward declaration */
class Netlist;
class FMParam;
class KWayConstrMgr3;

/** @addtogroup group5
 *  @{
 */

/**
 * FM Multi-way Partitioning Gain Manager. This class responsible for
 * maintaining 
 * the gain bucket structures and helps to select the best vertex. In
 * this version, K(K-1) bucket arrays are used, one for each partition. 
 * @see FMBiGainMgr2.
 */
class FMKWayGainMgr4
{
  typedef GNode Vertex;
  typedef GBPQueue<Vertex> VertexPQ;
  typedef GBPQueueCursor<Vertex> VertexPQCur;

  static const unsigned int _M = 64;  /**< maximum no. of partitions */

public:
  /** Constructor */
  FMKWayGainMgr4(const FMParam& param);

  /** Setup the gain bucket structure */
  void init(const std::vector<unsigned char>& part, std::vector<short>* gain);


  /** @return the next "best" vertex that satisfies all constraints
      when it is moved. Return also move from where to where. */
  Vertex* select(FMMoveInfo& moveInfo,
                 const std::vector<unsigned char>& part,
                 KWayConstrMgr3& constrMgr);

  /** @return the gain of vertex if it moves from fromPart to toPart */
  int getGain(const FMMoveInfo& mInfo) const
  {
    const unsigned int toPart = mInfo._toPart;
    const unsigned int fromPart = mInfo._fromPart;
    const Vertex& v = _vertexArray[toPart][mInfo._i_v];
    return _gainBucket[fromPart][toPart].getKey(v);
  }


  /** Update the changes according to the move of vertex v from
      fromPart to toPart */  
  void updateMove(const FMMoveInfo& moveInfo,
                  const short* = 0)      ///< the gain changes (not used here)
  {
    const unsigned int fromPart = moveInfo._fromPart;
    const unsigned int i_v = moveInfo._i_v;
    for (unsigned int k=0; k<getNumPartitions(); k++) {
      if (fromPart == k) continue;
      _gainBucket[fromPart][k].detach(_vertexArray[k][i_v]);
    }
  }


  /** Update the gain changes of the neighbor vertex in whichPart */
  void updateNeighbor(unsigned int whichPart,  ///< in which partition
                      unsigned int i_w,    ///< the affected neighbor vertex
                      const short* deltaGain)  ///< the gain changes
  {
    if (_cellList[i_w].isLocked()) return;
    for (unsigned int k=0; k<getNumPartitions(); k++) {
      if (k == whichPart) continue;
      _gainBucket[whichPart][k].modifyKeyLIFOBy(_vertexArray[k][i_w], 
                                                deltaGain[k]);
    }
  }


  /** Push back the vertices of cell i_w to the gain manager. */
  void pushBack(unsigned int whichPart, unsigned int i_w, const short* gain)
  {
    for (unsigned int k=0; k<getNumPartitions(); k++) {
      if (whichPart == k) continue;
      _gainBucket[whichPart][k].pushBack(_vertexArray[k][i_w], gain[k]);
    }
  }

  
  /** Update the gain structure according to the current gain. */
  void updateGain(const std::vector<unsigned char>& part, 
                  std::vector<short>* gain
                  )
  {
    init(part, gain);
  }


protected:
  /** @return the number of partitions. */
  unsigned int getNumPartitions() const { return _K; }

protected:
  Netlist&       _H;                      /**< netlist hypergraph */
  unsigned int   _K;                      /**< number of partitions */
  VertexPQ       _gainBucket[_M][_M];     /**< gain bucket for each partition */
  VertexPQCur    _gainBucketCur[_M][_M];  /**< gain bucket cursor */
  std::vector<Vertex> _vertexArray[_M];   /**< keep the data struct for the algo.*/
  Cell* const    _cellList;               /**< Cell array offset */
};

/** @} */

#endif
