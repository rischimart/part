#include <Netlist.hpp>
#include <Cell.hpp>
#include <map>
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>

using namespace std;

int main(int argc, const char* argv[])
{
  double balanceTol = 0.02;

  if (argc >= 2) {
    balanceTol = atof(argv[1]);
  }

  const char* filename[] = { //xxx "dwarf1", 
	                     "ibm01", "ibm02", "ibm03",
	                     "ibm04", "ibm05", "ibm06",
	                     "ibm07", "ibm08", "ibm09",
	                     "ibm10", "ibm11", "ibm12",
	                     "ibm13", "ibm14", "ibm15",
	                     "ibm16", "ibm17", "ibm18" };

  cout << "Balance tol = " << balanceTol << endl;
  cout << "Circuit";
  for (int k=2; k<=32; k*=2) {
    cout << " & k=" << k;
  }
  cout << "   \\\\\n";
  cout << "\\hline %--------------------------------------------------------%";
  cout << endl;

  for (int i=0; i<18; i++) {
    Netlist H;

    // string netDFile = "Nets/" + string(filename[i]) + ".netD";
    // if (!H.readNetD(netDFile.c_str())) {
    //   cout << "Could not read .netD file " << netDFile << endl;
    //   continue;
    // }    
    // string areFile = "Nets/" + string(filename[i]) + ".are";
    // ifstream in(areFile.c_str());
    // if (in.fail()) {
    //   cout << "Could not open .are file " << areFile << endl;
    // }   
    // else {
    //   H.readAre(areFile.c_str());
    // }
    // in.close();

    string hgrFile = "Nets/" + string(filename[i]) + "c.hgr";
    if (!H.readHMetis(hgrFile.c_str())) {
      cout << "Could not read .hgr file " << hgrFile << endl;
      continue;
    }    

    H.initVertexArray();
    cout << filename[i];
    for (int k=2; k<=32; k*=2) {
      cout << " & " << (H.isVILEOk(balanceTol, k) ? "+" : "-");
    }
    cout << "   \\\\" << endl;

  }    
}
