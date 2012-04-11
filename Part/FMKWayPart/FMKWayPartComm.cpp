#include "FMKWayPartComm.hpp"
#include "FMKWayGainArray.hpp"
#include <Netlist.hpp>
#include <iostream>
#include <vector>
#include <cassert>  //added by Zhou
#include <cmath> //for abs
#include <algorithm>

using std::vector;
using std::max;
using std::min;
using std::cout;
using std::endl;
/** Constructor */
FMKWayPartComm::FMKWayPartComm(const FMParam& param) :
  FMKWayPartMgrBase(param)
{
  for (unsigned int k=0; k<getNumPartitions(); ++k) {
    _gain[k].resize(getNumCells());
  }
  getNetlist().initVertexArray();
}



/** @return the cell degree of net e. At the same time, cache the
    neighbor information */
inline void FMKWayPartComm::
initGainCalcDegree(const Net& e, const vector<unsigned char>& part)
{
  _degree = 0;
  CellConstCursor nc(e);
  while (nc.hasNext()) {
    const Cell& w = nc.getNext();
    const unsigned int i_w = &w - _cellList;
    ++_num[part[i_w]];
    if (w.isVisited()) continue;
    _IdVec[_degree] = i_w;
    ++_degree;
  }

  assert(_degree <= 32768);
}


inline void FMKWayPartComm::
updateMoveCalcDegree(const Net& e,
		     const Cell& v)
{
  CellConstCursor nc(e);
  cout << "nc.count = " << nc.count() << endl;
  assert(nc.count() >= 2);
  _degree = 0;
  while (nc.hasNext()) {
    const Cell& w = nc.getNext();
    if (&w == &v) continue;
    if (w.isVisited()) continue;  // filter cells that don't need to be updated
    _IdVec[_degree] = &w - _cellList;
    std::fill_n(_deltaGain[_degree], getNumPartitions(), short(0));
    ++_degree;
  }

  assert(_degree <= 32768);
}



/** Setting the initial gain for each partition contributed by two-pin
    net. */
inline void FMKWayPartComm::
initGain2PinNet(const Net& twoPinNet,
                const vector<unsigned char>& part, int weight)
{
  CellConstCursor nc(twoPinNet);
  assert(nc.count() == 2);

  const unsigned int i_w1 = &nc.getNext() - _cellList;
  const unsigned int i_w2 = &nc.getNext() - _cellList;
  const unsigned int part_w1 = part[i_w1];
  const unsigned int part_w2 = part[i_w2];

  if (part_w1 == part_w2) {
    for (unsigned int k=0; k<getNumPartitions(); ++k) {
      _gain[k][i_w1] -= weight;
      _gain[k][i_w2] -= weight;
    }
  } else {
    _gain[part_w2][i_w1] += weight;
    _gain[part_w1][i_w2] += weight;
  }
  ++_num[part_w1];
  ++_num[part_w2];
}


/** Update the neighbors structure according to the move. */
template <class MoveStruct>
void FMKWayPartComm::
updateMove2PinNet(const Net& e,
                  const vector<unsigned char>& part,
                  const FMMoveInfo& moveInfo,
                  int weight,  // net weight (double in SOED metric)
                  MoveStruct& ms)
{
  CellConstCursor nc(e);
  assert(nc.count() == 2);       // special handle for two-pin nets
  const Cell* w = &nc.getNext(); // get the first cell
  if (w == moveInfo._cell) {     // if it is the moving cell,
    w = &nc.getNext();           // then get the opposite cell
  }
  if (w->isVisited()) return;

  const unsigned int i_w = w - _cellList;
  const unsigned int part_w = part[i_w];
  const unsigned int fromPart = moveInfo._fromPart;
  const unsigned int toPart = moveInfo._toPart;

  unsigned int k;

  // Update the gain of the neighbor cell
  vector<short> deltaGainW(getNumPartitions());
  if (part_w == fromPart) {
    for (k=0; k<getNumPartitions(); ++k) {
      deltaGainW[k] += weight;
    }
  } else if (part_w == toPart) {
    for (k=0; k<getNumPartitions(); ++k) {
      deltaGainW[k] -= weight;
    }
  }
  deltaGainW[fromPart] -= weight;
  deltaGainW[toPart] += weight;

  ms.updateNeighbor(part_w, i_w, &deltaGainW[0]);
}



/** Update the structures (both neighbors and the moving cell)
    according to the move. */
template <class MoveStruct>
void FMKWayPartComm::
updateMove2PinNet4(const Net& e,
                   const vector<unsigned char>& part,
                   const FMMoveInfo& moveInfo,
                   int weight,  // net weight (double in SOED metric)
                   MoveStruct& ms)
{
  CellConstCursor nc(e);
  assert(nc.count() == 2);       // special handle for two-pin nets
  const Cell* w = &nc.getNext(); // get the first cell
  if (w == moveInfo._cell) {     // if it is the moving cell,
    w = &nc.getNext();           // then get the opposite cell
  }

  const unsigned int i_w = w - _cellList;
  const unsigned int part_w = part[i_w];
  const unsigned int fromPart = moveInfo._fromPart;
  const unsigned int toPart = moveInfo._toPart;

  unsigned int k;

  // Update the gain of the moving cell first
  if (part_w == fromPart) {
    for (k=0; k<getNumPartitions(); ++k) {
      _deltaGainV[k] += weight;
    }
  } else if (part_w == toPart) {
    for (k=0; k<getNumPartitions(); ++k) {
      _deltaGainV[k] -= weight;
    }
  }

  if (w->isVisited()) return;

  // Update the gain of the neighbor cell
  vector<short> deltaGainW(getNumPartitions());
  if (part_w == fromPart) {
    for (k=0; k<getNumPartitions(); ++k) {
      deltaGainW[k] += weight;
    }
  } else if (part_w == toPart) {
    for (k=0; k<getNumPartitions(); ++k) {
      deltaGainW[k] -= weight;
    }
  }
  deltaGainW[fromPart] -= weight;
  deltaGainW[toPart] += weight;

  ms.updateNeighbor(part_w, i_w, &deltaGainW[0]);
}



