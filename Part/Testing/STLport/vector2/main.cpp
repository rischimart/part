/*****************************************************************

  Check if your std::vector<> is a plaint
  pointer. (c.f. A. Alexandrescu, Modern C++ Design: Generic
  Programming and Design Patterns Applied)

*****************************************************************/
#include <iostream>
#include <vector>
#include <TypeTraits.h>

using namespace Loki;
using namespace std;

int main()
{
  const bool 
    iterIsPtr = TypeTraits<vector<int>::iterator>::isPointer;
  printf("Hhhh %d\n",iterIsPtr);

  cout << "Hi" << endl;
  cout << "vector<int>::iterator is " << 
    (iterIsPtr ? "fast" : "smart") << '\n'; 
  cout << "Hi" << endl;
}
