#ifndef FM_KWAY_PART_COMM_HPP
#define FM_KWAY_PART_COMM_HPP

#include "FMKWayPartMgrBase.hpp"
#include <FMMoveInfo.hpp>
#include <vector>

/** @addtogroup group5
 *  @{
 */

/**
 * FM Multi-way Partitioning Common Utility. This class is mainly reponsible
 * for calculating the initial gain and calculating the amount of the gain
 * change (delta gain) according to the move.
 */
class FMKWayPartComm : public FMKWayPartMgrBase
{
  static const unsigned int _M = 64;  /**< maximum number of partitions */

protected:
  //xxx /** Constructor */
  //xxx FMKWayPartComm(Netlist& H, unsigned int K=3, int CM=1);

  /** Constructor */
  FMKWayPartComm(const FMParam& param);

  /** Setup the initial gain with (K-1) metric. Also setup the gain
      buckets data structure. Take O(n) time. */
  void initGainKMinus1(const Net& e, const std::vector<unsigned char>& part);

  /** Setup the initial gain with SOED metric. Also setup the gain
      buckets data structure. Take O(n) time. */
  void initGainSOED(const Net& e, const std::vector<unsigned char>& part);

  /** Setup the initial gain with hyperedge metric. Also setup the
      gain buckets data structure. Take O(n) time. */
  void initGainHEdge(const Net& e, const std::vector<unsigned char>& part);


  /** Setup the initial gain with TSV metric. Also setup the
      gain buckets data structure. Take O(n) time. */
  void initGainTSV(const Net& e, const std::vector<unsigned char>& part);

  template <class MoveStruct>
  void updateMove2PinNet(const Net& e,
			 const std::vector<unsigned char>& part,
                         const FMMoveInfo& moveInfo,
			 int weight,
			 MoveStruct& ms);


  template <class MoveStruct>
  void updateMove2PinNet4(const Net& e,
			  const std::vector<unsigned char>& part,
                          const FMMoveInfo& moveInfo,
			  int weight,
			  MoveStruct& ms);


  /** Update the gain structure with hyperedge metric given the moving
      vertex */
  template <class MoveStruct>
  void updateMoveGeneralNetHEdge(const Net& e,
				 const std::vector<unsigned char>& part,
				 const FMMoveInfo& moveInfo,
				 MoveStruct& ms);

  /** Update the gain structure with hyperedge metric given the moving
      vertex */
  template <class MoveStruct>
  void updateMoveGeneralNetHEdge4(const Net& e,
				  const std::vector<unsigned char>& part,
				  const FMMoveInfo& moveInfo,
				  MoveStruct& ms);

  /** Update the gain structure with (K-1) metric given the moving vertex */
  template <class MoveStruct>
  void updateMoveGeneralNetKMinus1(const Net& e,
				   const std::vector<unsigned char>& part,
				   const FMMoveInfo& moveInfo,
				   MoveStruct& ms);

  /** Update the gain structure with (K-1) metric given the moving vertex */
  template <class MoveStruct>
  void updateMoveGeneralNetKMinus14(const Net& e,
				   const std::vector<unsigned char>& part,
				   const FMMoveInfo& moveInfo,
				   MoveStruct& ms);

  /** Update the gain structure with SOED metric given the moving vertex */
  template <class MoveStruct>
  void updateMoveGeneralNetSOED(const Net& e,
				const std::vector<unsigned char>& part,
				const FMMoveInfo& moveInfo,
				MoveStruct& ms);

  /** Update the gain structure with SOED metric given the moving vertex */
  template <class MoveStruct>
  void updateMoveGeneralNetSOED4(const Net& e,
				const std::vector<unsigned char>& part,
				const FMMoveInfo& moveInfo,
				MoveStruct& ms);

  /** Update the gain structure using TSV metric given the moving vertex */
  template <class MoveStruct>
  void updateMoveGeneralNetTSV(const Net& e,
			  const std::vector<unsigned char>& part,
              const FMMoveInfo& moveInfo,
			  MoveStruct& ms);
  /** Update the gain structure using TSV metric given the moving vertex */
  template <class MoveStruct>
  void updateMoveGeneralNetTSV4(const Net& e,
			   const std::vector<unsigned char>& part,
               const FMMoveInfo& moveInfo,
			   MoveStruct& ms);

  /** @return the gain of the moving cell */
  int getGain(const FMMoveInfo& moveInfo) const
  {
    return _gain[moveInfo._toPart][moveInfo._i_v];
  }


private:
  /** Initialize gain for two-pin nets */
  inline void initGain2PinNet(const Net& e,
			      const std::vector<unsigned char>& part, int weight);

  inline void initGainGeneralNetHEdge(const Net& e,
				      const std::vector<unsigned char>& part);

  inline void initGainGeneralNetKMinus1(const Net& e,
					const std::vector<unsigned char>& part);

  inline void initGainGeneralNetSOED(const Net& e,
				     const std::vector<unsigned char>& part);

  inline void initGainGeneralNetTSV(const Net& e,
                     const std::vector<unsigned char>& part);

  /** Calculate degree and capture the neighbor informations */
  inline void initGainCalcDegree(const Net& e,
					 const std::vector<unsigned char>& part);

  /** Calculate degree and capture the neighbor informations */
  inline void updateMoveCalcDegree(const Net& e, const Cell& v);

  /** Update the gain structure with hyperedge metric given the moving
      vertex */
  template <class MoveStruct>
  inline void
  updateMoveGeneralNetCommHEdge(const Net& e,
				const std::vector<unsigned char>& part,
				const FMMoveInfo& moveInfo,
				MoveStruct& ms);

  /** Update the gain structure with (K-1) metric given the moving vertex */
  template <class MoveStruct>
  inline void
  updateMoveGeneralNetCommKMinus1(const Net& e,
				  const std::vector<unsigned char>& part,
				  const FMMoveInfo& moveInfo,
				  MoveStruct& ms);

  /** Update the gain structure with SOED metric given the moving vertex */
  template <class MoveStruct>
  inline void
  updateMoveGeneralNetCommSOED(const Net& e,
			       const std::vector<unsigned char>& part,
			       const FMMoveInfo& moveInfo,
			       MoveStruct& ms);

  /** Update the gain structure using SOED metric given the moving vertex */
  template <class MoveStruct>
  inline void
  updateMoveGeneralNetCommTSV(const Net& e,
			      const std::vector<unsigned char>& part,
                              const FMMoveInfo& moveInfo,
			      MoveStruct& ms);

private:
  int   _IdVec[32768];           /**< cache neighbor information */
  short _deltaGain[32768][_M];   /**< cache neighbor information */
  unsigned int _degree;          /**< number of unvisited neighbors */
  unsigned int _numNonEmpty;     /**< number of non-empty partotions */

protected:
  std::vector<short> _gain[_M];  /**< gain of each node */
  int              _num[_M];     /**< number of vertices on each partitions */
  short     _deltaGainV[_M];     /**< gain change of the moving cell */
};

/** @} */

#endif
