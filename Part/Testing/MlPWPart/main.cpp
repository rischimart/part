#include <FMParam.hpp>
#include <MlPartTmpl.hpp>
#include <FMException.hpp>
#include <FMPartTmpl.hpp>
#include <FMPartTmpl4.hpp>
#include <FMKWayPartCore.hpp>
#include <FMKWayPartCore4.hpp>
#include <FMKWayGainMgr2.hpp>
#include <FMPWPartTmpl.hpp>
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
  double balanceTol = 0.10;
  int numOfRuns = 20;
  int K = 8;
  int costModel = 2;
  int lr = 1;

  if (argc >= 2) {
    balanceTol = atof(argv[1]);
  }
  if (argc >= 3) {
    numOfRuns = atoi(argv[2]);
  }
  if (argc >= 4) {
    K = atoi(argv[3]);
  }
  if (argc >= 5) {
    costModel = atoi(argv[4]);
  }
  if (argc >= 6) {
    lr = atoi(argv[5]);
  }

  const char* filename[] = { "ibm01", "ibm02", "ibm03",
			     "ibm04", "ibm05", "ibm06",
			     "ibm07", "ibm08", "ibm09",
			     "ibm10", "ibm11", "ibm12",
			     "ibm13", "ibm14", "ibm15",
			     "ibm16", "ibm17", "ibm18" };

  cout << "Compare K with KWay" << endl;
  cout << "Balance tol = " << balanceTol << endl;
  cout << "Number of starts = " << numOfRuns << endl;
  cout << "Number of Partitions = " << K << endl;
  cout << "Cost model = " << costModel << endl;
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

    typedef
      MlPartTmpl
      < FMKWayPartMgrBase, 
      FMPWPartTmpl<FMBiGainMgr2>, 
      // FMPWPartTmpl<FMBiGainMgr2> 
      FMPartTmpl4<FMKWayPartCore4, FMKWayGainMgr2> 
      > MlPWPartMgr;

    MlPWPartMgr* P2 = 0;
    const FMParam param(H, K, costModel);

    try {
      srand(1);
      unsigned short int seed=1;
      seed48(&seed);
      P2 = new MlPWPartMgr(param); 
      P2->setContractMethod(&Netlist::contractByMMC);
      // P2->setVCycle();
      P2->setPValue(1);
      P2->setQValue(10);
      P2->setVerbosity(0);
      P2->setBalanceTol(balanceTol);
      P2->setLocalityRatio(lr);

      // P2->setBoundType(1);
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
      P.setPValue(1);
      P.setQValue(10);
      P.setContractMethod(&Netlist::contractByMMC);
      P.setVerbosity(0);
      P.setBalanceTol(balanceTol);
      P.setLocalityRatio(lr);
      // P.setBoundType(1);
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

    cout << "  \\\\" << endl;
  }    
}
