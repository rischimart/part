#include "FMBiGainMgr.hpp"
#include "BiConstrMgr3.hpp"
#include "FMParam.hpp"
#include <Netlist.hpp>
#include <GBPQueue.hpp>
#include <vector>

using std::vector;

typedef GNode Vertex;

/** Constructor */
FMBiGainMgr::FMBiGainMgr(const FMParam& param) : 
  _H(param._H),
  _vertexArray(_H.getVertexArray()),
  _cellList(&_H.getCell(0))
{
  const int pmax = _H.getMaxDegree();  // maximum pins of cells among all cells
  _gainBucket.setUp(-pmax, pmax);     // the gain is bound by [-pmax, pmax]
  _gainBucketCur.associate(_gainBucket);
}


/** Initialize the gain bucket */
void FMBiGainMgr::init(const vector<short>& gain)
{
  const unsigned int n = _H.getNumCells();
  vector<unsigned int> vaRandIdx(n);
  unsigned int i;
  for (i=0; i<n; ++i) vaRandIdx[i] = i;
  std::random_shuffle(vaRandIdx.begin(), vaRandIdx.end());

  _gainBucket.clear();

  if (_H.hasFixedCells()) {
    for (i=0; i<n; ++i) {
      const unsigned int i_v = vaRandIdx[i];
      if (_cellList[i_v].isVisited()) continue;
      _gainBucket.pushBackFast(_vertexArray[i_v], gain[i_v]);
    }
  } else { // don't need to check one by one
    for (i=0; i<n; ++i) {
      const unsigned int i_v = vaRandIdx[i];
      _gainBucket.pushBackFast(_vertexArray[i_v], gain[i_v]);
    }
  }

  _gainBucket.resync();
}



// @return vertex for next move. @return NULL if no move is allowed.
Vertex* FMBiGainMgr::select(FMMoveInfo& moveInfo,
                            const vector<unsigned char>& part, 
                            const BiConstrMgr3& constraintMgr)
{
  _gainBucketCur.reset();

  while (_gainBucketCur.hasNext()) {
    Vertex& vertex = _gainBucketCur.getNext();
    const unsigned int i_v = &vertex - _vertexArray;
    const unsigned int fromPart = part[i_v];
    const unsigned int toPart = fromPart^1;
    Cell& cell = _cellList[i_v];
    const bool ok = constraintMgr.satisfied(cell, fromPart, toPart);
    if (ok) {
      moveInfo = FMMoveInfo(i_v, cell, fromPart, toPart);
      return &vertex;
    }
  }

  return 0;
}


