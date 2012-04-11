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

    int numCells = H.getNumCells();
    int numNets  = H.getNumNets();
    int numPins  = H.getNumPins();
    int count = 0;

    Netlist::SPN H1 = H.contractByMMC();
    H.contractPhase2(H1);
    numCells += H1->getNumCells();
    numNets  += H1->getNumNets();
    numPins  += H1->getNumPins();
    count++;

    while (H1->getNumCells() > H1->getMaxDegree()) { 
      Netlist::SPN H2 = H1->contractByMMC();
      H1->contractPhase2(H2);
      cout << filename[i] << setprecision(4);
      cout << " & " << H1->getNumCells() << " " << H2->getNumCells();
      cout << " & " << H2->getMaxDegree() << '\n';
      H1 = H2;
      numCells += H1->getNumCells();
      numNets  += H1->getNumNets();
      numPins  += H1->getNumPins();
      count++;
    }

    string hgrFile = "Nets/" + string(filename[i]) + "c.hgr";
    H1->writeHMetis(hgrFile.c_str());

    cout << filename[i];        
    cout << " & " << std::setw(4) << count;
    cout << " & " << std::setw(10) << numCells;
    cout << " & " << std::setw(10) << numNets;
    cout << " & " << std::setw(10) << numPins;
    cout << " & " << std::setw(11) <<  sizeof(Net) * numNets + 
      sizeof(Pin) * numPins + (sizeof(Cell) + sizeof(void*)) * numCells;
    cout << "   \\\\" << endl;

  }    
}
