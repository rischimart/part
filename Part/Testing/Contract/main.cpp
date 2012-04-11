#include <Netlist.hpp>
#include <Cell.hpp>
#include <map>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>

using namespace std;

#define CPPUNIT_ASSERT(_e)   if (!(_e)) { print_error(__FILE__, __LINE__); \
	cerr << #_e << endl; }

inline void print_error(const char* file, int lineno)
{
  cerr << "Test fail in " << file << ":" << lineno << endl;
}

int main(int, const char*)
{
  Netlist* _H;
  _H = new Netlist;
  _H->readNetD("Nets/newPartProb.netD");

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

  //xxx const unsigned int n = _H->getNumCells();
  //xxx const unsigned int nParts = 2;
  //xxx vector<unsigned char> part(n);
  //xxx for (unsigned int i=0; i<n; ++i) {
  //xxx   part[i] = rand() % nParts;  // randomly assigned to 4 partitions
  //xxx }
  //xxx 
  //xxx _H->setFixedNets(part);
  //xxx Netlist::SPN H2 = _H->contractByMMC(part);  
  //xxx vector<unsigned char> partUp(H2->getNumCells());
  //xxx H2->projectUp(*_H, partUp, part);
  //xxx 
  //xxx _H->contractPhase2(H2);  
  //xxx CPPUNIT_ASSERT (H2->checkOk());
  //xxx 
  //xxx vector<unsigned char> part2(n);
  //xxx _H->projectDown(*H2, partUp, part2);
  //xxx 
  //xxx CPPUNIT_ASSERT (part == part2);
}