void FMKWayPartComm::
initGainHEdge(const Net& e, const vector<unsigned char>& part)
{
  CellConstCursor nc(e);
  assert(nc.count() >= 2);
  if (nc.count() == 2) { // special handle for two-pin nets
    initGain2PinNet(e, part, e.getWeight());
  } else {
    std::fill_n(_num, getNumPartitions(), 0);
    initGainGeneralNetHEdge(e, part);
  }
}


/** Setting the initial gain for each partition. Here, we use the
    HyperEdge metric cost model. */
inline void FMKWayPartComm::
initGainGeneralNetHEdge(const Net& e, const vector<unsigned char>& part)
{
  initGainCalcDegree(e, part);
  _numNonEmpty = 0;
  unsigned int k=0;
  for (k=0; k<getNumPartitions(); ++k) {
    if (_num[k] > 0) ++_numNonEmpty;
    if (_numNonEmpty > 2) return;
  }

  const unsigned int iw1 = _IdVec[0];
  const unsigned int partw1 = part[iw1];
  const int weight = e.getWeight();
  unsigned int idx;

  if (_numNonEmpty == 1) {
    for (k=0; k<getNumPartitions(); ++k) {
      if (k == partw1) continue;
      for (idx=0; idx<_degree; ++idx) {
        _gain[k][_IdVec[idx]] -= weight;
      }
    }
    return;
  }

  unsigned int iw2 = 9999;
  unsigned int partw2 = 9999;

  CellConstCursor nc(e);
  const Cell* w2Ptr = 0;
  while (1) {
    assert(nc.hasNext());
    w2Ptr = &nc.getNext();
    iw2 = w2Ptr - _cellList;
    partw2 = part[iw2];
    if (partw1 != partw2) break;
  }

  if (_num[partw1] == 1) _gain[partw2][iw1] += weight;
  if (_num[partw2] == 1 && !w2Ptr->isVisited()) _gain[partw1][iw2] += weight;
}




/** Update the structure according to the move. The gains are measured
    in hyperedge metric. Note that I can think
    of other possible ways to perform the same function. However, don't
    know which way can be the fastest one. */
template <class MoveStruct>
void FMKWayPartComm::
updateMoveGeneralNetHEdge4(const Net& e,
			   const vector<unsigned char>& part,
                           const FMMoveInfo& moveInfo,
			   MoveStruct& ms)
{
  const unsigned int fromPart = moveInfo._fromPart;
  const unsigned int toPart = moveInfo._toPart;
  if (_num[fromPart] > 2 && _num[toPart] > 1) return;

  updateMoveGeneralNetCommHEdge(e, part, moveInfo, ms);

  // Update the gain of the moving cell
  if (_num[fromPart] == 0) {
    if ( _num[toPart] > 1) {
      if (_numNonEmpty == 1) {
        const unsigned int weight = e.getWeight();
        for (unsigned int k=0; k<getNumPartitions(); ++k) {
          _deltaGainV[k] -= weight;
        }
      }
    }
  } else {
    if (_num[toPart] == 1) {
      if (_numNonEmpty == 2) {
        const unsigned int weight = e.getWeight();
        for (unsigned int k=0; k<getNumPartitions(); ++k) {
          _deltaGainV[k] += weight;
	}
      }
    }
  }
}



/** Update the structure according to the move. The gains are measured
    in hyperedge metric. Note that I can think
    of other possible ways to perform the same function. However, don't
    know which way can be the fastest one. */
template <class MoveStruct>
void FMKWayPartComm::
updateMoveGeneralNetHEdge(const Net& e,
			  const vector<unsigned char>& part,
                          const FMMoveInfo& moveInfo,
			  MoveStruct& ms)
{
  const unsigned int fromPart = moveInfo._fromPart;
  const unsigned int toPart = moveInfo._toPart;
  if (_num[fromPart] > 2 && _num[toPart] > 1) return;
  updateMoveGeneralNetCommHEdge(e, part, moveInfo, ms);
}



