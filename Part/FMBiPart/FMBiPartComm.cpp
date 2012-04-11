#include "FMBiPartComm.hpp"
#include "FMParam.hpp"
#include <Netlist.hpp>
#include <GDList.hpp>
#include <vector>
#include <cassert>  //added by Zhou

using std::vector;

/** Constructor */
FMBiPartComm::FMBiPartComm(const FMParam& param) : 
  FMBiPartMgrBase(param),
  _gain(param._H.getNumCells()) 
{ 
  getNetlist().initVertexArray(); 
}


/** Initialize the gain structure (c.f. Sarrafzadeh and Wong, page 38) */
void FMBiPartComm::initGain(const Net& e, const vector<unsigned char>& part)
{
  CellConstCursor nc(e);
  assert(nc.count() >= 2);
  if (nc.count() == 2) {     // special handle for two-pin nets
    const unsigned int weight = e.getWeight();  // weigth(e)
    const unsigned int i_w1 = &nc.getNext() - _cellList;
    const unsigned int i_w2 = &nc.getNext() - _cellList;
    if (part[i_w1] == part[i_w2]) { // on the same partition
      _gain[i_w1] -= weight;
      _gain[i_w2] -= weight;
    } else {
      _gain[i_w1] += weight;
      _gain[i_w2] += weight;
    }
    return;
  }

  // Calculate the number of modules on each partition
  _num[0] = _num[1] = 0;
  unsigned int degree = 0;
  while (nc.hasNext()) {
    const Cell& w = nc.getNext();
    const unsigned int i_w = &w - _cellList;
    ++_num[part[i_w]];
    if (w.isVisited()) continue; // skip the unmovable cells
    _IdVec[degree] = i_w;        // cache the neighbor information
    ++degree;
  }

  const unsigned int weight = e.getWeight();  // weigth(e)
  for (unsigned int k=0; k<2; ++k) {
    if (_num[k] == 0) {
      // The gain of each module w (they are all not in part k) is decreased
      // by weight(e). When w is moved to the other partition, the
      // cut-cost is increased by weight(e).
      for (unsigned int idx=0; idx<degree; ++idx) {
        const unsigned int i_w = _IdVec[idx];
        _gain[i_w] -= weight;
      }
    }
    else if (_num[k] == 1) {
      // The gain of the one module that is in part k is increased.
      for (unsigned int idx=0; idx<degree; ++idx) {
        const unsigned int i_w = _IdVec[idx];
        if (part[i_w] == k) {
          _gain[i_w] += weight;
          break;
        }
      }
    }
  }
}



/** When the cell v is moved from fromPart, update the gains and the
    gain structures of its neighbors. (c.f. Sarrafzadeh and Wong, 
    page 39)  */ 
template <class GainStruct>
void FMBiPartComm::
updateMove2PinNet(
  const Net& e,                      ///< net connected to the moved cell
  const vector<unsigned char>& part, ///< current partition solution
  unsigned int fromPart,             ///< from where v moves
  const Cell& v,                     ///< the moving cell
  GainStruct& ms                     ///< gain related structre
  )
{
  // Calculate the number of modules on each partition
  CellConstCursor nc(e);
  assert(nc.count() == 2);
  // special handle for two-pin nets
  const Cell* w = &nc.getNext();
  if (w == &v) w = &nc.getNext();
  if (w->isVisited()) return;

  const unsigned int i_w = w - _cellList;
  const unsigned int part_w = part[i_w];
  const int weight = e.getWeight();
  const int deltaGainW = (part_w == fromPart)? 2*weight : -2*weight;

  ms.updateNeighbor(part_w, i_w, deltaGainW);
}


/** When the cell v is moved from fromPart, update the gains and the
    gain structures of its neighbors. (c.f. Sarrafzadeh and Wong, 
    page 39)  */ 
