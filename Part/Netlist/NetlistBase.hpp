#ifndef NET_LIST_BASE_HPP
#define NET_LIST_BASE_HPP

#ifndef PIN_HPP
#include "Pin.hpp"
#endif
#ifndef NET_HPP
#include "Net.hpp"
#endif
#ifndef CELL_HPP
#include "Cell.hpp"
#endif
#include <boost/utility.hpp>
#include <cassert>
#include <vector>

/** @addtogroup data_model
 *  @{
 */

/** A netlist base class. This class owns its cells, nets and pins.*/
class NetlistBase : private boost::noncopyable
{
public:
  /** Constructor */
  NetlistBase();

  /** Destructor */
  ~NetlistBase();

  /** Reset an empty netlist */
  void clear();

  /** @return the number of cells */
  unsigned int getNumCells() const { return _numCells; }

  /** @return the number of nets */
  unsigned int getNumNets() const { return _numNets; }

  /** @return the number of pads. Note: only for netlists contructed
      by IBM .NET format. */
  unsigned int getNumPads() const { return _numCells - _padOffset - 1; }

  /** @return the number of pins */
  unsigned int getNumPins()  const { return _numPins; }

  /** @return the max number of pins on one cell */
  unsigned int getMaxDegree() const { return _maxDegree; }

  /** @return cell index. */
  unsigned int getId(const Cell& c) const { return &c - &getCell(0); }

  /** @return net index. */
  unsigned int getId(const Net& e) const { return &e - &getNet(0); }

  /** @return cell by index. Precondition: index within array bound. */
  Cell& getCell(unsigned int id) { return _cellList[id]; }

  /** @return net by index. Precondition: index with array bound. */
  Net& getNet(unsigned int id) { return _netList[id]; }

  /** @return pin by index. Precondition: index with array bound. */
  Pin& getPin(unsigned int id) { return _pinList[id]; }

  /** @return cell by index. Precondition: index with array bound. */
  const Cell& getCell(unsigned int id) const { return _cellList[id]; }

  /** @return the net by index. Precondition: index with array bound. */
  const Net& getNet(unsigned int id) const { return _netList[id]; }

  /** @return the pin by index. Precondition: index with array bound. */
  const Pin& getPin(unsigned int id) const { return _pinList[id]; }

  /** @return total weight of cells. */
  unsigned int getTotalWeight() const { return _totalWeight; }

  /** Clear visited attribute of all nets. Take O(n) time. */
  void clrVisitedAllNets() const
  {
    for (unsigned int i=0; i<getNumNets(); ++i) getNet(i).clrVisited();
  }

  /** Clear visited attribute of all nets. Take O(m) time. */
  void clrFixedAllNets()
  {
    for (unsigned int i=0; i<getNumNets(); ++i) getNet(i).clrFixed();
  }

  /** Clear visited attribute of all cells. Take O(n) time. */
  void clrVisitedAllCells() const
  {
    for (unsigned int i=0; i<getNumCells(); ++i) getCell(i).clrVisited();
  }

  /** Read the HMetis format. */
  bool readHMetis(const char* hgrFileName);

  /** Write the HMetis format. */
  int writeHMetis(const char* fileName) const;

  /** Write the Dot format. */
  int writeDot(const char* fileName) const;

  /** Read the .netD format */
  bool readNetD(const char* fileName);

  /** Read the .netD format */
  bool readQtNetD(const char* fileName);

  /** Read the .are format */
  void readAre(const char* fileName);

  /** Write the partition solution. */
  bool writeSol(const char* fileName, int K,
                const std::vector<unsigned char>& part) const;

  /** Read the partition solution in hmetis format. */
  bool readHMetisSol(const char* fileName,
                     std::vector<unsigned char>& part) const;

  /** @return true if the netlist contains fixed cells.
      Take O(n) time the first time it is called. */
  bool hasFixedCells();

  /** Check if the netlist was created properly. Take O(n+m) time. */
  bool checkOk() const;

  /** Print the net distribution information */
  void printNetDist() const;

  /** @return the cost model. */
  int getCostModel() const { return _costModel; }

  /** Set the cost model. */
  void setCostModel(int costModel) { _costModel = costModel; }

  /** @return the cut cost according to the defined cost model. Take
      O(m) time. */
  unsigned int cutCost(const std::vector<unsigned char>& part,
                       unsigned int numOfPartitions) const;

  /** @return the cut cost in the (K-1) metric. The partitioning
      routines in this package try to minimize this quantity. Take
      O(m) time. */
  unsigned int KMinus1Cost(const std::vector<unsigned char>& part,
                           unsigned int numOfPartitions) const;

  /** @return the cut cost in Sum-Of-External-Degrees (SOED)
      metric. The external degree of a partition is defined as the
      number of hyperedges, that are incident but not fully inside the
      partition. Take O(m) time. */
  unsigned int SOEDCost(const std::vector<unsigned char>& part,
                        unsigned int numOfPartitions) const;

  /** @return the cut cost in Hyperedge metric. This is the number of
      the hyperedges that span multiple partition. Take O(m) time. */
  unsigned int HyperEdgeCost(const std::vector<unsigned char>& part) const;