template <class MoveStruct>
inline void FMKWayPartComm::
updateMoveGeneralNetCommHEdge(const Net& e,
			      const vector<unsigned char>& part,
                              const FMMoveInfo& moveInfo,
			      MoveStruct& ms)
{
  unsigned int k;
  _numNonEmpty = 0;

  for (k=0; k<getNumPartitions(); ++k) {
    assert(_num[k] >= 0);
    if (_num[k] > 0) ++_numNonEmpty;
    if (_numNonEmpty > 3) return;
  }

  updateMoveCalcDegree(e, *moveInfo._cell);
  //xxx if (_degree == 0) return;

  unsigned int idx;
  const unsigned int fromPart = moveInfo._fromPart;
  const unsigned int toPart = moveInfo._toPart;
  const unsigned int weight = e.getWeight();

  if (_numNonEmpty == 1) {
    const unsigned int partw = part[_IdVec[0]];
    for (k=0; k<getNumPartitions(); ++k) {
      if (k == partw) continue;
      for (idx=0; idx<_degree; ++idx) {
        _deltaGain[idx][k] += weight;
      }
    }
  } else if (_numNonEmpty == 2) {
    for (idx=0; idx<_degree; ++idx) {
      const unsigned int partw = part[_IdVec[idx]];
      if (partw != fromPart && _num[partw] == 1) {
        _deltaGain[idx][fromPart] -= weight;
        break;
      }
    }
  }

  if (_num[toPart] == 0) ++_numNonEmpty;
  ++_num[toPart];
  --_num[fromPart];
  if (_num[fromPart] == 0) --_numNonEmpty;

  if (_numNonEmpty == 1) {
    const unsigned int partw = part[_IdVec[0]];
    for (k=0; k<getNumPartitions(); ++k) {
      if (k == partw) continue;
      for (idx=0; idx<_degree; ++idx) {
        _deltaGain[idx][k] -= weight;
      }
    }
  } else if (_numNonEmpty == 2) {
    for (idx=0; idx<_degree; ++idx) {
      const unsigned int partw = part[_IdVec[idx]];
      if (partw != toPart && _num[partw] == 1) {
        _deltaGain[idx][toPart] += weight;
        break;
      }
    }
  }

  for (idx=0; idx<_degree; ++idx) {
    const unsigned int i_w = _IdVec[idx];
    ms.updateNeighbor(part[i_w], i_w, _deltaGain[idx]);
  }
}



/** Note that the code for (K-1) metric looks similar to the
    bi-partitioning code. */


/** Setting the initial gain for each net. Here, we use the
    (K-1) metric cost model. */
void FMKWayPartComm::
initGainKMinus1(const Net& e, const vector<unsigned char>& part)
{
  CellConstCursor nc(e);
  assert(nc.count() >= 2);
  if (nc.count() == 2) { // special handle for two-pin nets
    initGain2PinNet(e, part, e.getWeight());
  } else {
    std::fill_n(_num, getNumPartitions(), 0);
    initGainGeneralNetKMinus1(e, part);
  }
}




/** Setting the initial gain for each net. Here, we use the
    (K-1) metric cost model. */
inline void FMKWayPartComm::
initGainGeneralNetKMinus1(const Net& e, const vector<unsigned char>& part)
{
  initGainCalcDegree(e, part);
  const int weight = e.getWeight();

  unsigned int k;
  unsigned int idx;

  for (k=0; k<getNumPartitions(); ++k) {
    if (_num[k] == 0) {
      for (idx=0; idx<_degree; ++idx) {
        _gain[k][_IdVec[idx]] -= weight;
      }
    }
    else if (_num[k] == 1) {
      for (idx=0; idx<_degree; ++idx) {
        const unsigned int i_w = _IdVec[idx];
        if (part[i_w] == k) {
          unsigned int k2;
          for (k2=0; k2<getNumPartitions(); ++k2) {
            if (k2 == k) continue;
            _gain[k2][i_w] += weight;
          }
          break;
        }
      }
    }
  }
}



/** Update the structure according to the move. The gains are measured
    in (K-1) metric. */
template <class MoveStruct>
void FMKWayPartComm::
updateMoveGeneralNetKMinus14(const Net& e,
			    const vector<unsigned char>& part,
                            const FMMoveInfo& moveInfo,
			    MoveStruct& ms)
{
  const unsigned int fromPart = moveInfo._fromPart;
  const unsigned int toPart = moveInfo._toPart;
  if (_num[fromPart] > 1 && _num[toPart] > 1) return;

  updateMoveGeneralNetCommKMinus1(e, part, moveInfo, ms);

  // Update the gain of the moving cell
  if (_num[fromPart] == 0) {
    if (_num[toPart] > 0) {
      const int weight = e.getWeight();
      for (unsigned int k=0; k<getNumPartitions(); ++k) {
        _deltaGainV[k] -= weight;
      }
    }
  } else {
    if (_num[toPart] == 0) {
      const int weight = e.getWeight();
      for (unsigned int k=0; k<getNumPartitions(); ++k) {
        _deltaGainV[k] += weight;
      }
    }
  }

}



/** Update the structure according to the move. The gains are measured
    in (K-1) metric. */
template <class MoveStruct>
void FMKWayPartComm::
updateMoveGeneralNetKMinus1(const Net& e,
			    const vector<unsigned char>& part,
                            const FMMoveInfo& moveInfo,
			    MoveStruct& ms)
{
  const unsigned int fromPart = moveInfo._fromPart;
  const unsigned int toPart = moveInfo._toPart;
  if (_num[fromPart] > 1 && _num[toPart] > 1) return;
  updateMoveGeneralNetCommKMinus1(e, part, moveInfo, ms);
}


/** Update the structure according to the move. The gains are measured
    in (K-1) metric. */