template <class GainStruct>
void FMBiPartComm::
updateMoveGeneralNet(
  const Net& e,                      ///< net connected to the moved cell
  const vector<unsigned char>& part, ///< current partition solution
  unsigned int fromPart,             ///< from where v moves
  const Cell& v,                     ///< the moving cell
  GainStruct& ms                     ///< gain related structre
  )
{
  CellConstCursor nc(e);
  assert(nc.count() > 2);
  unsigned int degree = 0;
  while (nc.hasNext()) {
    const Cell& w = nc.getNext();
    if (&w == &v) continue;           // skip the moved cell
    if (w.isVisited()) continue;      // skip the unmovable cells
    _IdVec[degree] = &w - _cellList;  // cache the neighbor cells id
    _deltaGain[degree] = 0;           // init deltaGain
    ++degree;                         // count how many neigbor cells 
  }

  assert(degree <= 32768);
  if (degree == 0) return;

  int weight = (fromPart == 0) ? e.getWeight() : -e.getWeight();
  unsigned int idx;
  for (unsigned int k=0; k<2; ++k) {
    if (_num[k] == 0) {
      for (idx=0; idx<degree; ++idx) {
    	_deltaGain[idx] -= weight;
      }
    }
    else if (_num[k] == 1) { // only one cell in part k
      for (idx=0; idx<degree; ++idx) { // look for this cell
    	const unsigned int part_w = part[_IdVec[idx]];
    	if (part_w == k) {
    	  _deltaGain[idx] += weight;
    	  break;
    	}
      }
    }
    weight = -weight;
  }

  for (idx=0; idx<degree; ++idx) {
    const unsigned int i_w = _IdVec[idx];
    ms.updateNeighbor(part[i_w], i_w, _deltaGain[idx]);
  }
}



#include "FMBiGainMgr.hpp"
#include "FMBiGainMgr2.hpp"
#include "FMBiGainMgr3.hpp"
#include "FMBiGainArray.hpp"
#include "FMBiMoveStruct.hpp"

/** Explicitly instantiation */

template void FMBiPartComm::updateMove2PinNet(
  const Net& e,
  const vector<unsigned char>& part, 
  unsigned int fromPart, 
  const Cell& v, 
  FMBiGainArray& s);

template void FMBiPartComm::updateMove2PinNet(
  const Net& e,
  const vector<unsigned char>& part, 
  unsigned int fromPart, 
  const Cell& v, 
  FMBiGainMgr& s);

template void FMBiPartComm::updateMove2PinNet(
  const Net& e,
  const vector<unsigned char>& part, 
  unsigned int fromPart, 
  const Cell& v, 
  FMBiGainMgr2& s);

template void FMBiPartComm::updateMove2PinNet(
  const Net& e,
  const vector<unsigned char>& part, 
  unsigned int fromPart, 
  const Cell& v, 
  FMBiGainMgr3& s);

template void FMBiPartComm::updateMove2PinNet(
  const Net& e,
  const vector<unsigned char>& part, 
  unsigned int fromPart, 
  const Cell& v, 
  FMBiMoveStruct<FMBiGainMgr, FMBiGainArray>& s);

template void FMBiPartComm::updateMove2PinNet(
  const Net& e,
  const vector<unsigned char>& part, 
  unsigned int fromPart, 
  const Cell& v, 
  FMBiMoveStruct<FMBiGainMgr2, FMBiGainArray>& s);

template void FMBiPartComm::updateMove2PinNet(
  const Net& e,
  const vector<unsigned char>& part, 
  unsigned int fromPart, 
  const Cell& v, 
  FMBiMoveStruct<FMBiGainMgr3, FMBiGainArray>& s);

template void FMBiPartComm::updateMoveGeneralNet(
  const Net& e,
  const vector<unsigned char>& part, 
  unsigned int fromPart, 
  const Cell& v, 
  FMBiGainArray& s);

template void FMBiPartComm::updateMoveGeneralNet(
  const Net& e,
  const vector<unsigned char>& part, 
  unsigned int fromPart, 
  const Cell& v, 
  FMBiGainMgr& s);

template void FMBiPartComm::updateMoveGeneralNet(
  const Net& e,
  const vector<unsigned char>& part, 
  unsigned int fromPart, 
  const Cell& v, 
  FMBiGainMgr2& s);

template void FMBiPartComm::updateMoveGeneralNet(
  const Net& e,
  const vector<unsigned char>& part, 
  unsigned int fromPart, 
  const Cell& v, 
  FMBiGainMgr3& s);

template void FMBiPartComm::updateMoveGeneralNet(
  const Net& e,
  const vector<unsigned char>& part, 
  unsigned int fromPart, 
  const Cell& v, 
  FMBiMoveStruct<FMBiGainMgr, FMBiGainArray>& s);

template void FMBiPartComm::updateMoveGeneralNet(
  const Net& e,
  const vector<unsigned char>& part, 
  unsigned int fromPart, 
  const Cell& v, 
  FMBiMoveStruct<FMBiGainMgr2, FMBiGainArray>& s);

template void FMBiPartComm::updateMoveGeneralNet(
  const Net& e,
  const vector<unsigned char>& part, 
  unsigned int fromPart, 
  const Cell& v, 
  FMBiMoveStruct<FMBiGainMgr3, FMBiGainArray>& s);
