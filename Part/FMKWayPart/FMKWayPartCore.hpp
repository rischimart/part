#ifndef FM_KWAY_PART_CORE_HPP
#define FM_KWAY_PART_CORE_HPP

#include "FMKWayPartComm.hpp"
#include <vector>

class FMKWayGainArray;

/** @addtogroup group5
 *  @{
 */

/**
 * FM Multi-way Partitioning Core. This class is mainly for
 * calculating the initial gain and calculating the amount of the gain
 * change (delta gain) according to the move.
 */
class FMKWayPartCore : public FMKWayPartComm
{
protected:
  /** Constructor */
  FMKWayPartCore(const FMParam& param);

  /** Setup the initial gain. Also setup the gain buckets data
      structure. Take O(n) time. */
  void initGainCore(const std::vector<unsigned char>& part);

  void updateNetStatus();

  /** Update the gain structure given the moving vertex */
  template <class MoveStruct>
  void updateMove(const std::vector<unsigned char>& part,
                  const FMMoveInfo& moveInfo,
                  MoveStruct& ms);


  int updateCore(const std::vector<unsigned char>& part,
                 const FMMoveInfo& moveInfo);

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

  /** Update the gain structure given the moving vertex */
  inline void updateMoveCore(const std::vector<unsigned char>& part,
                             const FMMoveInfo& moveInfo,
                             FMKWayGainArray& ga);

  /** Update the gain structure with hyperedge metric given the moving
      vertex */
  inline void updateMoveHEdgeCore(const std::vector<unsigned char>& part,
			      const FMMoveInfo& moveInfo,
			      FMKWayGainArray& ga);

  /** Update the gain structure with (K-1) metric given the moving vertex */
  template <class MoveStruct>
  inline void updateMoveKMinus1Core(const std::vector<unsigned char>& part,
				    const FMMoveInfo& moveInfo,
				    MoveStruct& ms);

  /** Update the gain structure with SOED metric given the moving vertex */
  template <class MoveStruct>
  inline void updateMoveSOEDCore(const std::vector<unsigned char>& part,
				 const FMMoveInfo& moveInfo,
				 MoveStruct& ms);

  /** Update the gain structure with TSV metric given the moving vertex */
  template <class MoveStruct>
  inline void updateMoveTSVCore(const std::vector<unsigned char>& part,
		        const FMMoveInfo& moveInfo,
		        MoveStruct& ms);

private:
  std::vector<unsigned char>  _netStatus; /**< 0-253 : loose net (anchor idx),
                                               64:free net, 65: locked net*/
};

/** @} */

#endif
