/**
 * This example shows the comparsion of the FIFO manager
 * against the LIFO manager under the multilevel
 * framework. Experiments showed that the difference between these two
 * versions are small.
 */
#include <FMParam.hpp>
#include <MlPartTmpl.hpp>
#include <FMException.hpp>
#include <FMPartTmpl.hpp>
#include <FMPartTmpl4.hpp>
#include <FMBiPartCore.hpp>
#include <FMBiPartCore4.hpp>
#include <FMBiGainMgr3.hpp>
#include <FMBiGainMgr2.hpp>
#include <Netlist.hpp>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <time.h>

using namespace std;

int main(int argc, const char* argv[])
{
  double balanceTol = 0.02;
  int numOfRuns = 20;

  if (argc >= 2) {
    balanceTol = atof(argv[1]);
  }
  if (argc >= 3) {
    numOfRuns = atoi(argv[2]);
  }

  const char* filename[] = { //xxx "null", "two", "dwarf1", "ibm01c"
	                     "ibm01", "ibm02", "ibm03",
			     "ibm04", "ibm05", "ibm06",
			     "ibm07", "ibm08", "ibm09",
			     "ibm10", "ibm11", "ibm12",
			     "ibm13", "ibm14", "ibm15",
			     "ibm16", "ibm17", "ibm18" };

  cout << "Compare ML FIFO with ML LIFO" << endl;
  cout << "Balance tol = " << balanceTol << endl;
  cout << "Number of starts = " << numOfRuns << endl;
  cout << "Circuit";
  cout << " & Cut(Min) & CPU(sec.) & Diff(%)";
  cout << " & Cut(Min) & CPU(sec.) & Diff(%)";
  cout << "  \\\\\n";
  cout << "\\hline %--------------------------------------------------------%";
  cout << endl;
  
  double elapsed;

  for (int i=0; i<18; i++) {
    Netlist H;
    string netDFile = "Nets/" + string(filename[i]) + ".netD";
    if (!H.readNetD(netDFile.c_str())) {
      cout << "Could not read .netD file " << netDFile << endl;
      continue;
    }    
    string areFile = "Nets/" + string(filename[i]) + ".are";
    ifstream in(areFile.c_str());
    if (in.fail()) {
      cout << "Could not open .are file " << areFile << endl;
      continue;
    }   
    in.close();
 
    H.readAre(areFile.c_str());
    cout << filename[i];
    std::vector<unsigned char> part(H.getNumCells());

    //xxx Cell& c = H.getCell(3);
    //xxx c.setFixed();
    //xxx part[3] = 1;

    typedef
    MlPartTmpl
    < FMBiPartMgrBase, 
      FMPartTmpl<FMBiPartCore, FMBiGainMgr3>, 
      FMPartTmpl4<FMBiPartCore4, FMBiGainMgr3> 
    > MlBiPartMgr;

    const FMParam param(H);
    
    try {
      srand(1);
      unsigned short int seed=1;
      seed48(&seed);
      MlBiPartMgr P2(param);  
      // P2.setContractMethod(&Netlist::contractByFC);
      P2.setVerbosity(1);
      P2.setBalanceTol(balanceTol);
      elapsed = P2.doPartition(part, numOfRuns);
      cout << " & " << std::fixed << std::setw(4) << P2.cutCost(part);
      cout << "(" << std::setprecision(2) << P2.getAvgCost() << ")";
      cout << " & " << std::fixed << std::setw(5) << std::setprecision(2)
      	 << elapsed/numOfRuns;
      	 //xxx << elapsed;
      cout << " & " << std::fixed << std::setw(4) << std::setprecision(2) 
           << P2.diff();
    } catch (FMException& e) {
      cout << "Error: " << e.what() << '\n';
    }
    //xxx if (part[3] != 1) {
    //xxx   cout << "Error: in Fix feature." << (int) part[3] << '\n';
    //xxx }
    //xxx 
    //xxx for (unsigned int i_v=0; i_v<H.getNumCells(); i_v++) {
    //xxx   H.getCell(i_v).setFixed();
    //xxx }
    //xxx for (unsigned int i_v=0; i_v<H.getNumCells(); i_v+=100) { // 1%
    //xxx   H.getCell(i_v).clrFixed();
    //xxx }
    //xxx H.setProxCells(3);
    //xxx H.getCell(3).setFixed();

    typedef
    MlPartTmpl
    < FMBiPartMgrBase, 
      FMPartTmpl<FMBiPartCore, FMBiGainMgr2>, 
      FMPartTmpl4<FMBiPartCore4, FMBiGainMgr2> 
    > MlBiPartMgr2;

    vector<unsigned char> part2(part);
    
    try {
      srand(1);
      unsigned short int seed=1;
      seed48(&seed);
      MlBiPartMgr2 P(param);  
      // P.setVCycle();
      // P.setNoInit(P.cutCost(part2));
      // P.setContractMethod(&Netlist::contractByMMC);
      P.setVerbosity(1);
      P.setBalanceTol(balanceTol);
      elapsed = P.doPartition(part2, numOfRuns);
      cout << " & " << std::fixed << std::setw(4) << P.cutCost(part2);
      cout << "(" << std::setprecision(2) << P.getAvgCost() << ")";
      cout << " & " << std::fixed << std::setw(5) << std::setprecision(2)
      	 << elapsed/numOfRuns;
      	 //xxx << elapsed;
      cout << " & " << std::fixed << std::setw(4) << std::setprecision(2) 
           << P.diff();
    } catch (FMException& e) {
      cout << "Error: " << e.what() << '\n';
    }

    //xxx for (unsigned int i_v=0; i_v<H.getNumCells(); i_v++) {
    //xxx   if (!H.getCell(i_v).isFixed()) continue;
    //xxx   if (part[i_v] != part2[i_v]) {
    //xxx     cout << "Error: in Fix feature A." << (int) part[3] << '\n';
    //xxx     break;
    //xxx   }
    //xxx }
    //xxx 
    //xxx if (part2[3] != 1) {
    //xxx   cout << "Error: in Fix feature." << (int) part[3] << '\n';
    //xxx }

    cout << "  \\\\" << endl;
  }    
}
