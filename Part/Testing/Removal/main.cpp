#include <CycleRemovalServ.hpp>
#include <Netlist.hpp>
#include <Cell.hpp>
#include <map>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>

using namespace std;

int main(int, const char*)
{
  const char* filename[] = { //xxx "dwarf1", 
	                     "ibm01", "ibm02", "ibm03",
	                     "ibm04", "ibm05", "ibm06",
	                     "ibm07", "ibm08", "ibm09",
	                     "ibm10", "ibm11", "ibm12",
	                     "ibm13", "ibm14", "ibm15",
	                     "ibm16", "ibm17", "ibm18" };

  cout << "Circuit & #cells & #pads & #modules &  #nets &  #pins & #W";
  cout << "   \\\\\n";
  cout << "\\hline %--------------------------------------------------------%";
  cout << endl;

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
 
    
    const int n = H.getNumCells();
    typedef std::map<int, Cell*> WeightCellMap;
    WeightCellMap aMap;
    
    for (int i_v=0; i_v<n; i_v++) {
      Cell& v = H.getCell(i_v);
      aMap[v.getWeight()] = &v;
    }

    cout << filename[i];        
    cout << "   & " << std::setw(6) << H.getNumCells() - H.getNumPads();
    cout << " & " << std::setw(5) << H.getNumPads();
    cout << " & " << std::setw(8) << H.getNumCells();
    cout << " & " << std::setw(6) << H.getNumNets();
    cout << " & " << std::setw(6) << H.getNumPins();
    //xxx cout << " & " << std::setw(2) << aMap.size();
    cout << " & " << std::setw(3) << H.getMaxDegree();
    cout << "   \\\\" << endl;

    CycleRemovalServ crs(H);
    crs.doRemove();
    //xxx Netlist* H1 = crs.contract();
    Netlist* H1 = 0;
    while (H1->getNumCells() > H1->getMaxDegree() ) { 

    cout << filename[i];        
    cout << "c  & " << std::setw(6) << H1->getNumCells() - H1->getNumPads();
    cout << " & " << std::setw(5) << H1->getNumPads();
    cout << " & " << std::setw(8) << H1->getNumCells();
    cout << " & " << std::setw(6) << H1->getNumNets();
    cout << " & " << std::setw(6) << H1->getNumPins();
    cout << " & " << std::setw(3) << H1->getMaxDegree();
    cout << "   \\\\" << endl;
    CycleRemovalServ crs2(*H1);
    crs2.doRemove();
    //xxx Netlist* H2 = crs2.contract();
    Netlist* H2 = 0;
    delete H1;
    H1 = H2;

    }

    delete H1;
  }    
}
