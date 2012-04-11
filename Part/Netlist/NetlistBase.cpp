#include "NetlistBase.hpp"
#include <iostream>
#include <fstream>   //modified by Zhou
#include <vector>
#include <cassert>  //added by Zhou
#include <climits>  //added by Zhou

using std::ifstream; //added by Zhou
using std::ofstream; //added by Zhou
using std::vector;

// #include <boost/pool/singleton_pool.hpp>
// struct MyPoolTag { };
// typedef boost::singleton_pool<MyPoolTag, sizeof(Pin)> my_pool;

/** Constructor */
NetlistBase::NetlistBase() :
  _numCells(0),
  _numNets(0),
  _numPins(0),
  _padOffset(0),
  _maxDegree(0),
  _totalWeight(0),
  _hasFixedCells(-1),
  _costModel(0)
{
}


/** Destructor */
NetlistBase::~NetlistBase()
{
  clear();
}


/** Reset to an empty NetlistBase */
void NetlistBase::clear()
{
  vector<Cell>().swap(_cellList);  // see Effective C++, item 7
  vector<Net>().swap(_netList);
  vector<Pin>().swap(_pinList);

  _numCells = 0;
  _numNets = 0;
  _numPins = 0;
  _maxDegree = 0;
  _totalWeight = 0;
  _hasFixedCells = -1;
  _costModel = 0;
}


/** @return true if the netlist contains fixed cells.
    Take O(n) time the first time it is called. */
bool NetlistBase::hasFixedCells()
{
  if (_hasFixedCells != -1) return _hasFixedCells;
  for (unsigned int i=0; i<getNumCells(); ++i) {
    if (getCell(i).isFixed()) {
      _hasFixedCells = 1;
      return true;
    }
  }
  _hasFixedCells = 0;
  return false;
}


// Write the partition solution
bool NetlistBase::writeSol(const char* solFileName, int numParts,
			   const vector<unsigned char>& part) const
{
  ofstream out(solFileName);
  if (out.fail()) return false;
  out << getNumCells() << ' ' << numParts << '\n';
  for (unsigned int i=0; i<getNumCells(); i++) {
    const Cell& c = _cellList[i];
    if (c.isPad()) out << 'p' << i - _padOffset;
    else out << 'a' << i;
    out << ' ' << (int) part[i] << '\n';
  }

  return true;
}


//xxx // read the partition solution in hmetris format
//xxx bool NetlistBase::readHMetisSol(const char* solFileName,
//xxx 				vector<unsigned char>& part) const
//xxx {
//xxx   ifstream sol(solFileName);
//xxx   if (sol.fail()) {
//xxx     std::cerr << " Could not open " << solFileName << std::endl;
//xxx     return 0;
//xxx   }
//xxx
//xxx   const unsigned int bufferSize = 100;
//xxx   char lineBuffer[bufferSize];
//xxx   char c;
//xxx   unsigned int w;
//xxx   unsigned int lineno = 1;
//xxx   unsigned int part_no;
//xxx
//xxx   // Skip the first 9 lines
//xxx   sol.getline(lineBuffer, bufferSize);
//xxx   sol.getline(lineBuffer, bufferSize);
//xxx   sol.getline(lineBuffer, bufferSize);
//xxx   sol.getline(lineBuffer, bufferSize);
//xxx   sol.getline(lineBuffer, bufferSize);
//xxx   sol.getline(lineBuffer, bufferSize);
//xxx   sol.getline(lineBuffer, bufferSize);
//xxx   sol.getline(lineBuffer, bufferSize);
//xxx   sol.getline(lineBuffer, bufferSize);
//xxx
//xxx   for (unsigned int i=0; i<_numCells; i++) {
//xxx     if (sol.eof()) {
//xxx       std::cerr << "Warning: Unexpected end of file.\n";
//xxx       break;
//xxx     }
//xxx     do sol.get(c); while (isspace(c) && c != EOF);
//xxx     if (c == '\n') {
//xxx       lineno++;
//xxx       continue;
//xxx     }
//xxx     if (c == 'a') {
//xxx       sol >> w;
//xxx     }
//xxx     else if (c == 'p') {
//xxx       sol >> w;
//xxx       w += _padOffset;
//xxx     }
//xxx     else {
//xxx       std::cerr << "Syntax error in line " << lineno << ":"
//xxx                 << "expect keyword \"a\" or \"p\"" << std::endl;
//xxx       exit(0);
//xxx     }
//xxx
//xxx     do sol.get(c); while (!isdigit(c) && c != EOF);
//xxx     if (isdigit(c)) {
//xxx       sol.putback(c);
//xxx       sol >> part_no;
//xxx       part[w] = part_no;
//xxx     }
//xxx     sol.getline(lineBuffer, bufferSize);
//xxx     lineno++;
//xxx   }
//xxx
//xxx   std::cout << "Initial cost = " << cutCost(part, 2) << ", ";
//xxx   return true;
//xxx }


