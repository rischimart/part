#ifndef FM_BI_GAIN_MGR3_HPP
#define FM_BI_GAIN_MGR3_HPP

#include "FMBiGainMgr2.hpp"

/** 
 * @addtogroup group3
 * @{
 */

/**
 * FM Bi-partitioning Gain Manager (FIFO version). Note that when the
 * class is used in flat mode, it works significantly worse than the
 * LIFO gain management. However, when it is used in multilevel mode,
 * the performances are almost the same.
 */
class FMBiGainMgr3 : public FMBiGainMgr2
{
public:
  /** Constructor */
  FMBiGainMgr3(const FMParam& param) : FMBiGainMgr2(param) {}

  /** Update the neighbor of the moved cells due to the move. */
  void updateNeighbor(unsigned int whichPart, 
                      unsigned int i_w, short deltaGain)
  {
    // Only free vertices are allowed to moved, therefore, only their
    // gains are updated.
    if (_cellList[i_w].isLocked()) return;
    // Vertices are inserted in FIFO manner.
    _gainBucket[whichPart].modifyKeyBy(_vertexArray[i_w], deltaGain);
  }
};

#endif
