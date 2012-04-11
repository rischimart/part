#ifndef FM_LIMITS_HPP
#define FM_LIMITS_HPP

#include <limits>

/** @addtogroup partitioning_group
 *  @{
 */

/** 
 * Limitations of the Software. Most of the limitations are due to
 * the decision of trade-off either the memory storge or run-time
 * or both. If larger capabilities are requested, do let me
 * know. Thanks. 
 * By the way, another limitation of this software is that in fact it
 * is not thread-safe. One reason is that I don't know how to write
 * thread-safe codes. Second, I tried to replace the current STL with
 * the STLport's thread-safe STL and found that the run-time almost
 * double than before. Because so far this software is only used for a
 * stand-alone non-window program, this issue leaves as a future enhancement.
 */ 
class FMLimits
{
public:
  /** @return maximum number of partitions */
  static unsigned int maxNumOfPartitions() 
  { // Partition numbers are represented by "unsigned char".
    return 256; // 0-255
  }

  /** @return maximum cell weight. @see Cell. */
  static int maxCellWeight() 
  { // Cell weights are represented by 28-bit unsigned integer. Note
    // that this includes the limits of the weights of clusters.
    return 268435455; // 2^28 - 1
  }

  /** @return maximum net weight. @see Net */
  static int maxNetWeight() 
  { // Net weights are represented by "unsigned short". Note that this
    // includes the limit of the weights of the merged nets.
    return std::numeric_limits<unsigned short>::max(); 
  }

  /** @return maximum (weighted) degree of cells. Note that for SOED
      cost model (Sum-Of-External-Degrees), the limit should be further
      divided by 2 in order to guarantee no risk. */
  static unsigned int maxDegreeOfCells() 
  { // Gains are represented by "short".
    return std::numeric_limits<short>::max(); 
  }

  /** @return maximum number of pins per net. Note that the
      partitioner can tolerate the present of high-fanout net by
      pruning them out before start. But it may degrade the quality of
      results if a circuit contains a lot of such nets. */
  static unsigned int maxFanOutOfNets() 
  { // The arrays "numA" and "numB" are represented by "unsigned char".
    return std::numeric_limits<unsigned char>::max(); 
  }
};

/** @} */
#endif
