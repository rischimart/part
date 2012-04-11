#include "KWayConstrMgrBase.hpp"
#include <boost/array.hpp>
#include <cassert>  //added by Zhou

using boost::array;

/** @return the item of maximum different of the array. 
    Precondition: num >=1 */
inline int maxDiff(const array<int,64>& array, int num)
{
  assert(num > 0);
  int maxValue = array[0];
  int minValue = array[0];
  for (int i=1; i<num; ++i) {
    if (maxValue < array[i]) {
      maxValue = array[i];
    } else if (minValue > array[i]) {
      minValue = array[i];
    }
  }
  return maxValue - minValue;
}


int KWayConstrMgrBase::getMaxDiff() const
{
  return maxDiff(_diff, getNumPartitions());
}