// read the partition solution in hmetris format
bool NetlistBase::readHMetisSol(const char* solFileName,
                            vector<unsigned char>& part) const
{
  ifstream in(solFileName);
  if (in.fail()) return false;

  const unsigned int bufferSize = 100;
  char lineBuffer[bufferSize];
  char c;
  unsigned int part_no;
  unsigned int max_part_no = 0;
  unsigned int lineno = 1;
  for (unsigned int i=0; i<_numCells; i++) {
    if (in.eof()) break;
    do in.get(c); while (isspace(c) && c != EOF);
    if (isdigit(c)) {
      in.putback(c);
      in >> part_no;
      part[i] = part_no;
      if (part_no > max_part_no) max_part_no = part_no;
    }
    in.getline(lineBuffer, bufferSize);
    lineno++;
  }

  //xxx std::cout << "Initial cost = " << cutCost(part, max_part_no+1) << ", ";
  return true;
}


// Read the IBM .netD/.net format. Precondition: Netlist is empty.
bool NetlistBase::readNetD(const char* netDFileName)
{
  ifstream netD(netDFileName);
  if (netD.fail()) return false;

  char t;
  netD >> t; // eat 1st 0
  netD >> _numPins >> _numNets >> _numCells >> _padOffset;
  if (_padOffset == 0) _padOffset = _numCells - 1;
  //xxx numPads = _numCells - _padOffset - 1;

  _totalWeight = _numCells; // unweighted
  _cellList.resize(_numCells);
  _netList.resize(_numNets + 1); //### plus one dummy net at the end
  _pinList.resize(_numPins);

  // Allocate memory for this function only
  vector<unsigned int> degList(_numCells, 0);

  unsigned int i;
  for (i=_padOffset+1; i<_numCells; i++) _cellList[i]._weight = 0;

  const unsigned int bufferSize = 100;
  char lineBuffer[bufferSize];  // Does it work for other compiler?
  netD.getline(lineBuffer, bufferSize);

  unsigned int w, e=0;
  Cell* aCell = 0;
  Net* aNet = 0;
  char c;

  for (i=0; i<_numPins; i++) {
    Pin& aPin = _pinList[i];
    if (netD.eof()) {
      std::cerr << "Warning: Unexpected end of file.\n";
      break;
    }
    do netD.get(c); while (isspace(c) && c != EOF);
    if (c == '\n') continue;
    if (c == 'a') {
      netD >> w;
      aCell = &_cellList[w];
      degList[w]++;
    }
    else if (c == 'p') {
      netD >> w;
      w += _padOffset;
      aCell = &_cellList[w];
      // aCell->_weight = 0;  // area of pad is 0
      degList[w]++;
    }
    do netD.get(c); while (isspace(c) && c != EOF);
    if (c == 's') {
      aNet = &_netList[e];
      aNet->_firstAdjPin = &aPin;  //###
      e++;
    }

    // Should check if same net connected to different pins of same cell
    connect(aPin, *aCell, *aNet);
    do netD.get(c); while (isspace(c) && c != '\n' && c != EOF);
    // switch (c) {
    // case 'O': aPin.setDirection(Pin::OUTPUT); break;
    // case 'I': aPin.setDirection(Pin::INPUT); break;
    // case 'B': aPin.setDirection(Pin::BIDIR); break;
    // }

    if (c != '\n') netD.getline(lineBuffer, bufferSize);
  }

  if (e < _numNets) {
    std::cerr << "Warning: number of pins is not " << _numNets << ".\n";
    _numNets = e;
    _netList.resize(_numNets + 1); //###
  }
  else if (e > _numNets) {
    std::cerr << "Error: number of pins is not " << _numNets << ".\n";
    exit(0);
  }
  if (i < _numPins) {
    std::cerr << "Warning: number of pins is not " << _numPins << ".\n";
    _numPins = i;
    _pinList.resize(_numPins);
  }

  //### add one dummy net
  _netList[_numNets]._firstAdjPin = &_pinList[_numPins];

  // Calculate the maximnum degree
  _maxDegree = 0;
  for (i=0; i<_numCells; i++) {
    if (_maxDegree < degList[i]) _maxDegree = degList[i];
  }

  assert(checkOk());
  return true;
}



