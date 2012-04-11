#ifndef FM_BI_PART_CORE_HPP
#define FM_BI_PART_CORE_HPP

#include "FMBiPartComm.hpp"
#include <vector>

class FMBiGainArray;

/** @addtogroup group3
 *  @{
 */

/** 
 * FM Bi-partitioning Manager Core. 
 */
class FMBiPartCore : public FMBiPartComm
{
protected:
  /** Constructor */
  FMBiPartCore(const FMParam& param);

  /** Destructor */
  virtual ~FMBiPartCore() {}

  /** Setup the initial gain. Take O(n) time. */
  void initGainCore(const std::vector<unsigned char>& part);

  void updateNetStatus();

  /** Update the gain structure given the moving vertex */
  template <class S>
  void updateMove(const std::vector<unsigned char>& part, 
                  const FMMoveInfo& moveInfo,
                  S& s);

  /** Update the information after moving i_v */
  int updateCore(const std::vector<unsigned char>& part,
                 const FMMoveInfo& moveInfo);

private:
  inline void updateMoveCore(const std::vector<unsigned char>& part, 
                             const FMMoveInfo& moveInfo,
                             FMBiGainArray& ga);


private:
  std::vector<unsigned char>  _netStatus; /**< 0-253 : loose net (anchor idx), 
                                               64:free net, 65: locked net*/
}; 

/** @} */

#endif 
