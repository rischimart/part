#ifndef FM_BI_PART_COMM_HPP
#define FM_BI_PART_COMM_HPP

#include "FMBiPartMgrBase.hpp"
#include "FMMoveInfo.hpp"
#include <vector>

/** @addtogroup group3
 *  @{
 */

/** 
 * FM Bi-Partitioning Manager Common Utility. This class is shared by FM
 * bi-partitioning manager and refinement bi-partitioning
 * manager. Common code for calculating the gain.
 */
class FMBiPartComm : public FMBiPartMgrBase
{
protected:
  /** Constructor */
  FMBiPartComm(const FMParam& param);

  /** Constructor */
  virtual ~FMBiPartComm() {}

  /** Setup the initial gain. */
  void initGain(const Net& e, const std::vector<unsigned char>& part);

  /** Update the gain structure given the moving vertex */
  template <class GainStruct>
  void updateMove2PinNet(const Net& e,
                         const std::vector<unsigned char>& part, 
                         unsigned int fromPart,
                         const Cell& v, 
                         GainStruct& s);

  /** Update the gain structure given the moving vertex */
  template <class GainStruct>
  void updateMoveGeneralNet(const Net& e,
			    const std::vector<unsigned char>& part, 
			    unsigned int fromPart,
			    const Cell& v, 
			    GainStruct& s);

  /** @return the gain of the moving cell. */
  int getGain(const FMMoveInfo& moveInfo) const
  {
    return _gain[moveInfo._i_v];
  }

private:
  int   _IdVec[32768];       /** cache neighbor information */
  short _deltaGain[32768];   /** cache neighbor information */

protected:
  std::vector<short> _gain;  /**< gain of each node */
  int              _num[2];  /**< number of vertices on each partitions */
};

/** @} */

#endif 
