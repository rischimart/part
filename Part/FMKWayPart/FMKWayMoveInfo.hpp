#ifndef FM_KWAY_MOVE_INFO_HPP
#define FM_KWAY_MOVE_INFO_HPP

class Cell;
class GNodeKWay;

/** Move information */
class FMKWayMoveInfo {

public:
  GNodeKWay* _vertex;
  Cell*   _cell;     
  unsigned int _i_v;
  unsigned int _fromPart;
  unsigned int _toPart;

  /** Get the move information */
  void get(unsigned int& i_v,
           unsigned int& fromPart, 
           unsigned int& toPart) const
  {
    i_v = _i_v;
    fromPart = _fromPart;
    toPart = _toPart;
  }

  /** Set the move information */
  void set(unsigned int i_v,
           GNodeKWay* vertex,
           Cell* cell,
           unsigned int fromPart,
           unsigned int toPart)
  {
    _i_v      = i_v;
    _vertex   = vertex;
    _cell     = cell;
    _fromPart = fromPart;
    _toPart   = toPart;
  }

  /** Set the move information */
  void set(unsigned int i_v,
           GNodeKWay& vertex,
           Cell& cell,
           unsigned int fromPart,
           unsigned int toPart)
  {
    _i_v      = i_v;
    _vertex   = &vertex;
    _cell     = &cell;
    _fromPart = fromPart;
    _toPart   = toPart;
  }
};

#endif
