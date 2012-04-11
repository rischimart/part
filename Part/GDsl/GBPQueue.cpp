#include "GBPQueue.hpp"
#include <cassert>  //added by Zhou

/******************************************************

 Feature 'export' is not implemented by some compilers



#include "GBPQueue.hpp"


export template <class BPQNode>
GBPQueue<BPQNode>::GBPQueue(int a, int b)
{ 
  assert(b >= a);  
  _offset = a - 1;
  _high = b - a + 1;
  _max = 0;
  _T = new GDList<BPQNode>[b-a+2];
}

******************************************************/