   /** @return the cut cost according to TSV metric. The partitioning
      routines in this package try to minimize this quantity. Take
      O(m) time. */
  unsigned int TSVCost(const std::vector<unsigned char>& part) const;


protected:
  /** Connect cell v to net e by mean of p */
  static void connect(Pin& p, Cell& v, Net& e)
  {
    p._theCell = &v;
    p._theNet = &e;
    p._nextAdjPinForCell = v._firstAdjPin;
    v._firstAdjPin = &p;
    //### p._nextAdjPinForNet = e._firstAdjPin;
    //### e._firstAdjPin = &p;
  }

private:
  /** @return the cut cost of partition according to the metric. */
  template <class MetricFunction>
  inline unsigned int cutCost(const std::vector<unsigned char>& part,
                              MetricFunction metric,
                              unsigned int numOfPartitions) const;

protected:
  unsigned int    _numCells;    /**< number of cells */
  unsigned int    _numNets;     /**< number of nets */
  unsigned int    _numPins;     /**< number of pins */
  unsigned int    _padOffset;   /**< pad is one kind of cells */
  unsigned int    _maxDegree;   /**< maximum pins of cells */
  std::vector<Cell> _cellList;  /**< array of all cells */
  std::vector<Net>  _netList;   /**< array of all nets */
  std::vector<Pin>  _pinList;   /**< array of all pins */
  unsigned int  _totalWeight;   /**< total weight of cells */
  int _hasFixedCells;
  int _costModel;  /**< Cost Model: 0: HyperEdge, 1: K-1 metric, 2: SOED */

  /** Note: In order to save memory, the _vertexArray is not reserved
      memory in the constructor. Call initVertexArray() before it is
      used. */
};


/** Cursor to traverse all the pins of one net/cell */
template <class Net_, class Pin_ = Pin>
class PinForNetCursorTmpl
{
public:
  /** Constructor */
  PinForNetCursorTmpl(Net_& n) :
    _nextPin(n.firstAdjPin()),
    _lastPin((&n+1)->firstAdjPin())
  {
  }

  /** @return the number of pins remain to traverse. */
  unsigned int count() const { return _lastPin - _nextPin; }

  /** @return true if there exist a next item. */
  bool hasNext() const { return _nextPin != _lastPin; }

  /** @return the next pin. Precondition: there exist the next item. */
  Pin_& getNext()
  {
    assert(hasNext());
    return *_nextPin++;
  }

private:
  Pin_* _nextPin;         /**< next adjacency pin */
  Pin_* const _lastPin;   /**< marker of end pin */
};


/** Cursor to traverse all the cells of one net. Since there is no
    direct pointer to the cells (need to go through pin). It may make the
    FM algorithm a little slower than the highly tuned
    implementation. */
template <class Cell_, class Net_, class Pin_ = Pin>
class CellCursorTmpl : public PinForNetCursorTmpl<Net_, Pin_>
{
public:
  /** Constructor */
  CellCursorTmpl(Net_& n) : PinForNetCursorTmpl<Net_, Pin_>(n) {}

  /** @return the next cell. This function is crucial in FM algorithm.
      Precondition: there exist the next item. */
  Cell_& getNext()
  {
    return PinForNetCursorTmpl<Net_, Pin_>::getNext().getCell();
  }
};



/** Cursor to traverse all the pins of one net/cell */
template <class Cell_, class Pin_ = Pin>
class PinForCellCursorTmpl
{
public:
  /** Constructor */
  PinForCellCursorTmpl(Cell_& n) :
    _firstAdjPin(n.firstAdjPin()),
    _nextPin(_firstAdjPin)
  {
  }

  /** @return true if there exist a next item. */
  bool hasNext() const { return _nextPin != 0; }

  /** Reset cursor to the first position */
  void reset() { _nextPin = _firstAdjPin; }

  /** @return the next pin. Precondition: there exist the next item. */
  Pin_& getNext()
  {
    assert(hasNext());
    Pin_& p = *_nextPin;
    _nextPin = _nextPin->nextAdjPinForCell();
    return p;
  }

private:
  Pin_* const _firstAdjPin;  /**< first adjacency pin */
  Pin_* _nextPin;            /**< next adjacency pin */
};



/** Cursor to traverse all the cells of one net. Since there is no
    direct pointer to the cells (need to go through pin). It may make the
    FM algorithm a little slower than the highly tuned
    implementation. */
template <class Net_, class Cell_, class Pin_ = Pin>
class NetCursorTmpl : public PinForCellCursorTmpl<Cell_, Pin_>
{
public:
  /** Constructor */
  NetCursorTmpl(Cell_& n) : PinForCellCursorTmpl<Cell_, Pin_>(n) {}

  /** @return the next cell. This function is crucial in FM algorithm.
      Precondition: there exist the next item. */
  Net_& getNext()
  {
    return PinForCellCursorTmpl<Cell_, Pin_>::getNext().getNet();
  }
};


typedef PinForNetCursorTmpl<Net, Pin> PinForNetCursor;
typedef PinForNetCursorTmpl<const Net, const Pin> PinForNetConstCursor;
typedef CellCursorTmpl<Cell, Net, Pin> CellCursor;
typedef CellCursorTmpl<const Cell, const Net, const Pin> CellConstCursor;

typedef PinForCellCursorTmpl<Cell> PinForCellCursor;
typedef NetCursorTmpl<Net, Cell> NetCursor;
typedef PinForCellCursorTmpl<const Cell, const Pin> PinForCellConstCursor;
typedef NetCursorTmpl<const Net, const Cell, const Pin> NetConstCursor;

/** @} */ // end of group1

#endif