// Read the hMetis format. Precondition: Netlist is empty.
bool NetlistBase::readHMetis(const char* hgrFileName)
{
  ifstream hgr(hgrFileName);
  if (hgr.fail()) return false;

  int hgrType;
  hgr >> _numNets >> _numCells >> hgrType;
  _padOffset = _numCells - 1;

  _cellList.resize(_numCells);
  _netList.resize(_numNets + 1); //### plus one dummy net at the end

  const unsigned int bufferSize = 100;
  char lineBuffer[bufferSize];  // Does it work for other compiler?
  hgr.getline(lineBuffer, bufferSize);
  unsigned int lineno = 1;

  char c;

  unsigned int pin_reserve = 300000;
  _pinList.reserve(pin_reserve);

  _numPins = 0;
  unsigned int i, w;
  for (i=0; i<_numNets; i++) {
    Net& aNet = _netList[i];
    aNet._firstAdjPin = &_pinList[_numPins];
    if (hgr.eof()) {
      std::cerr << "Warning: Unexpected end of file.\n";
      break;
    }
    do {
      do hgr.get(c); while (!isdigit(c) && c != '\n' && c != EOF);
      if (isdigit(c)) {
        hgr.putback(c);
        hgr >> w;
        if (w > _numCells) {
          std::cerr << "Warning: number of cells is not ";
          std::cerr << _numCells << ". (line " << lineno << ")\n";
          _numCells = w;
          _cellList.resize(_numCells); //###
        }
        w -= 1; // index from 0 internally
        Cell& aCell = _cellList[w];

        ++_numPins;
        //xxx if (_numPins == pin_reserve) {
        //xxx   pin_reserve *= 2;
        //xxx   _pinList.reserve(pin_reserve);
        //xxx }
        _pinList.resize(_numPins);
        Pin& aPin = _pinList[_numPins-1];
        connect(aPin, aCell, aNet);
      }
    } while (c != '\n' && c != EOF);
    lineno++;
  }

  if (i < _numNets) {
    std::cerr << "Warning: number of nets is not " << _numNets << ".\n";
    _numNets = i;
    _netList.resize(_numNets + 1); //###
  }

  //### add one dummy net
  _netList[_numNets]._firstAdjPin = &_pinList[_numPins];

  _totalWeight = _numCells;

  if (hgrType >= 10) {
    _totalWeight = 0;
    for (i=0; i<_numCells; i++) {
      Cell& aCell = _cellList[i];
      if (hgr.eof()) {
        std::cerr << "Warning: Unexpected end of file.\n";
        break;
      }
      do hgr.get(c); while (!isdigit(c) && c != '\n' && c != EOF);
      if (isdigit(c)) {
        hgr.putback(c);
        hgr >> w;
        aCell._weight = w;
        _totalWeight += w;
      }
      hgr.getline(lineBuffer, bufferSize);
      lineno++;
    }
  }

  if (hgrType == 11) {
    for (i=0; i<_numNets; i++) {
      Net& aNet = _netList[i];
      if (hgr.eof()) {
        std::cerr << "Warning: Unexpected end of file.\n";
        break;
      }
      do hgr.get(c); while (!isdigit(c) && c != '\n' && c != EOF);
      if (isdigit(c)) {
        hgr.putback(c);
        hgr >> w;
        aNet._weight = w;
      }
      hgr.getline(lineBuffer, bufferSize);
      lineno++;
    }
  }

  // Calculate the maximnum degree
  _maxDegree = 0;
  for (i=0; i<_numCells; i++) {
    const Cell& c = _cellList[i];
    NetConstCursor ncc(c);
    unsigned int degL = 0;
    while (ncc.hasNext()) {
      const Net& e = ncc.getNext();
      degL += e.getWeight();
    }
    if (_maxDegree < degL) _maxDegree = degL;
  }

  assert(checkOk());
  return true;
}



// Read the IBM .are format
void NetlistBase::readAre(const char* areFileName)
{
  ifstream are(areFileName);
  if (are.fail()) {
    std::cerr << " Could not open " << areFileName << std::endl;
    return;
  }

  const unsigned int bufferSize = 100;
  char lineBuffer[bufferSize];

  char c;
  unsigned int w;
  unsigned int weight;
  unsigned int totalWeight = 0;
  //xxx unsigned int smallestWeight = UINT_MAX;

  unsigned int lineno = 1;
  for (unsigned int i=0; i<_numCells; i++) {
    if (are.eof()) break;
    do are.get(c); while (isspace(c) && c != EOF);
    if (c == '\n') {
      lineno++;
      continue;
    }
    if (c == 'a') {
      are >> w;
    }
    else if (c == 'p') {
      are >> w;
      w += _padOffset;
    }
    else {
      std::cerr << "Syntax error in line " << lineno << ":"
                << "expect keyword \"a\" or \"p\"" << std::endl;
      exit(0);
    }

    Cell& aCell = _cellList[w];
    do are.get(c); while (isspace(c) && c != EOF);
    if (isdigit(c)) {
      are.putback(c);
      are >> weight;
      aCell.setWeight(weight);
      totalWeight += weight;
    }
    are.getline(lineBuffer, bufferSize);
    lineno++;
  }

  // Update after the reading was sucessful
  _totalWeight = totalWeight;
}


