#include <GPoint.hpp>
#include <iostream>

using namespace std;

#define CHECK(_e)   if (!(_e)) print_error(__FILE__, __LINE__);

inline void print_error(const char* file, int lineno)
{
  cerr << "Test fail in " << file << ":" << lineno << endl;
}

int main()
{
  GPoint p(2,3);
  GPoint q(p);

  q.rotate90_then_reflect();
  q.rotate90_then_reflect();

  CHECK(p == q);

  cout << "Test finished." << endl;
}
