#include "setparthlpr.hpp"
#include <cstdio>

int main () 
{
  int n;
  int k;
  printf( "Enter n,k: " );  scanf( "%d %d", &n, &k );

  setPartHlpr S(n,k);
  S.run();
}
