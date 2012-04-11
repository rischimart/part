#ifndef FM_KWAY_GAIN_ARRAY_HPP
#define FM_KWAY_GAIN_ARRAY_HPP

#ifndef FM_MOVE_INFO_HPP
#include "FMMoveInfo.hpp"
#endif

#include <vector>

/** 
 * @defgroup group5 Multi-Way Partitioner Core Engine 
 * @ingroup partitioning_group
 *
 * This module contains the implementation of Multi-Way FM algorithm 
 * and its variants. From "M. Sarrafzadeh and Wong. An Introduction to
 * VLSI Design", p 47:
 * "The generlization of the bipartitioning, multiway partitioning, is
 * also an important issue in VLSI design... Most of them adopt a
 * bipartition algorithm iteratively to find a multiway partition. For
 * example, for a four-way partition, we would use the bipartition
 * alogrithm to partition the nodes into two blocks, and then
 * partition each of these blocks into two sub-blocks. As pointed out
 * in [181], a bad result in the first partition may bias the second
 * one (and the rest), if a bipartition alogrithm is used
 * hierarchically. An alternative way to reduce the multi-way partition
 * problem into serveral bipartition problems is to attempt to improve
 * the partition uniformly at each step...Experiments showed that the
 * concept is especially useful for partitioning into a large number
 * of subsets.
 *
 * From [G. Karypis and V. Kumar, Multilevel k-way
 * Hypergraph Partitioning, 36th Design Automation Conference]:
 * "... Second, a k-way partitioning algorithm is capable of enforcing
 * tighter balancing constraints while retaining the ability to
 * sufficiently explore the feasible solution space to optimize the
 * partitioning objective. This is especially true when the
 * partitioning solution must simultaneously satisfy multiple
 * balancing constraints [21]. Third, a method that obtains a k-way
 * partitioning directly can potentially produce much better
 * partitionings than a method that computes a k-way partitioning via
 * recursive bisection [8] "  
 *
 * From [J. Cong and S. Lim, "Multiway Partitioning with Pairwise
 * Movement," Proc. IEEE ICCAD'98]:
 * Recursive approach is a simple extension of bipartitioning to
 * multiway partitioning. It applies bipartitioning recusively until
 * the desired number of partitions is obtained. It is computationing
 * simple and fast, and many of the heuristics devised for
 * bipartitioning can be applied to further reduce the current level
 * cutsize. However, we note three major limitations of the recursive
 * approach. First cells can only move across the current
 * configuration as depicted in Figure 1-(a). The objective of
 * recursive bipartitioning is to redice the number of nets crossing
 * the current level cutline in the absence of global information,
 * which can trap the partitioner into a local minima and limit the
 * solution quality ... Third, it becomes harder and harder to reduce
 * the cutsize as the bipartitioner performs deeper level cuts.Highly
 * optimized 1st and 2nd level cuts can cause 3rd and 4th level cuts
 * to cut through very dense clusters. Thus, this conflicting
 * objective can cause recursive approach to produce low quality
 * multiway partitioning solutions.
 *
 * Compared with the pairwise movement method, this method may run
 * faster. In flat mode, this method may produce poorer result than
 * the pairwise movement method. However, it seems OK to be used as
 * refinement manager.
 * @{
 */

/**
 * FM Multi-Way Gain Array. An Adapter class.
 */
class FMKWayGainArray
{
public:
  /** Constructor */
  FMKWayGainArray(std::vector<short>* gain, unsigned int K) : 
    _gain(gain), _K(K) 
  {
  }


  /** Update the gain changes according to the move of vertex v from
      fromPart to toPart */
  void updateMove(const FMMoveInfo& moveInfo,
                  const short* deltaGain) ///< the gain change
  {
    const unsigned int i_v = moveInfo._i_v;
    const unsigned int fromPart = moveInfo._fromPart;
    const unsigned int toPart = moveInfo._toPart;
    for (unsigned int k=0; k<getNumPartitions(); ++k) {
      if (k == fromPart || k == toPart) continue;
      _gain[k][i_v] += deltaGain[k];
    }
    _gain[fromPart][i_v] = -_gain[toPart][i_v];
    _gain[toPart][i_v] = 0; // ???
  }


  /** Update the gain changes of the neighbor vertex in whichPart */
  void updateNeighbor(unsigned int whichPart,  ///< in which partition
                      unsigned int i_w,  ///< neighbor vertex affected
                      const short* deltaGain)   ///< the gain change
  {
    for (unsigned int k=0; k<getNumPartitions(); ++k) {
      if (k == whichPart) continue;
      _gain[k][i_w] += deltaGain[k]; 
    }
  }

private:
  /** @return the number of partitions. */
  unsigned int getNumPartitions() const { return _K; }


private:
  std::vector<short>* _gain; ///< original gain array
  unsigned int _K;           ///< number of partitions
};

/** @} */
#endif
