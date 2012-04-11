#include <Netlist.hpp>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main(int, const char*)
{
  const char* filename[] = { "ibm01", "ibm02", "ibm03",
	                 "ibm04", "ibm05", "ibm06",
	                 "ibm07", "ibm08", "ibm09",
	                 "ibm10", "ibm11", "ibm12",
	                 "ibm13", "ibm14", "ibm15",
	                 "ibm16", "ibm17", "ibm18" };

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

    string hgrFile = "Nets/" + string(filename[i]) + ".hgr";
    cout << "Writing out " << hgrFile << "..." << endl;
    H.writeHMetis(hgrFile.c_str());
  }    
}
