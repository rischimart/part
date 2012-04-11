#include <FMParam.hpp>
#include <FMPartTmpl.hpp>
#include <FMPartTmpl4.hpp>
#include <FMKWayPartCore.hpp>
#include <FMKWayPartCore4.hpp>
#include <FMKWayGainMgr2.hpp>
#include <FMBiPartCore.hpp>
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
  int numOfParts = 3;
 
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

  cout << "Compare single-bucket implementation with K-bucket implementation" << endl;
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
    }   
    else {
      H.readAre(areFile.c_str());
    }
    in.close();

    cout << filename[i];
    std::vector<unsigned char> part(H.getNumCells());
    
    Cell& c = H.getCell(0);
    c.setFixed();
    part[0] = 1;
    
    const FMParam param(H, numOfParts, 0);

    try {
      srand(1);
      unsigned short int seed=1;
      seed48(&seed);
      FMPartTmpl<FMKWayPartCore, FMKWayGainMgr2> P(param);  
      P.setBalanceTol(balanceTol);
      elapsed = P.doPartition(part, numOfRuns);
      
      if (part[0] != 1) {
        cout << "Error: in Fix Cell" << '\n';
      }
      
      cout << " & " << std::fixed << std::setw(4) << P.cutCost(part);
      cout << "(" << std::setprecision(2) << P.getAvgCost() << ")";
      cout << " & " << std::fixed << std::setw(5) << std::setprecision(2)
      	 << elapsed/numOfRuns;
      cout << " & " << std::fixed << std::setw(4) << std::setprecision(2) 
           << P.diff();
      if (H.SOEDCost(part,numOfParts) != H.KMinus1Cost(part,numOfParts) + 
          H.HyperEdgeCost(part)) {
        cout << "Implementation of SOED/KMinus1/HEdge error !" << endl;
      }
    }
    catch (...) {
    }

    // for (unsigned int i_v=0; i_v<H.getNumCells(); i_v++) {
    //   H.getCell(i_v).setFixed();
    // }
    // H.getCell(5).clrFixed();
    
    // try {
    //   srand(1);
    //   unsigned short int seed=1;
    //   seed48(&seed);
    //   FMPartTmpl4<FMKWayPartCore4, FMKWayGainMgr2> P2(H,numOfParts,0);  
    //   P2.setBalanceTol(balanceTol);
    //   elapsed = P2.doPartition(part, numOfRuns);
    //   
    //   if (part[0] != 1) {
    //     cout << "Error: in Fix Cell" << '\n';
    //   }
    //   
    //   cout << " & " << std::fixed << std::setw(4) << P2.cutCost(part);
    //   cout << "(" << std::setprecision(2) << P2.getAvgCost() << ")";
    //   cout << " & " << std::fixed << std::setw(5) << std::setprecision(2)
    //   	 << elapsed/numOfRuns;
    //   cout << " & " << std::fixed << std::setw(4) << std::setprecision(2) 
    //        << P2.diff();
    // }
    // catch (...) {
    // }
    

    cout << "  \\\\" << endl;
  }    
}
