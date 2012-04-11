#ifndef PIN_HPP
#define PIN_HPP


/** Forward declaration */
class Netlist;
class NetlistBase;
class Cell;
class Net;

/** @addtogroup data_model
 *  @{
 */

/** 
 * A Pin class. It may not be necessary to create pin in FM algorithm.
 * However, we use this class for CycleRemoval algorithm.
 */
class Pin
{ 
  friend class Netlist;
  friend class NetlistBase;
  friend class Cell;
  friend class Net;

public:

  /** 
   * Pin direction enum. 
   */
  enum DIRECTION {
    UNKNOWN,   /**< unknown direction */
    INPUT,     /**< input pin */
    OUTPUT,    /**< output pin */
    BIDIR      /**< bi-direction pin */
  };

private:
  //  DIRECTION  _direction : 8;        /**< pin direction */
  Pin*       _nextAdjPinForCell;    /**< next adjacent pin for cell */
  Cell*      _theCell;              /**< cell connect to */
  Net*       _theNet;               /**< net connect to */

public:
  /** A constructor. */
  Pin() : 
    // _direction(UNKNOWN),
    _nextAdjPinForCell(0),
    _theCell(0), 
    _theNet(0) 
  {
  }     

  /** @return the direction (not support yet) */
  DIRECTION getDirection() const { return UNKNOWN; }
  
  /** Set the direction. (not support yet) */
  void setDirection(DIRECTION ) { }

  /** @return the Cell connected to this pin */
  Cell& getCell() const { return *_theCell; }

  /** @return the net connected to this pin */
  Net& getNet() const { return *_theNet; }

  /** @return the next adjacency pin (for cell) */
  Pin* nextAdjPinForCell() const { return _nextAdjPinForCell; }
};

/** @} */ // end of group1

#endif
