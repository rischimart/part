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
  const int n = 2000000;
  const int k = 1000;
  srand(1);

  GNode* nodearray = new GNode[n];
  GBPQueue<GNode> PQ1(-k, k);

  int i;
  for (i=0; i<n; i++) {
    PQ1.pushBack(nodearray[i], rand()%(2*k) - k);
  }
  
  while (!PQ1.isEmpty()) {
    PQ1.popFront();
  }

  cout << "Test finished." << endl;
}