template <class MoveStruct>
inline void FMKWayPartComm::
updateMoveGeneralNetCommKMinus1(const Net& e,
			    const vector<unsigned char>& part,
                            const FMMoveInfo& moveInfo,
			    MoveStruct& ms)
{
  updateMoveCalcDegree(e, *moveInfo._cell);
  if (_degree == 0) return;

  unsigned int idx, k;
  const int weight = e.getWeight();
  const unsigned int fromPart = moveInfo._fromPart;
  const unsigned int toPart = moveInfo._toPart;

  // Look very complicated. Careful!
  if (_num[fromPart] == 0) {
    // Case 2: All modules of e are not in A (that is, right after
    // moving v).
    for (idx=0; idx<_degree; ++idx) {
      _deltaGain[idx][fromPart] -= weight;
    }
  } else if (_num[fromPart] == 1) {
    for (idx=0; idx<_degree; ++idx) {
      const unsigned int part_w = part[_IdVec[idx]];
      if (part_w == fromPart) {
	for (k=0; k<getNumPartitions(); ++k) {
	  if (k == part_w) continue;
	  _deltaGain[idx][k] += weight;
	}
	break;
      }
    }
  }
  if (_num[toPart] == 0) {
    // Case 1: All modules of e are not in B (that is, right before
    // moving v). This is the case because v will be locked in B
    // and moving the other modules of e cannot change the gain
    // (unless all of them are moved).
    for (idx=0; idx<_degree; ++idx) {
      _deltaGain[idx][toPart] += weight;
    }
  } else if (_num[toPart] == 1) {
    for (idx=0; idx<_degree; ++idx) {
      const unsigned int part_w = part[_IdVec[idx]];
      if (part_w == toPart) {
	for (k=0; k<getNumPartitions(); ++k) {
	  if (k == part_w) continue;
	  _deltaGain[idx][k] -= weight;
	}
	break;
      }
    }
  }

  for (idx=0; idx<_degree; ++idx) {
    const unsigned int i_w = _IdVec[idx];
    ms.updateNeighbor(part[i_w], i_w, _deltaGain[idx]);
  }
}




/** Note that SOED cost is equal to the (K-1) cost plus the Hyper Edge cost.*/

void FMKWayPartComm::
initGainSOED(const Net& e, const vector<unsigned char>& part)
{
  CellConstCursor nc(e);
  assert(nc.count() >= 2);
  if (nc.count() == 2) { // special handle for two-pin nets
    initGain2PinNet(e, part, 2*e.getWeight());
  } else {
    std::fill_n(_num, getNumPartitions(), 0);
    initGainGeneralNetSOED(e, part);
  }
}



/** Setting the initial gain for each partition. Here, we use the
    SOED metric cost model. */
inline void FMKWayPartComm::
initGainGeneralNetSOED(const Net& e, const vector<unsigned char>& part)
{
  initGainCalcDegree(e, part);
  unsigned int k=0;
  unsigned int idx;
  _numNonEmpty = 0;

  for (k=0; k<getNumPartitions(); ++k) {
    if (_num[k] > 0) ++_numNonEmpty;
    if (_numNonEmpty > 2) break;
  }

  const int weight = e.getWeight();
  const int weight2 = 2 * weight;
  const int weight3 = (_numNonEmpty == 1)? weight2 : weight;
  const int weight4 = (_numNonEmpty == 2)? weight2 : weight;

  for (k=0; k<getNumPartitions(); ++k) {
    if (_num[k] == 0) {
      for (idx=0; idx<_degree; ++idx) {
        _gain[k][_IdVec[idx]] -= weight3;
      }
    }
    else if (_num[k] == 1) {
      for (idx=0; idx<_degree; ++idx) {
        const unsigned int i_w = _IdVec[idx];
        if (part[i_w] == k) {
          unsigned int k2;
          for (k2=0; k2<getNumPartitions(); ++k2) {
            if (k2 == k) continue;
            if (_num[k2] > 0) {
              _gain[k2][i_w] += weight4;
            } else {
              _gain[k2][i_w] += weight;
            }
          }
          break;
        }
      }
    }
  }
}



/** Update the structure according to the move. The gains are measured
    in SOED metric. */
template <class MoveStruct>
void FMKWayPartComm::
updateMoveGeneralNetSOED(const Net& e,
			 const vector<unsigned char>& part,
                         const FMMoveInfo& moveInfo,
			 MoveStruct& ms)
{
  const unsigned int fromPart = moveInfo._fromPart;
  const unsigned int toPart = moveInfo._toPart;
  if (_num[fromPart] > 2 && _num[toPart] > 1) return; // ???
  updateMoveGeneralNetCommSOED(e, part, moveInfo, ms);
}




/** Update the structure according to the move. The gains are measured
    in SOED metric. */
template <class MoveStruct>
void FMKWayPartComm::
updateMoveGeneralNetSOED4(const Net& e,
			 const vector<unsigned char>& part,
                         const FMMoveInfo& moveInfo,
			 MoveStruct& ms)
{
  const unsigned int fromPart = moveInfo._fromPart;
  const unsigned int toPart = moveInfo._toPart;
  if (_num[fromPart] > 2 && _num[toPart] > 1) return; // ???

  updateMoveGeneralNetCommSOED(e, part, moveInfo, ms);

  if (_num[fromPart] == 0) {
    if ( _num[toPart] > 1) {
      const int weight = e.getWeight();
      const int weight5 = (_numNonEmpty == 1)? 2*weight : weight;
      for (unsigned int k=0; k<getNumPartitions(); ++k) {
        if (k==fromPart || k==toPart) continue;
        //xxx assert(_num[k] == 0);
        _deltaGainV[k] -= weight5;
      }
    }
  } else {
    if (_num[toPart] == 1) {
      const int weight = e.getWeight();
      const int weight6 = (_numNonEmpty == 2)? 2*weight : weight;
      for (unsigned int k=0; k<getNumPartitions(); ++k) {
        if (k==fromPart || k==toPart) continue;
        //xxx assert(_num[k] == 0);
        _deltaGainV[k] += weight6;
      }
    }
  }
}



