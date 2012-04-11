#include "NetlistBaseTest.hpp"
#include <NetlistBase.hpp>
#include <vector>

using std::vector;

CPPUNIT_TEST_SUITE_REGISTRATION( NetlistBaseTestCase );

void NetlistBaseTestCase::setUp ()
{
  _H = new NetlistBase;
  _H->readNetD("dwarf1.netD");
  _H->readAre("dwarf1.are");
}


void NetlistBaseTestCase::tearDown()
{
  delete _H;
}


void NetlistBaseTestCase::testCreate()
{
  CPPUNIT_ASSERT (_H->checkOk());
  CPPUNIT_ASSERT_EQUAL ((unsigned int) 7, _H->getNumCells());
  CPPUNIT_ASSERT_EQUAL ((unsigned int) 3, _H->getNumPads());
  CPPUNIT_ASSERT_EQUAL ((unsigned int) 5, _H->getNumNets());
  CPPUNIT_ASSERT_EQUAL ((unsigned int) 13, _H->getNumPins());
  CPPUNIT_ASSERT_EQUAL ((unsigned int) 3, _H->getMaxDegree());
  CPPUNIT_ASSERT_EQUAL ((unsigned int) 10, _H->getTotalWeight());
  CPPUNIT_ASSERT_EQUAL (false, _H->hasFixedCells());
}


void NetlistBaseTestCase::testCell()
{
  Cell& cell = _H->getCell(2);

  CPPUNIT_ASSERT_EQUAL ((unsigned int) 2, _H->getId(cell));
  CPPUNIT_ASSERT_EQUAL (Cell::CELL, cell.getType());
  CPPUNIT_ASSERT_EQUAL (false, cell.isCluster());
  CPPUNIT_ASSERT_EQUAL (false, cell.isPad());
  CPPUNIT_ASSERT_EQUAL (false, cell.isVisited());
  CPPUNIT_ASSERT_EQUAL (4, cell.getWeight());
}


void NetlistBaseTestCase::testNet()
{
  Net& net = _H->getNet(2);

  CPPUNIT_ASSERT_EQUAL ((unsigned int) 2, _H->getId(net));
  CPPUNIT_ASSERT_EQUAL (false, net.isVisited());
  CPPUNIT_ASSERT_EQUAL (1, net.getWeight());
}


void NetlistBaseTestCase::testPin()
{
  // Pin& pin = _H->getPin(2);

  // CPPUNIT_ASSERT_EQUAL (Pin::INPUT, pin.getDirection());
}


void NetlistBaseTestCase::testCellCursor()
{
  Net& net = _H->getNet(2);
  CellCursor cc(net);

  CPPUNIT_ASSERT (cc.hasNext());
  CPPUNIT_ASSERT_EQUAL ((unsigned int) 3, cc.count());  

  int count = 0;
  while (cc.hasNext()) {
    cc.getNext();
    ++count;
  }

  CPPUNIT_ASSERT_EQUAL (3, count);  
}


void NetlistBaseTestCase::testNetCursor()
{
  Cell& cell = _H->getCell(2);
  NetCursor nc(cell);

  CPPUNIT_ASSERT (nc.hasNext());

  int count = 0;
  while (nc.hasNext()) {
    nc.getNext();
    ++count;
  }

  CPPUNIT_ASSERT_EQUAL (3, count);  
}


void NetlistBaseTestCase::testPinCursor()
{
  Net& net = _H->getNet(2);
  PinForNetCursor pc(net);

  CPPUNIT_ASSERT (pc.hasNext());

  int count = 0;
  while (pc.hasNext()) {
    pc.getNext();
    ++count;
  }

  CPPUNIT_ASSERT_EQUAL (3, count);  
}


void NetlistBaseTestCase::testCutCost()
{
  const unsigned int n = _H->getNumCells();
  const unsigned int nParts = 3;
  vector<unsigned char> part(n);
  for (unsigned int i=0; i<n; ++i) {
    part[i] = rand() % nParts;  // randomly assigned to 4 partitions
  }

  CPPUNIT_ASSERT (_H->SOEDCost(part, nParts) == 
                  _H->KMinus1Cost(part, nParts) + _H->HyperEdgeCost(part));
}

