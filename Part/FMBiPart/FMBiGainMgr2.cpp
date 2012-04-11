#include "FMBiGainMgr2.hpp"
#include "BiConstrMgr3.hpp"
#include "FMParam.hpp"
#include <Netlist.hpp>
#include <GBPQueue.hpp>
#include <vector>

using std::vector;

typedef GNode Vertex;

/** Constructor */
FMBiGainMgr2::FMBiGainMgr2(const FMParam& param) : 
  _H(param._H),
  _vertexArray(_H.getVertexArray()),
  _cellList(&_H.getCell(0))
{
  const int pmax = _H.getMaxDegree();  // maximum pins of cells among all cells
  _gainBucket[0].setUp(-pmax, pmax);  // the gain is bound by [-pmax, pmax]
  _gainBucket[1].setUp(-pmax, pmax);
  _gainBucketCur[0].associate(_gainBucket[0]);
  _gainBucketCur[1].associate(_gainBucket[1]);
}


/** Initialize the gain buckets */
void FMBiGainMgr2::init(const vector<unsigned char>& part, 
                        const vector<short>& gain)
{
  const unsigned int n = _H.getNumCells();
  vector<unsigned int> vaRandIdx(n);
  unsigned int i;
  for (i=0; i<n; ++i) vaRandIdx[i] = i;
  std::random_shuffle(vaRandIdx.begin(), vaRandIdx.end());

  _gainBucket[0].clear();
  _gainBucket[1].clear();

  if (_H.hasFixedCells()) {
    for (i=0; i<n; ++i) {
      const unsigned int i_v = vaRandIdx[i];
      if (_cellList[i_v].isVisited()) continue;
      _gainBucket[part[i_v]].pushBackFast(_vertexArray[i_v], gain[i_v]);
    }
  } else { // don't need to check one by one
    for (i=0; i<n; ++i) {
      const unsigned int i_v = vaRandIdx[i];
      _gainBucket[part[i_v]].pushBackFast(_vertexArray[i_v], gain[i_v]);
    }
  }

  _gainBucket[0].resync();
  _gainBucket[1].resync();
}



// @return vertex for next move. @return NULL if no move is allowed.
Vertex* FMBiGainMgr2::select(FMMoveInfo& moveInfo,
                             const BiConstrMgr3& constraintMgr)
{
  unsigned int i_v;

  _gainBucketCur[0].reset();
  _gainBucketCur[1].reset();

  while (_gainBucketCur[0].hasNext() || _gainBucketCur[1].hasNext()) {
    const int gain0 = _gainBucketCur[0].getRelKey();
    const int gain1 = _gainBucketCur[1].getRelKey();
    if (gain0 > gain1) {
      Vertex& vertex0 = _gainBucketCur[0].getNext();
      i_v = &vertex0 - _vertexArray;
      Cell& cell = _cellList[i_v];
      if (constraintMgr.satisfied(cell, 0, 1)) {
        moveInfo = FMMoveInfo(i_v, cell, 0, 1);
        return &vertex0;
      }
    } else if (gain0 < gain1) {
      Vertex& vertex1 = _gainBucketCur[1].getNext();
      i_v = &vertex1 - _vertexArray;
      Cell& cell = _cellList[i_v];
      if (constraintMgr.satisfied(cell, 1, 0)) {
        moveInfo = FMMoveInfo(i_v, cell, 1, 0);
        return &vertex1;
      }
    } else { // gain0 == gain1, tie break randomly
      Vertex& vertex0 = _gainBucketCur[0].getNext();
      Vertex& vertex1 = _gainBucketCur[1].getNext();
      const unsigned int i_v0 = &vertex0 - _vertexArray;
      const unsigned int i_v1 = &vertex1 - _vertexArray;
      Cell& cell0 = _cellList[i_v0];
      Cell& cell1 = _cellList[i_v1];
      const bool ok0 = constraintMgr.satisfied(cell0, 0, 1);
      const bool ok1 = constraintMgr.satisfied(cell1, 1, 0);
      if (ok0 && !ok1) {
        moveInfo = FMMoveInfo(i_v0, cell0, 0, 1);
        return &vertex0;
      }
      if (ok1 && !ok0) {
        moveInfo = FMMoveInfo(i_v1, cell1, 1, 0);
        return &vertex1;
      }
      if (!ok0 && !ok1) continue;
      if (rand()&1==0) {
        moveInfo = FMMoveInfo(i_v0, cell0, 0, 1);
        return &vertex0;
      } else {
        moveInfo = FMMoveInfo(i_v1, cell1, 1, 0);
        return &vertex1;
      }
    }
  }

  return 0;
}


