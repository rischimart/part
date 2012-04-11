#ifndef FM_MOVE_INFO_HPP
#define FM_MOVE_INFO_HPP

class Cell;

/** Information about the moving cell */
class FMMoveInfo {
public:
  Cell*          _cell;       /**< the moving cell */
  unsigned int   _i_v;        /**< the index of the moving cell */
  unsigned int   _fromPart;   /**< from where the cell is moving */
  unsigned int   _toPart;     /**< to where the cell is moving */

public:
  /** Default constructor */
  FMMoveInfo() {}

  /** Constructor 
   *  @param   i_v        the index of the moving cell
   *  @param   cell       the moving cell
   *  @param   fromPart   from where the cell is moving
   *  @param   toPart     to where the cell is moving
   */
  FMMoveInfo(unsigned int i_v,
             Cell& cell,
             unsigned int fromPart,
             unsigned int toPart) :
    _cell(&cell),
    _i_v(i_v),
    _fromPart(fromPart),
    _toPart(toPart)
  {
  }

  //xxx /** Set the move information */
  //xxx void set(unsigned int i_v,
  //xxx          Cell& cell,
  //xxx          unsigned int fromPart,
  //xxx          unsigned int toPart)
  //xxx {
  //xxx   _i_v      = i_v;
  //xxx   _cell     = &cell;
  //xxx   _fromPart = fromPart;
  //xxx   _toPart   = toPart;
  //xxx }
};

#endif
