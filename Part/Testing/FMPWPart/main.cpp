#include <FMPartTmpl.hpp>
// #include <FMPartTmpl4.hpp>
#include <FMParam.hpp>
#include <FMPWPartTmpl.hpp>
#include <FMBiGainMgr2.hpp>
//xxx #include <FMKGainMgr2.hpp>
//xxx #include <FMKGainMgr3.hpp>
//xxx #include <FMKWayPartCore.hpp>
//xxx #include <FMKWayGainMgr2.hpp>
#include <Netlist.hpp>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <time.h>

using namespace std;

int main(int argc, const char* argv[])
{
  double balanceTol = 0.1;
  int numOfRuns = 2;
  int numOfParts = 4;
 
  if (argc >= 2) {
    balanceTol = atof(argv[1]);
  }
  if (argc >= 3) {
    numOfRuns = atoi(argv[2]);
  }
  if (argc >= 4) {
    numOfParts = atoi(argv[3]);
  }

  const char* filename[] = { //xxx "null", "two", "dwarf1",
    "ibm01", "ibm02", "ibm03",
    "ibm04", "ibm05", "ibm06",
    "ibm07", "ibm08", "ibm09",
    "ibm10", "ibm11", "ibm12",
    "ibm13", "ibm14", "ibm15",
    "ibm16", "ibm17", "ibm18" };

  cout << "Compare " << endl;
  cout << "Balance tol = " << balanceTol << endl;
  cout << "Number of starts = " << numOfRuns << endl;
  cout << "Number of Partitions = " << numOfParts << endl;
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
    }   
    else {
      H.readAre(areFile.c_str());
    }
    in.close();

    // string hgrFile = "Nets/" + string(filename[i]) + "c.hgr";
    // if (!H.readHMetis(hgrFile.c_str())) {
    //   cout << "Could not read .hgr file " << hgrFile << endl;
    //   continue;
    // }    

    cout << filename[i];
    std::vector<unsigned char> part(H.getNumCells());
    
    //xxx Cell& c = H.getCell(0);
    //xxx c.setFixed();
    //xxx part[0] = 1;

    // Cell& c = H.getCell(123);
    // c.setVisited();

    
    try {
      srand(1);
      unsigned short int seed=1;
      seed48(&seed);
      const FMParam param(H,numOfParts,2);
      FMPWPartTmpl<FMBiGainMgr2> P(param);  
      P.setBalanceTol(balanceTol);
      //xxx P.setPValue(1);
      //xxx P.setQValue(4);

      // P.setBoundType(1);
      elapsed = P.doPartition(part, numOfRuns);
      
      //xxx if (part[0] != 1) {
      //xxx   cout << "Error: in Fix Cell" << '\n';
      //xxx }
      
      cout << " & " << std::fixed << std::setw(4) << P.cutCost(part);
      cout << "(" << std::setprecision(2) << P.getAvgCost() << ")";
      cout << " & " << std::fixed << std::setw(5) << std::setprecision(2)
      	 << elapsed/numOfRuns;
      cout << " & " << std::fixed << std::setw(4) << std::setprecision(2) 
           << P.diff();
      //xxx if (H.SOEDCost(part,numOfParts) != H.KMinus1Cost(part,numOfParts) + 
      //xxx     H.HyperEdgeCost(part)) {
      //xxx   cout << "Implementation of SOED/KMinus1/HEdge error !" << endl;
      //xxx }
    }
    catch (...) {
    }

    // for (unsigned int i_v=0; i_v<H.getNumCells(); i_v++) {
    //   H.getCell(i_v).setFixed();
    // }
    // H.getCell(5).clrFixed();
    
    //xxx try {
    //xxx   srand(1);
    //xxx   unsigned short int seed=1;
    //xxx   seed48(&seed);
    //xxx   const FMParam param(H,numOfParts,2);
    //xxx   FMPartTmpl<FMKWayPartCore, FMKWayGainMgr2> P2(param);  
    //xxx   P2.setBalanceTol(balanceTol);
    //xxx   // P2.setNoInit(H.cutCost(part, numOfParts));
    //xxx   elapsed = P2.doPartition(part, numOfRuns);
    //xxx   
    //xxx   //xxx if (part[0] != 1) {
    //xxx   //xxx   cout << "Error: in Fix Cell" << '\n';
    //xxx   //xxx }
    //xxx   
    //xxx   cout << " & " << std::fixed << std::setw(4) << P2.cutCost(part);
    //xxx   cout << "(" << std::setprecision(2) << P2.getAvgCost() << ")";
    //xxx   cout << " & " << std::fixed << std::setw(5) << std::setprecision(2)
    //xxx   	 << elapsed/numOfRuns;
    //xxx   cout << " & " << std::fixed << std::setw(4) << std::setprecision(2) 
    //xxx        << P2.diff();
    //xxx }
    //xxx catch (...) {
    //xxx }
    

    cout << "  \\\\" << endl;
  }    
}
