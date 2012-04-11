#ifndef FM_KWAY_GAIN_MGR3_HPP
#define FM_KWAY_GAIN_MGR3_HPP

#include "FMKWayGainMgr2.hpp"

/** @addtogroup group5
 *  @{
 */

/**
 * FM Multi-way Partitioning Gain Manager (FIFO version). In this
 * version, vertices are inserted in FIFO manner. It is expected that
 * the performance of FIFO manager is worse than the LIFO
 * manager. However, the difference is not noticable in the multi-level
 * mode compared to the flat mode. 
 * @see FMKWayGainMgr2.
 */
class FMKWayGainMgr3 : public FMKWayGainMgr2
{
public:
  /** Constructor */
  FMKWayGainMgr3(const FMParam& param) : FMKWayGainMgr2(param) {}

  /** Update the gain changes of the neighbor vertex in whichPart */
  void updateNeighbor(unsigned int whichPart,  ///< in which partition
                      unsigned int i_w,    ///< the affected neighbor vertex
                      const short* deltaGain)  ///< the gain changes
  {
    if (_cellList[i_w].isLocked()) return;
    for (unsigned int k=0; k<getNumPartitions(); ++k) {
      if (k == whichPart) continue;
      _gainBucket[k].modifyKeyBy(_vertexArray[k][i_w], deltaGain[k]); // FIFO
    }
  }
};

/** @} */

#endif
