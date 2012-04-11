/*****************************************************************

  This program demonstrates a problem when we try to replace the
  C-style array with STL vector.

*****************************************************************/
#include <iostream>
#include <vector>

using namespace std;

class link
{
private:
  link* next;
  link* prev;

  // Unimplemented
  link(const link&);
  const link& operator=(const link&);

  // If you comment out the above two declarations, compiler will
  // generate the default copy constructor and assignment operator
  // automatically.
public:

  link() { next = prev = this; }
  bool isNull() const { return next == this; }
  // ...
};


int main()
{
  link* A = new link [100];

  // If the above C-style array is replaced by the below STL vector,
  // there will be a compilation error because the constructor of
  // vector calls the copy constructor of "link", which is
  // unimplemented. If you comment out the above copy constructor
  // declaration so that default constructor is used, then this problem
  // can be compiled, but the result are different from that by
  // C-style array. 
  //
  // vector<link> A;

  for (int i=0; i<100; ++i) {
    if (!A[i].isNull()) {
      cout << "Error!" << endl;
      break;
    }
  }
}
