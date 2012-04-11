#include <FMParam.hpp>
#include <MlPartTmpl.hpp>
#include <FMException.hpp>
#include <FMPartTmpl.hpp>
#include <FMPartTmpl4.hpp>
#include <FMKWayPartCore.hpp>
#include <FMKWayPartCore4.hpp>
#include <FMKWayGainMgr2.hpp>
#include <FMKWayGainMgr4.hpp>
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
  int K = 4;

  if (argc >= 2) {
    balanceTol = atof(argv[1]);
  }
  if (argc >= 3) {
    numOfRuns = atoi(argv[2]);
  }
  if (argc >= 4) {
    K = atoi(argv[3]);
  }

  const char* filename[] = { //xxx "null", "two", "dwarf1",
	                     "ibm01", "ibm02", "ibm03",
			     "ibm04", "ibm05", "ibm06",
			     "ibm07", "ibm08", "ibm09",
			     "ibm10", "ibm11", "ibm12",
			     "ibm13", "ibm14", "ibm15",
			     "ibm16", "ibm17", "ibm18" };

  cout << "Compare SOED , K-1 Metric, HyperEdge" << endl;
  cout << "Balance tol = " << balanceTol << endl;
  cout << "Number of starts = " << numOfRuns << endl;
  cout << "Number of Partitions = " << K << endl;
  cout << "Circuit";
  cout << " & Cut(Min) & CPU(sec.) & Diff(%)";
  cout << " & Cut(Min) & CPU(sec.) & Diff(%)";
  cout << "  \\\\\n";
  cout << "\\hline %--------------------------------------------------------%";
  cout << endl;
  
  double elapsed;

  for (unsigned int i=0; i<18; i++) {
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
    < FMKWayPartMgrBase, 
      // FMKPartTmpl<FMKGainMgr2>, 
      // FMKPartTmpl<FMKGainMgr2> 
      FMPartTmpl<FMKWayPartCore, FMKWayGainMgr2>,
      FMPartTmpl4<FMKWayPartCore4, FMKWayGainMgr2>
    > MlKWayPartMgr;

    MlKWayPartMgr* P2 = 0;

    const FMParam param(H, K, 1);

    try {
      srand(1);
      unsigned short int seed=1;
      seed48(&seed);
      P2 = new MlKWayPartMgr(param); 
      P2->setContractMethod(&Netlist::contractByMMC);
      // P2->setVCycle();
      P2->setVerbosity(0);
      P2->setBalanceTol(balanceTol);
      P2->setPValue(1);
      P2->setQValue(5);
      P2->setBoundType(1);
      elapsed = P2->doPartition(part, numOfRuns);
      cout << " & " << std::fixed << std::setw(4) << P2->cutCost(part);
      // cout << " & " << std::fixed << std::setw(4) << H.KMinus1Cost(part, K);
      // cout << " & " << std::fixed << std::setw(4) << H.SOEDCost(part, K);
      cout << "(" << std::setprecision(2) << P2->getAvgCost() << ")";
      // cout << " & " << std::fixed << std::setw(4) << H.HyperEdgeCost(part);
      cout << " & " << std::fixed << std::setw(5) << std::setprecision(2)
           << elapsed/numOfRuns;
      cout << " & " << std::fixed << std::setw(4) << std::setprecision(2) 
           << P2->diff();
    } catch (FMException& e) {
      cout << "Error: " << e.what() << '\n';
    }
    catch (std::bad_alloc) {
      cout << "Error: " << "Insufficient memory." << '\n';
    }

    delete P2;

    //xxx if (part[3] != 1) {
    //xxx   cout << "Error: in Fix feature." << (int) part[3] << '\n';
    //xxx }

    // for (unsigned int i_v=0; i_v<H.getNumCells(); i_v++) {
    //   H.getCell(i_v).setFixed();
    // }
    // for (unsigned int i_v=0; i_v<H.getNumCells(); i_v+=100) {
    //   H.getCell(i_v).clrFixed();
    // }
    // H.setProxCells(1);
    // H.getCell(3).setFixed();

    typedef
    MlPartTmpl
    < FMKWayPartMgrBase, 
      FMPartTmpl<FMKWayPartCore, FMKWayGainMgr2>, 
      FMPartTmpl4<FMKWayPartCore4, FMKWayGainMgr2> 
    > MlKWayPartMgr2;

    try {
      srand(1);
      unsigned short int seed=1;
      seed48(&seed);
      MlKWayPartMgr2 P(param);  
      // P.setVCycle();
      // P.setNoInit(P.cutCost(part));
      P.setContractMethod(&Netlist::contractByMMC);
      P.setVerbosity(0);
      P.setBalanceTol(balanceTol);
      // P.setPValue(1);
      // P.setQValue(4);
      P.setBoundType(1);
      elapsed = P.doPartition(part, numOfRuns);
      cout << " & " << std::fixed << std::setw(4) << P.cutCost(part);
      cout << "(" << std::setprecision(2) << P.getAvgCost() << ")";
      // cout << " & " << std::fixed << std::setw(4) << H.HyperEdgeCost(part);
      cout << " & " << std::fixed << std::setw(5) << std::setprecision(2)
       	   << elapsed/numOfRuns;
      cout << " & " << std::fixed << std::setw(4) << std::setprecision(2) 
           << P.diff();
    } catch (FMException& e) {
      cout << "Error: " << e.what() << '\n';
    }
    catch (std::bad_alloc) {
      cout << "Error: " << "Insufficient memory." << '\n';
    }
    //xxx if (part[3] != 1) {
    //xxx   cout << "Error: in Fix feature." << (int) part[3] << '\n';
    //xxx }
    
    //xxx try {
    //xxx   srand(1);
    //xxx   unsigned short int seed=1;
    //xxx   seed48(&seed);
    //xxx   MlKWayPartMgr P(H, K, 0);  
    //xxx   // P.setVCycle();
    //xxx   // P.setNoInit(P.cutCost(part));
    //xxx   // P.setContractMethod(&Netlist::contractByISC);
    //xxx   P.setVerbosity(0);
    //xxx   P.setBalanceTol(balanceTol);
    //xxx   elapsed = P.doPartition(part, numOfRuns);
    //xxx   cout << " & " << std::fixed << std::setw(4) << P.cutCost(part);
    //xxx   cout << "(" << std::setprecision(2) << P.getAvgCost() << ")";
    //xxx   // cout << " & " << std::fixed << std::setw(4) << H.HyperEdgeCost(part);
    //xxx   cout << " & " << std::fixed << std::setw(5) << std::setprecision(2)
    //xxx   	   << elapsed/numOfRuns;
    //xxx   // cout << " & " << std::fixed << std::setw(4) << std::setprecision(2) 
    //xxx   //      << P.diff();
    //xxx } catch (FMException& e) {
    //xxx   cout << "Error: " << e.what() << '\n';
    //xxx }
    //xxx catch (std::bad_alloc) {
    //xxx   cout << "Error: " << "Insufficient memory." << '\n';
    //xxx }
    //xxx //xxx if (part[3] != 1) {
    //xxx //xxx   cout << "Error: in Fix feature." << (int) part[3] << '\n';
    //xxx //xxx }

    cout << "  \\\\" << endl;
  }    
}
