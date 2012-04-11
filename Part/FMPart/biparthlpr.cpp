#include "biparthlpr.hpp"
#include <iostream>
#include <cassert>  //added by Zhou


/** Constructor */
biPartHlpr::biPartHlpr(vector<unsigned char>& part) :
  _rg(&part[0]),
  _n(part.size()),
  _cnt(0)
{
}


/** Destructor */
biPartHlpr::~biPartHlpr()
{
}


/** Run the partition */
void biPartHlpr::run()
{ 
  size_t i;

  for (i=0; i<_n-1; ++i) _rg[i] = 0;
  _rg[_n-1] = 1;
  _cnt = 0;
  ++_cnt;
  process_init();
  GEN0_even(_n);
  assert(stirling2nd(_n, 2) == _cnt);
}


/** Default processing function */
void biPartHlpr::process_init () 
{
  size_t i;

  std::cout << _cnt << ": ";
  for (i=0; i<_n; ++i ) std::cout << (size_t) _rg[i];  
  std::cout << std::endl;
}


/** Default processing function */
void biPartHlpr::process ( size_t x, bsize_t y ) 
{
  size_t i;

  std::cout << _cnt << ": ";
  bsize_t old = _rg[x];

  _rg[x] = y;
  for (i=0; i<_n; ++i) std::cout << (size_t) _rg[i];  
  std::cout << "   moved element " << x 
            << " from block " << (size_t) old 
            << " to " << (size_t) y;
  std::cout << std::endl;
}



// Stirling number of second kind. Note that this function is for
// testing purpose only and is slow because of reducdant calculation.
size_t biPartHlpr::stirling2nd(size_t n, bsize_t k)
{
  if (k >= n || k <= 1) return 1;
  return stirling2nd(n-1, k-1) + k * stirling2nd(n-1, k);
}


// The lists S(n,k,0) and S(n,k,1) satisfy the following properties.
// 1. Successive RG sequences differ in exactly one position.
// 2. first(S(n,k,0)) = first(S(n,k,1)) = 0^{n-k}0123...(k-1)
// 3. last(S(n,k,0)) = 0^{n-k}12...(k-1)0
// 4. last(S(n,k,1)) = 012...(k-1)0^{n-k}
// Note that first(S'(n,k,p)) = last(S(n,k,p))

// S(n,k,0) even k
void biPartHlpr::GEN0_even ( size_t n ) 
{
  if (n <= 2) return;

  //xxx GEN0_odd(n-1);      // S(n-1, k-1, 0).(k-1)
  move(n-1, 1);
  GEN1_even(n-1);     // S(n-1, k, 1).(k-1)
  move(n, 0);
  NEG1_even(n-1);     // S'(n-1, k, 1).(k-2)  
}


// S'(n, k, 0) even k
void biPartHlpr::NEG0_even ( size_t n ) 
{
  if (n <= 2) return;

  GEN1_even(n-1);     // S(n-1, k, 1).(k-2)
  move(n, 1);
  NEG1_even(n-1);     // S(n-1, k, 1).(k-1)
  move(n-1, 0);      
  //xxx NEG0_odd(n-1);    // S(n-1, k-1, 1).(k-1)
}


// S(n, k, 1) even k
void biPartHlpr::GEN1_even ( size_t n ) 
{
  if (n <= 2) return;

  //xxx GEN1_odd(n-1);
  move(2, 1);
  NEG1_even(n-1);
  move(n, 0);
  GEN1_even(n-1);
}


// S'(n, k, 1) even k
void biPartHlpr::NEG1_even ( size_t n ) 
{
  if (n <= 2) return;

  NEG1_even(n-1);
  move(n, 1);
  GEN1_even(n-1);
  move(2, 0);
  //xxx NEG1_odd(n-1);
}


