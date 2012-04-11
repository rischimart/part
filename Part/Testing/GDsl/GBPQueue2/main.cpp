#include <GBPQueue2.hpp>
#include <GNode.hpp>
#include <iostream>

using namespace std;

#define CHECK(_e)   if (!(_e)) print_error(__FILE__, __LINE__);

inline void print_error(const char* file, int lineno)
{
  cerr << "Test fail in " << file << ":" << lineno << endl;
}

int main()
{
  GBPQueue2<GNode2> PQ1(-10, 10, -4, 4);
  GBPQueue2<GNode2> PQ2(-10, 10, -4, 4);

  GNode2 d;
  GNode2 e;
  GNode2 f;

  PQ1.pushBack(e, 6, -2);
  PQ1.pushBack(f, -1, -3);
  PQ1.pushBack(d, 6, 3);
  PQ2.pushBack(PQ1.popFront(), -4, 3); // d
  PQ2.pushBack(PQ1.popFront(), 3, 3);
  PQ2.pushBack(PQ1.popFront(), 0, 0);
  
  //xxx PQ2.increaseKeyLIFOBy(d, 1, 4);
  //xxx PQ2.decreaseKeyLIFOBy(d, 3, 3);

  CHECK(PQ1.isEmpty());
  CHECK(PQ2.getMax() == 3);

  // Cursor
  PQ2.detach(d);
  int count = 0;
  GBPQueue2Cursor<GNode2> pqc(PQ2);
  while (pqc.hasNext()) {
    pqc.getNext();
    count++;
  }
  CHECK(count == 2);

  // Const Cursor
  count = 0;
  GBPQueue2Cursor<GNode2> pqc2(PQ2);
  while (pqc2.hasNext()) {
    GNode2& d = pqc2.getNext();
    PQ2.detach(d);
    count++;
  }
  CHECK(count == 2);

  cout << "Test finished." << endl;
}
