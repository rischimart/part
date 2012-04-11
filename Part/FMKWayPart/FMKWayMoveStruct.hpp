#ifndef FM_KWAY_MOVE_STRUCT_HPP
#define FM_KWAY_MOVE_STRUCT_HPP

class Cell;
class FMMoveInfo;

/** @addtogroup group5
 *  @{
 */

/** FM Multi-Way Move Structure. An adapter class. */
template <class A, class B>
class FMKWayMoveStruct
{
public:
  /** Constructor */
  FMKWayMoveStruct( A& a, B& b ) : _a(a), _b(b) {}


  /** Update the change of the move structure when the cell v moves
      from fromPart to toPart */
  void updateMove( const FMMoveInfo& moveInfo, const short* d ) 
  {
    _a.updateMove( moveInfo, d );
    _b.updateMove( moveInfo, d );
  }


  /** Update the change of the move structure, given each neighbour
      cell w and its corresponding gain change. Probably only use for
      the gain related structures such as gain array and gain buckets */
  void updateNeighbor( unsigned int whichPart, unsigned int i_w, 
                       const short* deltaGain ) 
  {
    _a.updateNeighbor( whichPart, i_w, deltaGain );
    _b.updateNeighbor( whichPart, i_w, deltaGain );
  }

private:
  A&  _a;
  B&  _b;
};

/** @} */

#endif
