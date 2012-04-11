#ifndef FM_PW_PART_TMPL_HPP
#define FM_PW_PART_TMPL_HPP

#include <FMKWayPartMgrBase.hpp>
#include <Netlist.hpp>
#include <vector>

using std::vector;

/** Forward declaration */
class FMParam;
class Netlist;
class GNode;

/** @addtogroup partitioning_group
 *  @{
 */

/** 
 * Iterative Improvement Partitioning Template Class for Pairwise 
 * Movement algorithm.
 */
template <class GainTmpl>
class FMPWPartTmpl : public FMKWayPartMgrBase
{
  typedef GNode Vertex;

public:
  /** Constructor. */
  FMPWPartTmpl(const FMParam& param);

  /** 
   * A complete single run. Initial solution is computed by this
   * function. If no feasible solution is found, an exception will be
   * thrown.  @return the cut cost.
   */
  virtual unsigned int doPartitionOne(vector<unsigned char>& part);

  /** For multilevel code only */
  unsigned int doPartitionOne4(vector<unsigned char>& part);

private:
  /** Pairwise grouping */
  inline void initGrouping2();

  inline unsigned int doPartitionInternal(vector<unsigned char>& part);

  inline void projectUp(const vector<unsigned char>& part,
                        vector<unsigned char>& pw_part,
                        unsigned int group) const;

  inline void projectDown(vector<unsigned char>& part,
                          const vector<unsigned char>& pw_part,
                          unsigned int group) const;

private:
  vector<unsigned char> _tpart;       /**< temporary solution for
				            sequences of changes */
  vector<unsigned int>  _moveTo;      /**< a map of block movings */
  vector<unsigned int>  _groupMap;    /**< group belong to */
  vector<unsigned int>  _groupInvMap; /**< group belong to */
  unsigned int          _numGroups;   /**< number of groups */
  vector<Netlist*>      _sGVec;

};

/** @} */

#endif 