int NetlistBase::writeHMetis(const char* fileName) const
{
  const Cell* const c0 = &getCell(0);
  ofstream out(fileName);

  if (out.fail()) {
    std::cerr << " Could not open " << fileName;
    return 0;
  }

  unsigned int n = _numCells;
  unsigned int m = _numNets;
  out << m << " " << n;            //  m: number of nets
			           //  n: number of cells
  out << " 11\n";                  //  weighted vertex and weighted net

  // For each net, write out the index of it connected cells
  unsigned int i;
  for (i=0; i<m; i++) {
    const Net& e = _netList[i];
    CellConstCursor cc(e);
    while (cc.hasNext()) {
      const Cell& c = cc.getNext();
      out << (&c - c0) + 1 << " ";
    }
    out << "\n";
  }

  // Write out the cell weights
  for (i=0; i<n; i++) out << _cellList[i].getWeight() << "\n";

  // Write out the net weights
  for (i=0; i<m; i++) out << _netList[i].getWeight() << "\n";

  return 1;
}


int NetlistBase::writeDot(const char* fileName) const
{
  const Net* const n0 = &getNet(0);
  ofstream out(fileName);

  if (out.fail()) {
    std::cerr << " Could not open " << fileName << std::endl;
    return 0;
  }

  unsigned int n = _numCells;
  unsigned int m = _numNets;
  unsigned int i;

  out << "digraph H {\n";
  out << "\trankdir=LR;\n";
  out << "\t{ node [shape=circle];\n";
  out << "\t";

  for (i=0; i<m; i++) {
    out << " n" << i << ";" ;
  }

  out << " }\n";

  for (i=0; i<n; i++) {
    const Cell& c = _cellList[i];

    PinForCellConstCursor pcc(c);
    while (pcc.hasNext()) {
      const Pin& p = pcc.getNext();
      const Net& e = p.getNet();
      out << "\t";
      // if (p.getDirection() == Pin::OUTPUT) {
      //   if (c.isPad()) {
      //     out << "p" << i - _padOffset;
      // 	}
      //   else {
      //     out << "a" << i;
      //   }
      //   out << " -> ";
      //   out << "n" << getId(e);
      // }
      // else {
      out << "n" << &e - n0;
      out << " -> ";
      if (c.isPad()) {
	out << "p" << i - _padOffset;
      }
      else {
	out << "a" << i;
      }
      // }
      out << ";\n";
    }
  }

  out << "}\n";
  return 1;
}


bool NetlistBase::checkOk() const
{
  unsigned int i=0;
  for (; i<getNumCells(); ++i) {
    const Cell& cell = getCell(i);
    PinForCellConstCursor pccc(cell);
    while (pccc.hasNext()) {
      const Pin& pin = pccc.getNext();
      if (&pin.getCell() != &cell) return false;
    }
  }
  for (i=0; i<getNumNets(); ++i) {
    const Net& net = getNet(i);
    if (&net.firstAdjPin()->getNet() != &net) return false;
    PinForNetConstCursor pccc(net);
    while (pccc.hasNext()) {
      const Pin& pin = pccc.getNext();
      if (&pin.getNet() != &net) return false;
    }
  }

  return true;
}


// Print the net distribution (for debug purpose)
void NetlistBase::printNetDist() const
{
  vector<int> ndcount(15+1); // print nets from 2 pins to 15 pins
  unsigned int i=0;

  for (; i<getNumNets(); ++i) {
    const Net& net = getNet(i);
    PinForNetConstCursor pccc(net);
    int count = pccc.count();
    if (count > 15) count = 15; // 15 pins or above
    ++ndcount[count];
  }

  std::cout << "ND: ";

  for (i=2; i<=15; ++i) {
    std::cout << ndcount[i] << " ";
  }

  std::cout << '\n';
}



/** @return the cut cost of partition */
unsigned int NetlistBase::cutCost(const vector<unsigned char>& part,
                                  unsigned int numOfPartitions) const
{
  switch (getCostModel()) {
  case 0: return HyperEdgeCost(part);
  case 1: return KMinus1Cost(part, numOfPartitions);
  case 2: return SOEDCost(part, numOfPartitions);
  case 3: return TSVCost(part); //added by Zhou
  }
  return 0;  /* make compiler happy */
}