template <class MoveStruct>
inline void FMKWayPartComm::
updateMoveGeneralNetCommSOED(const Net& e,
			 const vector<unsigned char>& part,
                         const FMMoveInfo& moveInfo,
			 MoveStruct& ms)
{
  unsigned int k;
  _numNonEmpty = 0;
  for (k=0; k<getNumPartitions(); ++k) {
    if (_num[k] > 0) ++_numNonEmpty;
    if (_numNonEmpty > 3) break;
  }

  updateMoveCalcDegree(e, *moveInfo._cell);
  //xxx if (_degree == 0) return;

  const int weight = e.getWeight();
  const unsigned int fromPart = moveInfo._fromPart;
  const unsigned int toPart = moveInfo._toPart;

  unsigned int idx;

  // Hyper-Edge cost
  if (_numNonEmpty == 1) {
    const unsigned int partw = part[_IdVec[0]];
    for (k=0; k<getNumPartitions(); ++k) {
      if (k == partw) continue;
      for (idx=0; idx<_degree; ++idx) {
        _deltaGain[idx][k] += weight;
      }
    }
  } else if (_numNonEmpty == 2) {
    for (idx=0; idx<_degree; ++idx) {
      const unsigned int partw = part[_IdVec[idx]];
      if (partw != fromPart && _num[partw] == 1) {
        _deltaGain[idx][fromPart] -= weight;
        break;
      }
    }
  }

  --_num[fromPart];
  if (_num[fromPart] == 0) --_numNonEmpty;

  // K-1 cost
  if (_num[fromPart] == 0) {
    // Case 2: All modules of e are not in A (that is, right after
    // moving v).
    for (idx=0; idx<_degree; ++idx) {
      _deltaGain[idx][fromPart] -= weight;
    }
  } else if (_num[fromPart] == 1) {
    for (idx=0; idx<_degree; ++idx) {
      const unsigned int part_w = part[_IdVec[idx]];
      if (part_w == fromPart) {
  	for (k=0; k<getNumPartitions(); ++k) {
  	  if (k == part_w) continue;
  	  _deltaGain[idx][k] += weight;
  	}
  	break;
      }
    }
  }
  if (_num[toPart] == 0) {
    // Case 1: All modules of e are not in B (that is, right before
    // moving v). This is the case because v will be locked in B
    // and moving the other modules of e cannot change the gain
    // (unless all of them are moved).
    for (idx=0; idx<_degree; ++idx) {
      _deltaGain[idx][toPart] += weight;
    }
  } else if (_num[toPart] == 1) {
    for (idx=0; idx<_degree; ++idx) {
      const unsigned int part_w = part[_IdVec[idx]];
      if (part_w == toPart) {
  	for (k=0; k<getNumPartitions(); ++k) {
  	  if (k == part_w) continue;
  	  _deltaGain[idx][k] -= weight;
  	}
  	break;
      }
    }
  }

  if (_num[toPart] == 0) ++_numNonEmpty;
  ++_num[toPart];

  // Hyper-Edge cost
  if (_numNonEmpty == 1) {
    const unsigned int partw = part[_IdVec[0]];
    for (k=0; k<getNumPartitions(); ++k) {
      if (k == partw) continue;
      for (idx=0; idx<_degree; ++idx) {
        _deltaGain[idx][k] -= weight;
      }
    }
  } else if (_numNonEmpty == 2) {
    for (idx=0; idx<_degree; ++idx) {
      const unsigned int partw = part[_IdVec[idx]];
      if (partw != toPart && _num[partw] == 1) {
        _deltaGain[idx][toPart] += weight;
        break;
      }
    }
  }

  for (idx=0; idx<_degree; ++idx) {
    const unsigned int i_w = _IdVec[idx];
    ms.updateNeighbor(part[i_w], i_w, _deltaGain[idx]);
  }
}




/** Setting the initial gain for each net. Here, we use the
    TSV metric cost model. */
void FMKWayPartComm::
initGainTSV(const Net& e, const vector<unsigned char>& part)
{
  CellConstCursor nc(e);
  assert(nc.count() >= 2);
  std::fill_n(_num, getNumPartitions(), 0);
  initGainGeneralNetTSV(e, part);

}


/** Setting the initial gain for each partition. Here, we use the
    TSV metric cost model. */
