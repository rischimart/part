#ifndef BI_PART_HLPR_HPP
#define BI_PART_HLPR_HPP

#include <vector>
using std::vector;

/**
 * Set Partition Helper (k=2)
 * 
 * A set partition of the set [n] = {1,2,3,...,n} is a collection B0,
 * B1, ... Bj of disjoint subsets of [n] whose union is [n]. Each Bj
 * is called a block. Below we show the partitions of [4]. The periods
 * separtate the individual sets so that, for example, 1.23.4 is the
 * partition {{1},{2,3},{4}}.
 *   1 block:  1234
 *   2 blocks: 123.4   124.3   134.2   1.234   12.34   13.24   14.23
 *   3 blocks: 1.2.34  1.2.3.4  1.24.3  14.2.3  13.2.4  12.3.4
 *   4 blocks: 1.2.3.4
 * 
 * Each partition above has its blocks listed in increasing order of
 * smallest element; thus block 0 contains element 1, block1 contains
 * the smallest element not in block 0, and so on. A Restricted Growth
 * string (or RG string) is a sring rg[0..n-1] where rg[i] is the block in
 * which element i occurs. Restricted Growth strings are often called
 * restricted growth functions. Here are the RG strings corresponding
 * to the partitions shown above.
 * 
 *   1 block:  0000
 *   2 blocks: 0001, 0010, 0100, 0111, 0011, 0101, 0110
 *   3 blocks: 0122, 0121, 0112, 0120, 0102, 
 * 
 * ...more
 * 
 * Reference:
 * Frank Ruskey. Simple combinatorial Gray codesconstructed by
 * reversing sublists. Lecture Notes in Computer Science, #762,
 * 201-208. Also downloadable from
 * http://csr.csc.uvic.ca/home/fruskey/Publiccations/SimpleGray.html
 *
 * In this implementation, blocks are representated by "unsigned
 * char". Therefore, number of blocks are limited to 256.
 */  
class biPartHlpr
{
  typedef unsigned char bsize_t;

private:
  bsize_t*  _rg;     /**< pointer to Restricted Growth array */
  size_t    _n;      /**< number of elements */
  size_t    _cnt;    /**< record how many testing so far */

public:
  /** Constructor */
  biPartHlpr(vector<unsigned char>& part);

  /** Destructor */
  virtual ~biPartHlpr();

  /** Run the partition */
  void run();

protected:
  /** Process initial partition (user re-definable) */
  virtual void process_init();

  /** Process when element x moved to block y (user re-definable) */
  virtual void process ( size_t x, bsize_t y );

private:
  // Unimplement
  biPartHlpr(const biPartHlpr& );
  biPartHlpr& operator=(const biPartHlpr& );

  /** 
   * Stirling number of second kind. Note that this function is for
   * testing purpose only and is slow because of reducdant calculation. 
   */
  static size_t stirling2nd ( size_t n, bsize_t k );

  /** Obtain next combination by moving element x to block y */
  inline void move ( size_t x, bsize_t y ) 
  { ++_cnt; process( x-1, y ); }

  /** S(n,k,0) even k */
  void GEN0_even ( size_t n );  

  /** S'(n,k,0) even k */
  void NEG0_even ( size_t n );  

  /** S(n,k,1) even k */
  void GEN1_even ( size_t n );  

  /** S'(n,k,1) even k */
  void NEG1_even ( size_t n );  

  /** S(n,k,0) odd k */
  void GEN0_odd ( size_t n );  

  /** S'(n,k,0) odd k */
  void NEG0_odd ( size_t n );  

  /** S(n,k,1) odd k */
  void GEN1_odd ( size_t n );  

  /** S'(n,k,1) odd k */
  void NEG1_odd ( size_t n );  
};

#endif


