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
    string hgrFile = "Nets/" + string(filename[i]) + "c.hgr";
    if (!H.readHMetis(hgrFile.c_str())) {
      cout << "Could not read .hgr file " << hgrFile << endl;
      continue;
    }    

    int numCells = H.getNumCells();
    int numNets  = H.getNumNets();
    int numPins  = H.getNumPins();
    int maxDegree = H.getMaxDegree();

    cout << filename[i];
    cout << " & " << std::setw(10) << numCells;
    cout << " & " << std::setw(10) << numNets;
    cout << " & " << std::setw(10) << numPins;
    cout << " & " << std::setw(10) << maxDegree;
    cout << "   \\\\" << endl;
  }    
}