inline void FMKWayPartComm::
initGainGeneralNetTSV(const Net& e, const vector<unsigned char>& part)
{
  initGainCalcDegree(e, part);
  _numNonEmpty = 0;
  unsigned int k=0;
  vector<unsigned int> parts;
  for (k=0; k<getNumPartitions(); ++k) {
    if (_num[k] > 0) ++_numNonEmpty;
    parts.push_back(k);
  }

  const unsigned int iw1 = _IdVec[0];
  const unsigned int partw1 = part[iw1];
  //const int weight = e.getWeight();
  unsigned int idx;

  if (_numNonEmpty == 1) {
    for (k=0; k<getNumPartitions(); ++k) {
      if (k == partw1) continue;
      for (idx=0; idx<_degree; ++idx) {
        _gain[k][_IdVec[idx]] -= abs(k - partw1);
      }
    }
    return;
  }
  unsigned int upper = parts[_numNonEmpty - 1];
  unsigned int lower = parts[0];
  unsigned int subupp = parts[_numNonEmpty - 2];
  unsigned int sublow = parts[1];

  for (k = 0; k < getNumPartitions(); ++k) {
      if(k < lower) {
        for (idx=0; idx<_degree; ++idx)
          _gain[k][_IdVec[idx]] -= (lower - k);
      }
      if(k > upper) {
        for (idx=0; idx<_degree; ++idx)
          _gain[k][_IdVec[idx]] -= (k - upper);
      }
      if(k < upper) {
        for (idx=0; idx<_degree; ++idx)
          if(part[_IdVec[idx]] == upper && _num[upper] == 1)
            _gain[k][_IdVec[idx]] += (upper - max(k, subupp));
      }
      if( k > lower) {
          for (idx=0; idx<_degree; ++idx)
            if(part[_IdVec[idx]] == lower && _num[lower] == 1)
              _gain[k][_IdVec[idx]] += (min(k, sublow) - lower);
      }
  }

}


/** Update the structure according to the move. The gains are measured
    in TSV metric.  */
template <class MoveStruct> // todo
void FMKWayPartComm::
updateMoveGeneralNetTSV4(const Net& e,
			   const vector<unsigned char>& part,
                           const FMMoveInfo& moveInfo,
			   MoveStruct& ms)
{
  const unsigned int fromPart = moveInfo._fromPart;
  const unsigned int toPart = moveInfo._toPart;
  if (_num[fromPart] > 1 && _num[toPart] > 1) return;

  updateMoveGeneralNetCommTSV(e, part, moveInfo, ms);
    /*
  // Update the gain of the moving cell
  if (_num[fromPart] == 0) {
    if ( _num[toPart] > 1) {
      if (_numNonEmpty == 1) {
        const unsigned int weight = e.getWeight();
        for (unsigned int k=0; k<getNumPartitions(); ++k) {
          _deltaGainV[k] -= weight;
        }
      }
    }
  } else {
    if (_num[toPart] == 1) {
      if (_numNonEmpty == 2) {
        const unsigned int weight = e.getWeight();
        for (unsigned int k=0; k<getNumPartitions(); ++k) {
          _deltaGainV[k] += weight;
	}
      }
    }
  }*/
}



/** Update the structure according to the move. The gains are measured
    in TSV metric.  */
template <class MoveStruct> // in progress
void FMKWayPartComm::
updateMoveGeneralNetTSV(const Net& e,
			  const vector<unsigned char>& part,
                          const FMMoveInfo& moveInfo,
			  MoveStruct& ms)
{
  const unsigned int fromPart = moveInfo._fromPart;
  const unsigned int toPart = moveInfo._toPart;
  if (_num[fromPart] > 1 && _num[toPart] > 1) return;
  updateMoveGeneralNetCommTSV(e, part, moveInfo, ms);
}



