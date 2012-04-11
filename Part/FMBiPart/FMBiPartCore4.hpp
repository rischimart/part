#ifndef FM_BI_PART_MGR_CORE4_HPP
#define FM_BI_PART_MGR_CORE4_HPP

#include "FMBiPartComm.hpp"
#include "FMBiGainArray.hpp"
#include "FMBiMoveStruct.hpp"
#include <vector>

/** @addtogroup group3
 *  @{
 */

/** 
 * FM Bi-partitioning Manager Core (version 4). This version is tailered for
 * refinement bi-partitioning manager in multi-level method.
 */
class FMBiPartCore4 : public FMBiPartComm
{
protected:
  /** Constructor */
  FMBiPartCore4(const FMParam& param);

  /** Constructor */
  virtual ~FMBiPartCore4() {}

  /** Setup the initial gain. Take O(n) time. */
  void initGainCore(const std::vector<unsigned char>& part);

  /** Update the gain structure given the moving vertex 
      (special for refinement code) */
  template <class GainTmpl>
  void updateMove4(const std::vector<unsigned char>& part, 
                   const FMMoveInfo& moveInfo,
                   GainTmpl& gainMgr);

  /** Push back the vertex to the gain manager
      (special for refinement code) */
  template <class GainTmpl>
  void pushBack4(unsigned int whichPart, const Cell& v, GainTmpl& gainMgr)
  {
    gainMgr.pushBack(whichPart, v, _gain[&v - _cellList]);
  }

  void updateNetStatus();

private:
  /** Update the gain structure given the moving vertex */
  template <class S>
  void updateMove(const std::vector<unsigned char>& part, 
                  const FMMoveInfo& moveInfo,
                  S& s);

private:
  std::vector<unsigned char>  _numA[2];
  std::vector<unsigned char>  _netStatus; /**< 0-253 : loose net (anchor idx), 
                                               64:free net, 65: locked net*/
};

/** @} */

#endif 
