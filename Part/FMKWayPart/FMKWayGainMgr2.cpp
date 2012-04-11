#include "FMKWayGainMgr2.hpp"
#include "KWayConstrMgr3.hpp"
#include <FMParam.hpp>
#include <Netlist.hpp>
#include <GBPQueue.hpp>
#include <vector>
#include <climits>  //added by Zhou
#include <cassert>  //added by Zhou
#include <iostream> //for debug
using std::vector;
using std::cout;  //for debug
using std::endl;  //for debug
typedef GNode Vertex;

/** Constructor */
FMKWayGainMgr2::FMKWayGainMgr2(const FMParam& param) :
  _H(param._H),
  _K(param._K),
  _cellList(&_H.getCell(0))
{
  assert(_K >= 2);
  const unsigned int n = _H.getNumCells();
  const unsigned int pmax = // maximum weighted degree of cells among all cells
    (param._cm == 2)? 2*_H.getMaxDegree() : _H.getMaxDegree();

  for (unsigned int k=0; k<getNumPartitions(); k++) {
    _vertexArray[k].resize(n);
    _gainBucket[k].setUp(-pmax, pmax);
    _gainBucketCur[k].associate(_gainBucket[k]);
  }
}


void FMKWayGainMgr2::init(const vector<unsigned char>& part,
                          vector<short>* gain)
{
  const unsigned int n = _H.getNumCells();

  vector<unsigned int> vaRandIdx(n);
  unsigned int i;
  for (i=0; i<n; i++) vaRandIdx[i] = i;
  std::random_shuffle(vaRandIdx.begin(), vaRandIdx.end());

  for (unsigned int k=0; k<getNumPartitions(); k++) {
    _gainBucket[k].clear();
    vector<Vertex>& vk = _vertexArray[k];
    const vector<short>& gk = gain[k];
    for (i=0; i<n; i++) {
      const unsigned int i_v = vaRandIdx[i];
      if (_cellList[i_v].isVisited()) continue;
      if (part[i_v] == k) continue;
      _gainBucket[k].pushBackFast(vk[i_v], gk[i_v]);
    }
    _gainBucket[k].resync();
  }
}



// @return vertex for the next move. @return NULL if no move is allowed.
Vertex* FMKWayGainMgr2::select(FMMoveInfo& moveInfo,
                               const vector<unsigned char>& part,
                               KWayConstrMgr3& constrMgr)
{
  unsigned int toPart=9999;
  unsigned int k;

  for (k=0; k<getNumPartitions(); k++) {
    _gainBucketCur[k].reset();
  }

  while (1) {
    int maxGain = INT_MIN; // keep track the maximum gain
    int nmatch = 0; // keep track how many vertices with the same gain seen
    cout << "FMKWayGainMgr2::select checkpoint 1" << endl;
    for (k=0; k<getNumPartitions(); k++) {
      if (!constrMgr.toOk(k)) continue;
      cout << "FMKWayGainMgr2::select checkpoint 1.5" << endl;
      if (!_gainBucketCur[k].hasNext()) continue;
      cout << "FMKWayGainMgr2::select checkpoint 1.8" << endl;
      const int gaink = _gainBucketCur[k].getRelKey();
      cout << "FMKWayGainMgr2::select checkpoint 2" << endl;
      cout << "gaink = " << gaink << endl;
      if (maxGain < gaink) {
        maxGain = gaink;
        toPart = k;
        nmatch = 1; // restart the counting
      } else if (maxGain == gaink && rand() % ++nmatch == 0) {
        // ties are broken randomly
        toPart = k;
      }
    }
    /* Note: the randomization is used in order to evenly distribute
       cells into the multiple partitions. Experiments showed that without
       the procedure, the block sizes were baised and the QoS was poor. */

    if (maxGain == INT_MIN) break;
    cout << "FMKWayGainMgr2::select checkpoint 3" << endl;
    Vertex& vertex = _gainBucketCur[toPart].getNext();
    const unsigned int i_v = &vertex - &_vertexArray[toPart][0];
    const unsigned int fromPart = part[i_v];
    assert(fromPart != toPart);
    Cell& cell = _cellList[i_v];

    const bool ok = constrMgr.satisfied(cell, fromPart, toPart);
    cout << "FMKWayGainMgr2::select checkpoint 4" << endl;
    if (ok) {
      moveInfo = FMMoveInfo(i_v, cell, fromPart, toPart);
      cout << "FMKWayGainMgr2::select checkpoint 5" << endl;
      return &vertex;
    }
  }

  return 0;
}