template <class MoveStruct>
inline void FMKWayPartComm::
updateMoveGeneralNetCommTSV(const Net& e,
			      const vector<unsigned char>& part,
                              const FMMoveInfo& moveInfo,
			      MoveStruct& ms)  //in progress
{
  unsigned int _numNonEmpty = 0;
  vector<unsigned int> parts;
  for (int k = 0; k < getNumPartitions(); ++k) {
    if (_num[k] > 0) {
        _numNonEmpty++;
        parts.push_back(k);
    }
  }

  updateMoveCalcDegree(e, *moveInfo._cell);
  if (_degree == 0) return;

  unsigned int idx, k;
  //const int weight = e.getWeight();
  const unsigned int fromPart = moveInfo._fromPart;
  const unsigned int toPart = moveInfo._toPart;

  if(_numNonEmpty == 1) {
    if(toPart > fromPart) {
      for (k = fromPart + 1; k < getNumPartitions(); ++k) {
        for (idx=0; idx<_degree; ++idx)
          _deltaGain[idx][k] += min(k, toPart) - fromPart;
          if(_num[fromPart] == 1)
            _deltaGain[idx][k] += min(k, toPart) - fromPart;
        }
    }
    else {
      for (k = 0; k < fromPart; ++k)  {
        for (idx=0; idx<_degree; ++idx)
          _deltaGain[idx][k] += fromPart - max(k, toPart);
          if(_num[fromPart] == 1)
            _deltaGain[idx][k] += fromPart - max(k, toPart);
      }
    }
    for (idx=0; idx<_degree; ++idx) {
      const unsigned int i_w = _IdVec[idx];
      ms.updateNeighbor(part[i_w], i_w, _deltaGain[idx]);
    }
    return;
  }
  unsigned int upper = parts[_numNonEmpty - 1];
  unsigned int lower = parts[0];
  unsigned int subupp = parts[_numNonEmpty - 2];
  unsigned int sublow = parts[1];

  if(fromPart > parts[_numNonEmpty - 1]) {
    upper = fromPart;
    subupp = parts[_numNonEmpty - 1];
  }
  if(fromPart < parts[0]) {
    lower = fromPart;
    sublow = parts[0];
  }

  if (_num[toPart] == 0) {
    if(toPart > upper) {
      for (k = upper + 1; k < getNumPartitions(); ++k) {
        for (idx=0; idx<_degree; ++idx)
          _deltaGain[idx][k] += (min(k, toPart) - upper);
      }
    }
    else if(toPart < lower) {
      for (k = 0; k < lower; ++k) {
        for (idx=0; idx<_degree; ++idx)
          _deltaGain[idx][k] += (lower - max(k, toPart));
      }
    }
    else {
      if(_numNonEmpty == 2) {
        if(fromPart == lower && _num[upper] == 1) {
          for (k = 0; k < toPart; ++k) {
            for (idx=0; idx<_degree; ++idx) {
              if(part[_IdVec[idx]] == upper) {
                _deltaGain[idx][k] -= (toPart - max(k, lower));
                break;
              }
            }
          }
        }
        if(fromPart == upper && _num[lower] == 1 ) {
          for (k = toPart + 1; k < getNumPartitions(); ++k) {
            for (idx=0; idx<_degree; ++idx) {
              if(part[_IdVec[idx]] == lower) {
                _deltaGain[idx][k] -= (min(k, upper) - toPart);
                break;
              }
            }
          }
        }
      }
      else {
        if(toPart > subupp) {
          if(fromPart != upper && _num[upper] == 1) {
            for(k = 0; k < toPart; ++k)
              for(idx = 0; idx < _degree; ++idx) {
                if(part[_IdVec[idx]] == upper) {
                  _deltaGain[idx][k] -= (toPart - max(k, subupp));
                  break;
                }
              }
          }
        }
        if(toPart < sublow) {
          if(fromPart != lower && _num[lower] == 1) {
            for(k = toPart + 1; k < getNumPartitions(); ++k)
              for(idx = 0; idx < _degree; ++idx) {
                if(part[_IdVec[idx]] == lower) {
                  _deltaGain[idx][k] -= (min(k, sublow) - toPart);
                  break;
                }
              }
          }
        }
      }
    }
  }
  else if (_num[toPart] == 1) {
    if(toPart == upper) {
      for(k = 0; k < upper; ++k) {
        for(idx = 0; idx < _degree; ++idx) {
          if(part[_IdVec[idx]] == upper) {
            _deltaGain[idx][k] -= (upper - max(k, subupp));
            break;
          }
        }
      }
    }
    if(toPart == lower) {
      for(k = lower + 1; k < getNumPartitions(); ++k) {
        for(idx = 0; idx < _degree; ++idx) {
          if(part[_IdVec[idx]] == lower) {
            _deltaGain[idx][k] -= (min(k, sublow) - lower);
            break;
          }
        }
      }
    }
  }

  if (_num[fromPart] == 0) {
    if(fromPart == upper && upper > toPart && toPart >= subupp) {
      for(k = toPart + 1; k < getNumPartitions(); ++k) {
        for(idx = 0; idx < _degree; ++idx)
          _deltaGain[idx][k] -= (min(k, upper) - toPart);
      }
    }
    if(fromPart == lower && lower < toPart && toPart <= sublow) {
      for(k = 0; k < toPart; ++k) {
        for(idx = 0; idx < _degree; ++idx)
          _deltaGain[idx][k] -= (toPart - max(k, lower));
      }
    }
    if(_numNonEmpty > 2) {
      if(fromPart == upper && lower <= toPart && toPart < subupp) {
        for(k = subupp + 1; k < getNumPartitions(); ++k) {
          for(idx = 0; idx < _degree; ++idx)
            _deltaGain[idx][k] -= (min(k, upper) - subupp);
        }
      }
      if(fromPart == upper && _num[subupp] == 1 && toPart < subupp) {
        for(k = 0; k < subupp; ++k) {
          for(idx = 0; idx < _degree; ++idx) {
            if(part[_IdVec[idx]] == subupp) {
               unsigned int subsubu = parts[_numNonEmpty - 2];
               _deltaGain[idx][k] += (subupp - max(k, max(subsubu, toPart)));
               break;
            }
          }
        }
      }
      if(fromPart == lower && upper >= toPart && toPart > sublow) {
        for(k = 0; k < sublow; ++k) {
          for(idx = 0; idx < _degree; ++idx)
            _deltaGain[idx][k] -= (sublow - max(k, lower));
        }
      }
      if(fromPart == lower && _num[sublow] == 1 && toPart > sublow) {
        for(k = sublow + 1; k < getNumPartitions(); ++k) {
          for(idx = 0; idx < _degree; ++idx) {
            if(part[_IdVec[idx]] == sublow) {
               unsigned int subsubl = parts[1];
               _deltaGain[idx][k] += (min(k, min(subsubl, toPart)) - sublow);
               break;
            }
          }
        }
      }
    }
  }

  else if (_num[fromPart] == 1) {
    if(fromPart == upper) {
      for(k = 0; k < upper; ++k) {
        for(idx = 0; idx < _degree; ++idx) {
          if(part[_IdVec[idx]] == upper)
            _deltaGain[idx][k] += (upper - max(k, max(subupp, toPart)));
        }
      }
    }
    if(fromPart == lower) {
      for(k = lower + 1; k < getNumPartitions(); ++k) {
        for(idx = 0; idx < _degree; ++idx) {
          if(part[_IdVec[idx]] == lower)
            _deltaGain[idx][k] += (min(k, min(sublow, toPart)) - lower);
        }
      }
    }
  }

  for (idx=0; idx<_degree; ++idx) {
    const unsigned int i_w = _IdVec[idx];
    ms.updateNeighbor(part[i_w], i_w, _deltaGain[idx]);
  }
}



