#ifndef FM_KWAY_PART_CORE4_HPP
#define FM_KWAY_PART_CORE4_HPP

#include "FMKWayPartComm.hpp"
#include "FMKWayGainArray.hpp"
#include "FMKWayMoveStruct.hpp"
#include <vector>

/** @addtogroup group5
 *  @{
 */

/**
 * FM Multi-way Partitioning Core (version 4). This class is mainly for
 * calculating the initial gain and calculating the amount of the gain
 * change (delta gain) according to the move. This class is tailored
 * for refinement manager.
 */
class FMKWayPartCore4 : public FMKWayPartComm
{
  static const unsigned int _M = 64;  /**< maximum number of partitions */

protected:
  /** Constructor */
  FMKWayPartCore4(const FMParam& param);

  /** Setup the initial gain. Also setup the gain buckets data
      structure. Take O(n) time. */
  void initGainCore(const std::vector<unsigned char>& part);

  void updateNetStatus();

  /** Update the gain structure given the moving vertex */
  template <class MoveStruct>
  void updateMove(const std::vector<unsigned char>& part,
                  const FMMoveInfo& moveInfo,
                  MoveStruct& ms);

  /** Update the gain structure given the moving vertex
      (special for refinement code) */
  template <class GainMgr>
  void updateMove4(const std::vector<unsigned char>& part,
                   const FMMoveInfo& moveInfo,
                   GainMgr& gainMgr)
  {
    FMKWayGainArray gainArray(_gain, getNumPartitions());
    FMKWayMoveStruct<GainMgr, FMKWayGainArray> gs(gainMgr, gainArray);
    updateMove(part, moveInfo, gs);
  }


  /** Push back the vertex to the gain manager
      (special for refinement code) */
  template <class GainMgr>
  void pushBack4(unsigned int toPart, const Cell& v, GainMgr& gainMgr)
  {
    const unsigned int i_v = &v - _cellList;
    std::vector<short> gainV(getNumPartitions());
    for (unsigned int k=0; k<getNumPartitions(); k++) {
      gainV[k] = _gain[k][i_v];
    }
    gainMgr.pushBack(toPart, i_v, &gainV[0]);
  }

private:
  /** Setup the initial gain with Hyperedge metric. Also setup the
      gain buckets data structure. Take O(n) time. */
  inline void initGainCoreHEdge(const std::vector<unsigned char>& part);

  /** Setup the initial gain with (K-1) metric. Also setup the gain
      buckets data structure. Take O(n) time. */
  inline void initGainCoreKMinus1(const std::vector<unsigned char>& part);

  /** Setup the initial gain with SOED metric. Also setup the gain
      buckets data structure. Take O(n) time. */
  inline void initGainCoreSOED(const std::vector<unsigned char>& part);

  /** Setup the initial gain with TSV metric. Also setup the gain
      buckets data structure. Take O(n) time. */
  inline void initGainCoreTSV(const std::vector<unsigned char>& part);

  /** Update the gain structure with hyperedge metric given the moving
      vertex */
  template <class MoveStruct>
  inline void updateMoveHEdge(const std::vector<unsigned char>& part,
			      const FMMoveInfo& moveInfo,
			      MoveStruct& ms);

  /** Update the gain structure with (K-1) metric given the moving vertex */
  template <class MoveStruct>
  inline void updateMoveKMinus1(const std::vector<unsigned char>& part,
				const FMMoveInfo& moveInfo,
				MoveStruct& ms);

  /** Update the gain structure with SOED metric given the moving vertex */
  template <class MoveStruct>
  inline void updateMoveSOED(const std::vector<unsigned char>& part,
			     const FMMoveInfo& moveInfo,
			     MoveStruct& ms);

  /** Update the gain structure with TSV metric given the moving vertex */
  template <class MoveStruct>
  inline void updateMoveTSV(const std::vector<unsigned char>& part,
		      const FMMoveInfo& moveInfo,
		      MoveStruct& ms);

private:
  std::vector<unsigned char>  _numA[_M];
  std::vector<unsigned char>  _netStatus; /**< 0-253 : loose net (anchor idx),
                                              64:free net, 65: locked net*/
};

/** @} */

#endif
