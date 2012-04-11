#ifndef FM_BI_GAIN_ARRAY_HPP
#define FM_BI_GAIN_ARRAY_HPP

#ifndef FM_MOVE_INFO_HPP
#include "FMMoveInfo.hpp"
#endif

#include <vector>

/** @addtogroup group3
 *  @{
 */

/**
 * FM Bi-partitioning Gain Array. An Adapter class.
 */
class FMBiGainArray
{
public:
  /** Constructor */
  FMBiGainArray(std::vector<short>& gain) : 
    _gain(gain) 
  {
  }


  /** Update the gain of the moved cell after the move. */
  void updateMove(const FMMoveInfo& moveInfo)
  {
    const unsigned int i_v = moveInfo._i_v;
    _gain[i_v] = -_gain[i_v];
  }


  /** Update the gain of the neighbor of the moved cells due to the move. */
  void updateNeighbor(unsigned int , unsigned int i_w, short deltaGain)
  {
    _gain[i_w] += deltaGain;
  }

private:
  std::vector<short>& _gain;  /**< array of gains of cells */
};

/** @} */
#endif