#include "FMKWayGainMgr2.hpp"
#include "FMKWayGainMgr3.hpp"
#include "FMKWayGainMgr4.hpp"
#include "FMKWayMoveStruct.hpp"

/** Explicitly instantiation */
template void FMKWayPartComm::
updateMove2PinNet
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 int weight,
 FMKWayGainMgr2& s);


template void FMKWayPartComm::
updateMove2PinNet
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 int weight,
 FMKWayGainMgr3& s);

template void FMKWayPartComm::
updateMove2PinNet
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 int weight,
 FMKWayGainMgr4& s);


template void FMKWayPartComm::
updateMove2PinNet4
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 int weight,
 FMKWayGainArray& s);

template void FMKWayPartComm::
updateMove2PinNet4
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 int weight,
 FMKWayMoveStruct<FMKWayGainMgr2, FMKWayGainArray>& s);

template void FMKWayPartComm::
updateMove2PinNet4
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 int weight,
 FMKWayMoveStruct<FMKWayGainMgr3, FMKWayGainArray>& s);

template void FMKWayPartComm::
updateMove2PinNet4
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 int weight,
 FMKWayMoveStruct<FMKWayGainMgr4, FMKWayGainArray>& s);



/** Explicitly instantiation */
template void FMKWayPartComm::
updateMoveGeneralNetHEdge
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 FMKWayGainMgr2& s);

template void FMKWayPartComm::
updateMoveGeneralNetHEdge
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 FMKWayGainMgr3& s);

template void FMKWayPartComm::
updateMoveGeneralNetHEdge
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 FMKWayGainMgr4& s);

template void FMKWayPartComm::
updateMoveGeneralNetHEdge4
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 FMKWayGainArray& s);

template void FMKWayPartComm::
updateMoveGeneralNetHEdge4
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 FMKWayMoveStruct<FMKWayGainMgr2, FMKWayGainArray>& s);


template void FMKWayPartComm::
updateMoveGeneralNetHEdge4
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 FMKWayMoveStruct<FMKWayGainMgr3, FMKWayGainArray>& s);

template void FMKWayPartComm::
updateMoveGeneralNetHEdge4
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 FMKWayMoveStruct<FMKWayGainMgr4, FMKWayGainArray>& s);




/** Explicitly instantiation */
template void FMKWayPartComm::
updateMoveGeneralNetKMinus1
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 FMKWayGainMgr2& s);

template void FMKWayPartComm::
updateMoveGeneralNetKMinus1
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 FMKWayGainMgr3& s);

template void FMKWayPartComm::
updateMoveGeneralNetKMinus1
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 FMKWayGainMgr4& s);

template void FMKWayPartComm::
updateMoveGeneralNetKMinus14
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 FMKWayGainArray& s);

template void FMKWayPartComm::
updateMoveGeneralNetKMinus14
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 FMKWayMoveStruct<FMKWayGainMgr2, FMKWayGainArray>& s);

template void FMKWayPartComm::
updateMoveGeneralNetKMinus14
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 FMKWayMoveStruct<FMKWayGainMgr3, FMKWayGainArray>& s);

template void FMKWayPartComm::
updateMoveGeneralNetKMinus14
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 FMKWayMoveStruct<FMKWayGainMgr4, FMKWayGainArray>& s);




/** Explicitly instantiation */
template void FMKWayPartComm::
updateMoveGeneralNetSOED
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 FMKWayGainMgr2& s);

template void FMKWayPartComm::
updateMoveGeneralNetSOED
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 FMKWayGainMgr3& s);

template void FMKWayPartComm::
updateMoveGeneralNetSOED
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 FMKWayGainMgr4& s);

template void FMKWayPartComm::
updateMoveGeneralNetSOED4
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 FMKWayGainArray& s);

template void FMKWayPartComm::
updateMoveGeneralNetSOED4
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 FMKWayMoveStruct<FMKWayGainMgr2, FMKWayGainArray>& s);

template void FMKWayPartComm::
updateMoveGeneralNetSOED4
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 FMKWayMoveStruct<FMKWayGainMgr3, FMKWayGainArray>& s);

template void FMKWayPartComm::
updateMoveGeneralNetSOED4
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 FMKWayMoveStruct<FMKWayGainMgr4, FMKWayGainArray>& s);


/** Explicitly instantiation */
template void FMKWayPartComm::
updateMoveGeneralNetTSV
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 FMKWayGainMgr2& s);

template void FMKWayPartComm::
updateMoveGeneralNetTSV
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 FMKWayGainMgr3& s);

template void FMKWayPartComm::
updateMoveGeneralNetTSV
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 FMKWayGainMgr4& s);

template void FMKWayPartComm::
updateMoveGeneralNetTSV4
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 FMKWayGainArray& s);

template void FMKWayPartComm::
updateMoveGeneralNetTSV4
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 FMKWayMoveStruct<FMKWayGainMgr2, FMKWayGainArray>& s);

template void FMKWayPartComm::
updateMoveGeneralNetTSV4
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 FMKWayMoveStruct<FMKWayGainMgr3, FMKWayGainArray>& s);

template void FMKWayPartComm::
updateMoveGeneralNetTSV4
(const Net& e,
 const vector<unsigned char>& part,
 const FMMoveInfo& moveInfo,
 FMKWayMoveStruct<FMKWayGainMgr4, FMKWayGainArray>& s);



