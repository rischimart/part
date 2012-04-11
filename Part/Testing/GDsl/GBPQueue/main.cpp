#include <GBPQueue.hpp>
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
  GBPQueue<GNode> PQ1(-10, 10);
  GBPQueue<GNode> PQ2(-10, 10);

  GNode d;
  GNode e;
  GNode f;

  PQ1.pushBack(e, 3);
  PQ1.pushBack(f, -10);
  PQ1.pushBack(d, 5);
  PQ2.pushBack(PQ1.popFront(), -6); // d
  PQ2.pushBack(PQ1.popFront(), 3);
  PQ2.pushBack(PQ1.popFront(), 0);
  
  PQ2.increaseKeyLIFOBy(d, 15);
  PQ2.decreaseKeyLIFOBy(d, 3);

  CHECK(PQ1.isEmpty());
  CHECK(PQ2.getMax() == 6);

  // Cursor
  PQ2.detach(d);
  int count = 0;
  GBPQueueCursor<GNode> pqc(PQ2);
  while (pqc.hasNext()) {
    pqc.getNext();
    count++;
  }
  CHECK(count == 2);

  // Const Cursor
  count = 0;
  GBPQueueCursor<GNode> pqc2(PQ2);
  while (pqc2.hasNext()) {
    GNode& d = pqc2.getNext();
    PQ2.detach(d);
    count++;
  }
  CHECK(count == 2);

  cout << "Test finished." << endl;
}
