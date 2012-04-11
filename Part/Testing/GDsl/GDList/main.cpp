#include <GDList.hpp>
#include <iostream>

using namespace std;

#define CHECK(_e)   if (!(_e)) { print_error(__FILE__, __LINE__); \
	cerr << #_e << endl; }

inline void print_error(const char* file, int lineno)
{
  cerr << "Test fail in " << file << ":" << lineno << endl;
}

class DNode
{
public:
  DNode* _next;
  DNode* _prev;
};

int main()
{
  GDList<DNode> L1;
  GDList<DNode> L2;
  DNode d;
  DNode e;
  DNode f;

  L1.pushBack(e);
  L1.pushFront(f);
  L1.pushBack(d);
  L2.pushBack(L1.popBack());
  L2.pushFront(L1.popFront());
  L2.concat(L1);
  CHECK(L1.isEmpty());
  CHECK(L2.isValid());

  // List Cursor
  int count = 0;
  GDListCursor<DNode> lc(L2);
  while (lc.hasNext()) {
    lc.getNext();
    count++;
  }
  CHECK(count == 3);

  
  count = 0;
  GDListCursor<DNode> lcc(L2);
  while (lcc.hasNext()) {
    DNode& g = lcc.getNext();
    L2.detach(g); // detach the node should not invalidate the cursor
    count++;
  }
  CHECK(count == 3);

  cout << "Test finished." << endl;
}
