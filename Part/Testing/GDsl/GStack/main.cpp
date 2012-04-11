#include <GStack.hpp>
#include <iostream>

using namespace std;

#define CHECK(_e)   if (!(_e)) print_error(__FILE__, __LINE__);

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
  GStack<DNode> S1;
  GStack<DNode> S2;

  S1.push(new DNode);
  S1.pushFront(new DNode);
  S2.push(S1.pop());
  S2.pushFront(S1.popFront());
  CHECK(S1.isEmpty());

  // Cursor
  int count = 0;
  GStackCursor<DNode> sc(S2);
  while (sc.hasNext()) {
    sc.getNext();
    count++;
  }
  CHECK(count == 2);
  CHECK(S2.isValid());

  // Const Cursor
  count = 0;
  GStackConstCursor<DNode> sc2(S2);
  while (sc2.hasNext()) {
    sc2.getNext();
    count++;
  }
  CHECK(count == 2);
  CHECK(S2.isValid());

  cout << "Test finished." << endl;
}