unsigned int NetlistBase::
HyperEdgeCost(const vector<unsigned char>& part) const
{
  const Cell* const c0 = &getCell(0);
  unsigned int result = 0;
  for (unsigned int i_e=0; i_e<getNumNets(); ++i_e) {
    const Net& e = getNet(i_e);
    CellConstCursor nc(e);
    bool sameSide = true;
    assert(nc.hasNext());  // a net is at least connected to one cell
    const Cell& w = nc.getNext();
    const unsigned int part_w = part[&w - c0];
    while (nc.hasNext()) {
      const Cell& v = nc.getNext();
      if (part[&v - c0] != part_w) {
        sameSide = false;
        break;
      }
    }
    if (!sameSide) result += e.getWeight();
  }
  return result;
}


/** (K-1) Metric Functor Class */
class KMinus1Metric
{
public:
  /** @return the cut cost of a net that spans in K partitions. */
  unsigned int operator()(unsigned int K) const { return K - 1; }

  /** @return the cut cost of a net that spans in 2 partitions. */
  unsigned int operator()() const { return 1; }
};

/** Sum-Of-External-Degrees Metric Functor Class */
class SOEDMetric
{
public:
  /** @return the cut cost of a net that spans in K partitions. */
  unsigned int operator()(unsigned int K) const { return (K > 1)? K : 0; }

  /** @return the cut cost of a net that spans in 2 partitions. */
  unsigned int operator()() const { return 2; }
};


/** @return the cut cost of partition according to the metric. */
template <class MetricFunction>
inline unsigned int NetlistBase::
cutCost(const vector<unsigned char>& part,
        MetricFunction metric, unsigned int numOfPartitions) const
{
  static int numA[256];
  const Cell* const c0 = &getCell(0); // offset of array
  unsigned int result = 0;
  for (unsigned int i_e=0; i_e<getNumNets(); ++i_e) {
    const Net& e = getNet(i_e);
    CellConstCursor nc(e);
    if (nc.count() == 2) { // special handle for two-pin nets
      const Cell& w1 = nc.getNext();
      const Cell& w2 = nc.getNext();
      if (part[&w1 - c0] != part[&w2 - c0]) {
        result += metric() * e.getWeight();
      }
      continue;
    }
    //xxx vector<int> numA(numOfPartitions);
    std::fill_n(numA, numOfPartitions, 0);
    while (nc.hasNext()) {
      const Cell& w = nc.getNext();
      ++numA[part[&w - c0]];
    }
    int numNonEmpty = 0;
    for ( unsigned int k=0; k<numOfPartitions; ++k ) {
      if ( numA[k] != 0 ) ++numNonEmpty;
    }

    result += metric(numNonEmpty) * e.getWeight();
  }

  return result;
}



/** @return the cut cost of partition in K-1 metric. */
unsigned int NetlistBase::
KMinus1Cost(const vector<unsigned char>& part,
            unsigned int numOfPartitions) const
{
  KMinus1Metric kf;
  return cutCost(part, kf, numOfPartitions);
}


/** @return the cut cost of partitions in Sum-Of-External-Degrees
    (SOED) metric. Note that the SOED cost is equal to the K-1 cost
    plus the HyperEdge cost. */
unsigned int NetlistBase::
SOEDCost(const vector<unsigned char>& part,
         unsigned int numOfPartitions) const
{
  SOEDMetric sf;
  return cutCost(part, sf, numOfPartitions);
}

/** @return the cut cost of the partitioning according to the
    TSV metric, which is used to evaluate the quality of 3D IC
    partitioning. */  //added by Zhou
unsigned int NetlistBase::
TSVCost(const vector<unsigned char>& part) const
{
  const Cell* const c0 = &getCell(0);
  unsigned int result = 0;
  for (unsigned int i_e=0; i_e<getNumNets(); ++i_e) {
    const Net& e = getNet(i_e);
    CellConstCursor nc(e);
    assert(nc.hasNext());
    const Cell& w = nc.getNext();
    unsigned int maxPart = part[&w - c0];  // the highest of the layers that the net spans
    unsigned int minPart = part[&w - c0];   // the lowest of the layers that the net spans
    while (nc.hasNext()) {
      const Cell& v = nc.getNext();
      if (part[&v - c0] > maxPart)
        maxPart = part[&v - c0];
      else if(part[&v - c0] < minPart)
        minPart = part[&v - c0];
    }
    result += (maxPart - minPart);
  }
  return result;
}


