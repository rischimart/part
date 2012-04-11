#ifndef FM_BI_MOVE_STRUCT_HPP
#define FM_BI_MOVE_STRUCT_HPP

class Cell;
class FMMoveInfo;

/** FM Bi-partitioning Move structure. An adaptor class */
template <class A, class B>
class FMBiMoveStruct
{
public:
  /** Constructor */
  FMBiMoveStruct( A& a, B& b ) : _a(a), _b(b) {}


  /** Update the change of the move structure when the cell v moves
      from fromPart to toPart */
  void updateMove( const FMMoveInfo& moveInfo ) 
  {
    _a.updateMove( moveInfo );
    _b.updateMove( moveInfo );
  }


  /** Update the change of the move structure, given each neighbour
      cell w and its corresponding gain change. Probably only use for
      the gain related structures such as gain array and gain buckets */
  void updateNeighbor( unsigned int whichPart, 
                       unsigned int i_w, short deltaGain ) 
  {
    _a.updateNeighbor( whichPart, i_w, deltaGain );
    _b.updateNeighbor( whichPart, i_w, deltaGain );
  }

private:
  A&  _a;
  B&  _b;
};

#endif
