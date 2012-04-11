#include "NetlistTest.hpp"
#include <Netlist.hpp>
#include <vector>

using std::vector;

CPPUNIT_TEST_SUITE_REGISTRATION( NetlistTestCase );

void NetlistTestCase::setUp ()
{
  _H = new Netlist;
  _H->readNetD("Nets/newPartProb.netD");
}


void NetlistTestCase::tearDown()
{
  delete _H;
}


void NetlistTestCase::testMNC()
{
  vector<unsigned int> weight(_H->getNumNets());
  _H->clusterSize(weight);
  _H->minimumNetCover(weight);
  CPPUNIT_ASSERT (_H->minimumNetCoverOK());    
}


void NetlistTestCase::testMIS()
{
  vector<unsigned int> weight(_H->getNumNets());
  _H->clusterSize(weight);
  _H->maximumIndependentSet(weight);
  CPPUNIT_ASSERT (_H->maximumIndependentSetOK());    
}


void NetlistTestCase::testContraction()
{
  _H->clrFixedAllNets();
  Netlist::SPN H2 = _H->contractByMMC();
  _H->contractPhase2(H2);
  CPPUNIT_ASSERT (H2->checkOk()); // MMC

  _H->clrFixedAllNets();
  Netlist::SPN H3 = _H->contractByFC();
  _H->contractPhase2(H3);
  CPPUNIT_ASSERT (H3->checkOk()); // FC

  _H->clrFixedAllNets();
  Netlist::SPN H4 = _H->contractByRandom();
  _H->contractPhase2(H4);
  CPPUNIT_ASSERT (H4->checkOk()); // Random
}


void NetlistTestCase::testProjection()
{
  const unsigned int n = _H->getNumCells();
  const unsigned int nParts = 2;
  vector<unsigned char> part(n);
  for (unsigned int i=0; i<n; ++i) {
    part[i] = rand() % nParts;  // randomly assigned to 4 partitions
  }

  _H->setFixedNets(part);
  Netlist::SPN H2 = _H->contractByMMC(part);
  
  vector<unsigned char> partUp(H2->getNumCells());
  H2->projectUp(*_H, partUp, part);

  _H->contractPhase2(H2);  
  vector<unsigned char> part2(n);
  _H->projectDown(*H2, partUp, part2);
  
  CPPUNIT_ASSERT (part == part2);
}
