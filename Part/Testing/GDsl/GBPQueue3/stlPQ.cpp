#include <iostream>
#include <queue>
using namespace std;

int main()
{
  const int n = 10000000;
  const int k = 10000;
  srand(1);

  priority_queue<int> PQ;

  int i;
  for (i=0; i<n; i++) {
    PQ.push(rand()%(2*k) - k);
  }  

  while (!PQ.empty()) {
    PQ.top();
    PQ.pop();
  }

  cout << "Test finished." << endl;
}
