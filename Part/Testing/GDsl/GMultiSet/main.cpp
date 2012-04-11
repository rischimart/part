#include <GMultiSet.hpp>
#include <iostream>

using namespace std;

#define CHECK(_e)   if (!(_e)) print_error(__FILE__, __LINE__);

inline void print_error(const char* file, int lineno)
{
  cerr << "Test fail in " << file << ":" << lineno << endl;
}

int main()
{
  GMultiSet<double> S;
  CHECK(S.isEmpty());

  typedef GMultiSetItem MyItem;
  MyItem d,e,f,g,h,i;
  
  S.insert(g, 4.0);
  S.insert(h, 5);
  S.insert(i, 5);
  S.insert(d, 5);
  S.insert(e, 4);
  S.insert(f, 7);





  GDList<MyItem>* L = S.ascendingOrder();
  CHECK(L->isValid());
  //xxx delete L;

  cout << "Test finished." << endl;
}
