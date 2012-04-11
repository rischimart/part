#include "FMKWayGainMgr4.hpp"
#include "KWayConstrMgr3.hpp"
#include <Netlist.hpp>
#include <FMParam.hpp>
#include <GBPQueue.hpp>
#include <vector>
#include <climits>  //added by Zhou
#include <cassert>  //added by Zhou

using std::vector;

typedef GNode Vertex;

/** Constructor */
FMKWayGainMgr4::FMKWayGainMgr4(const FMParam& param) : 
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
  }

  for (unsigned int k1=0; k1<getNumPartitions()-1; k1++) {
    for (unsigned int k2=k1+1; k2<getNumPartitions(); k2++) {
      _gainBucket[k1][k2].setUp(-pmax, pmax);
      _gainBucket[k2][k1].setUp(-pmax, pmax);
      _gainBucketCur[k1][k2].associate(_gainBucket[k1][k2]);
      _gainBucketCur[k2][k1].associate(_gainBucket[k2][k1]);
    }
  }
}


void FMKWayGainMgr4::init(const vector<unsigned char>& part, 
                          vector<short>* gain)
{
  const unsigned int n = _H.getNumCells();

  vector<unsigned int> vaRandIdx(n);
  unsigned int i;
  unsigned int k, k1, k2;
  for (i=0; i<n; i++) vaRandIdx[i] = i;
  std::random_shuffle(vaRandIdx.begin(), vaRandIdx.end());

  for (k1=0; k1<getNumPartitions()-1; k1++) {
    for (k2=k1+1; k2<getNumPartitions(); k2++) {
      _gainBucket[k1][k2].clear();
      _gainBucket[k2][k1].clear();
    }
  }
  for (k=0; k<getNumPartitions(); k++) {
    vector<Vertex>& vk = _vertexArray[k];
    const vector<short>& gk = gain[k];  
    for (i=0; i<n; i++) {
      const unsigned int i_v = vaRandIdx[i];
      if (_cellList[i_v].isVisited()) continue;
      const unsigned int part_v = part[i_v];
      if (part_v == k) continue;
      _gainBucket[part_v][k].pushBackFast(vk[i_v], gk[i_v]);
    }
  }
  for (k1=0; k1<getNumPartitions()-1; k1++) {
    for (k2=k1+1; k2<getNumPartitions(); k2++) {
      _gainBucket[k1][k2].resync();
      _gainBucket[k2][k1].resync();
    }
  }
}



// @return vertex for the next move. @return NULL if no move is allowed.
Vertex* FMKWayGainMgr4::select(FMMoveInfo& moveInfo,
                               const vector<unsigned char>& ,
                               KWayConstrMgr3& constrMgr)
{
  unsigned int k1, k2;
  unsigned int fromPart=9999, toPart=9999;

  for (k1=0; k1<getNumPartitions()-1; k1++) {
    for (k2=k1+1; k2<getNumPartitions(); k2++) {
      _gainBucketCur[k1][k2].reset();
      _gainBucketCur[k2][k1].reset();
    }
  }

  while (1) {
    int maxGain = INT_MIN; // keep track the maximum gain
    int nmatch = 0; // keep track how many vertices with the same gain seen
    for (k1=0; k1<getNumPartitions(); k1++) {
      if (!constrMgr.fromOk(k1)) continue;
      for (k2=0; k2<getNumPartitions(); k2++) {
        if (k1==k2) continue;
        if (!constrMgr.toOk(k2)) continue;
        if (!_gainBucketCur[k1][k2].hasNext()) continue; 
        const int gaink = _gainBucketCur[k1][k2].getRelKey();
        if (maxGain < gaink) {
          maxGain = gaink;
          fromPart = k1;
          toPart = k2;
          nmatch = 1; // restart the counting
        } else if (maxGain == gaink && rand() % ++nmatch == 0) {
          // tie break randomly
          fromPart = k1;
          toPart = k2;
        }
      }
    }
    /* Note: the randomization is used in order to evenly distribute
       cells into the multiple partitionss. Experiments showed that without
       the procedure, the block sizes were baised and the QoS was poor. */
    if (maxGain == INT_MIN) break;

    Vertex& vertex = _gainBucketCur[fromPart][toPart].getNext();
    const unsigned int i_v = &vertex - &_vertexArray[toPart][0];
    assert(fromPart != toPart);
    Cell& cell = _cellList[i_v];

    const bool ok = constrMgr.satisfied(cell, fromPart, toPart);
    if (ok) {
      moveInfo = FMMoveInfo(i_v, cell, fromPart, toPart);
      return &vertex;
    }
  }

  return 0;
}

