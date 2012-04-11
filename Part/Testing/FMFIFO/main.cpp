/**
 * This example shows the effect of the FIFO updating.
 */
#include <FMParam.hpp>
#include <FMPartTmpl.hpp>
#include <FMBiPartCore.hpp>
#include <FMBiGainMgr3.hpp>  // FIFO
#include <FMBiGainMgr2.hpp>  // LIFO
#include <Netlist.hpp>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <time.h>

using namespace std;

int main(int argc, const char* argv[])
{
  double balanceTol = 0.10;
  int numOfRuns = 20;

  if (argc >= 2) {
    balanceTol = atof(argv[1]);
  }
  if (argc >= 3) {
    numOfRuns = atoi(argv[2]);
  }

  const char* filename[] = { //xxx "null", "two", "dwarf1",
	                     "ibm01", "ibm02", "ibm03",
			     "ibm04", "ibm05", "ibm06",
			     "ibm07", "ibm08", "ibm09",
			     "ibm10", "ibm11", "ibm12",
			     "ibm13", "ibm14", "ibm15",
			     "ibm16", "ibm17", "ibm18" };

  cout << "Compare single-bucket implementation with two-bucket implementation" << endl;
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

    // Cell& c = H.getCell(0);
    // c.setFixed();
    // part[0] = 1;
    FMParam param(H,2,0);

    try {
    srand(1);
    FMPartTmpl<FMBiPartCore, FMBiGainMgr3> P(param);  
    P.setBalanceTol(balanceTol);
    elapsed = P.doPartition(part, numOfRuns);

    // if (part[0] != 1) {
    //    cout << "Error: in Fix Cell" << '\n';
    // }

    cout << " & " << std::fixed << std::setw(4) << P.cutCost(part);
    cout << "(" << std::setprecision(2) << P.getAvgCost() << ")";
    cout << " & " << std::fixed << std::setw(5) << std::setprecision(2)
	 << elapsed/numOfRuns;
    cout << " & " << std::fixed << std::setw(4) << std::setprecision(2) 
         << P.diff();
    }
    catch (...) {
    }

    // for (unsigned int i_v=0; i_v<H.getNumCells(); i_v++) {
    //   H.getCell(i_v).setFixed();
    // }
    // for (unsigned int i_v=0; i_v<H.getNumCells(); i_v+=100) {
    //   H.getCell(i_v).clrFixed();
    // }
    // H.setProxCells(3);
    // H.getCell(0).setFixed();

    try {
    srand(1);
    FMPartTmpl<FMBiPartCore, FMBiGainMgr2> P2(param);  
    P2.setBalanceTol(balanceTol);
    elapsed = P2.doPartition(part, numOfRuns);
    
    // if (part[0] != 1) {
    //   cout << "Error: in Fix Cell" << '\n';
    // }
    
    
    cout << " & " << std::fixed << std::setw(4) << P2.cutCost(part);
    cout << "(" << std::setprecision(2) << P2.getAvgCost() << ")";
    cout << " & " << std::fixed << std::setw(5) << std::setprecision(2)
    	 << elapsed/numOfRuns;
    cout << " & " << std::fixed << std::setw(4) << std::setprecision(2) 
         << P2.diff();
    
    }
    catch (...) {
    }

    cout << "  \\\\" << endl;
  }    
}
