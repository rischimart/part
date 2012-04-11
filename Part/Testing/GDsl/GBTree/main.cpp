#include <GBTree.hpp>
#include <iostream>

using namespace std;

#define CHECK(_e)   if (!(_e)) print_error(__FILE__, __LINE__);

inline void print_error(const char* file, int lineno)
{
  cerr << "Test fail in " << file << ":" << lineno << endl;
}

class TNode
{
public:
  TNode* _prev;  // left
  TNode* _next;  // right
  TNode() : _prev(0), _next(0) {}
};

int main()
{
  GBTree<TNode> T;

  TNode t1;
  TNode t2;
  TNode t3;
  TNode t4;

  t1._prev = &t2;
  t1._next = &t3;
  t2._next = &t4;

  T.setRoot(t1);

  GDList<TNode>* L = T.suffixOrder();
  CHECK(L->isValid());

  GDListCursor<TNode> lc(*L);
  int count = 0;
  while (lc.hasNext()) {
    lc.getNext();
    count++;
  }
  CHECK(count == 4);
  delete L;

  cout << "Test finished." << endl;
}
