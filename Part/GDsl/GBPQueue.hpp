#ifndef G_BPQUEUE_HPP
#define G_BPQUEUE_HPP
#include <cassert>  //added by Zhou

#ifndef G_DLIST_HPP
#include "GDList.hpp"
#endif

/** @defgroup group2 Data Structure Group
 *  @ingroup Part
 *
 * This group contains data structure classes such as doubly-linked
 * list and bounded priority queue. Note that in order to acheive the
 * best performance, the class may be tailered for circuit
 * partitioning purpose. Please use with caution.
 *
 * For developers:
 * Some compilers do not support 'export' feature for template codes
 * (c.f. B. Stroustrup, The C++ Programming Language, 3rd ed. Section 13.7). 
 * Previously we encountered a lot of compilation issue with
 * non-inlined template codes.
 * 
 * @{
 */
 
/* Forward declaration */
template <class BPQNode> class GBPQueueCursor;
template <class BPQNode> class GBPQueueConstCursor;


/**
 * Bounded Priority Queue with integer keys in [a..b]. 
 * Implemented by array (bucket) of doubly-linked lists. 
 * Efficient if key is bounded by a small integer value.
 * Note that this class does not own the PQ nodes, even though PQ node
 * class is defined in this header file. This feature makes the nodes
 * sharable between doubly linked list class and this class. In the FM
 * algorithm, the node either attached to the gain buckets (PQ) or in the
 * freeVertexList (doubly linked list), but not in both of them in the
 * same time.
 *
 * Another improvement is to make the array size one element bigger
 * i.e. (b - a + 2). The extra dummy array element (which is T[0])
 * is used to reduce the boundary checking during updating.
 *
 * All the member functions assume that the keys are within the bound.
 */
template <class BPQNode>
class GBPQueue
{
  friend class GBPQueueCursor<BPQNode>;
  friend class GBPQueueConstCursor<BPQNode>;

public:
  GBPQueue() : _offset(0), _high(0), _max(0), _T(0) {}

  /** 
   * @brief  Constructor.
   * @param  a   lower bound
   * @param  b   upper bound
   * Precondition: a <= b 
   */
  GBPQueue(int a, int b);

  /** 
   * Destructor. 
   */
  ~GBPQueue() { if (_T != 0) delete [] _T; }

  /** 
   * @brief  Setup.
   * @param  a   lower bound
   * @param  b   upper bound
   * Precondition: a <= b 
   */
  void setUp(int a, int b);

  /**
   * @brief  Get the key.
   * @param  it  the item   
   * @return the key of it. 
   */
  int getKey(const BPQNode& it) const { return it._key + _offset; }

  /** @return the max value. */
  int getMax() const { return _max + _offset; }

  /** @return true if the PQ is empty. */
  bool isEmpty() const { return _max == 0; }

  /** @return true if the PQ is valid. Take O(n) time. */
  bool isValid() const;

  /** (For restricted used only) Make dummy. */
  void makeDummy() { _max = 1; }

  /** Reset the PQ. */
  void clear();

  /** 
   * @brief  Insert item at the end of this queue. 
   * @param  it  the item   
   * @param  k   the key
   *  Precondition: PQ does not contain it; a <= k <= b. 
   */
  void pushBack(BPQNode& it, int k);

  /** 
   * @brief  Insert item at the end without update the max value. 
   * @param  it  the item   
   * @param  k   the key
   * Precondition: PQ does not contain it; a <= k <= b.
   * Call resync() after calling this method.
   */
  void pushBackFast(BPQNode& it, int k);

  /** 
   * @brief  Arbitrarily push item without update the max value. 
   * @param  it  the item   
   * @param  k   the key
   * Precondition: PQ does not contain it; a <= k <= b.
   * Call resync() after calling this method.
   */
  // void pushRandomFast(BPQNode& it, int k);

  /** Update the max value */
  void resync();

  /** Pop an item with highest key. Precondition: PQ is not empty.*/
  BPQNode& popFront();

  /** @brief  Decrease key by m. Precondition: PQ contains it.
      @param  it  the item   
      @param  m   the change of key
      Note that order of items with same key will not be preserved.
      For FM algorithm, this is a prefered behavior. */
  void decreaseKeyLIFOBy(BPQNode& it, unsigned int m);

  /** @brief  Increase key by m. Precondition: PQ contains it.
      @param  it  the item   
      @param  m   the change of key
      Note that order of items with same key will not be preserved.
      For FM algorithm, this is a prefered behavior. */
  void increaseKeyLIFOBy(BPQNode& it, unsigned int m);

  void promote(BPQNode& it);

  void demote(BPQNode& it);

  /** @brief  Decrease key by m. Precondition: PQ contains it.
      @param  it  the item   
      @param  m   the change of key
      Note that order of items with same key will be preserved.
      For FM algorithm, this is a prefered behavior. */
  void decreaseKeyBy(BPQNode& it, unsigned int m);

  /** @brief  Increase key by m. Precondition: PQ contains it.
      @param  it  the item   
      @param  m   the change of key
      Note that order of items with same key will be preserved.
      For FM algorithm, this is a prefered behavior. */
  void increaseKeyBy(BPQNode& it, unsigned int m);

  /** @brief  Modify key by m. Precondition: PQ contains it.
      @param  it  the item   
      @param  m   the change of key
      Note that order of items with same key will not be preserved.
      For FM algorithm, this is a prefered behavior. */
  void modifyKeyLIFOBy(BPQNode& it, int m);

  /** @brief  Modify key by m. Precondition: PQ contains it.
      @param  it  the item   
      @param  m   the change of key
      Note that order of items with same key will be preserved.
      For FM algorithm, this is a prefered behavior. */
  void modifyKeyBy(BPQNode& it, int m);

  /** Detach the item from this queue. Precondition: PQ contains it */
  void detach(BPQNode& it);

  /** Push all the items into a given list. This BPQueue becomes empty
      after this operation. */
  void pushTo(GDList<BPQNode>& list);

private:
  // Unimplemented
  GBPQueue(const GBPQueue<BPQNode>&);
  GBPQueue<BPQNode>& operator=(const GBPQueue<BPQNode>&);

private:
  int _offset;          /**< a - 1 */
  unsigned int _high;   /**< b - a + 1 */
  unsigned int _max;    /**< max value */
  GDList<BPQNode>* _T;  /**< bucket, array of lists */
};

/** @} */ // end of group2


template <class BPQNode>
inline GBPQueue<BPQNode>::GBPQueue(int a, int b) :
  _offset(a - 1),
  _high(b - _offset),
  _max(0),
  _T(new GDList<BPQNode>[_high+1])
{ 
  assert(b >= a);
  _T[0].makeDummy(); // sentinel

  /* The propose of_T[0] is to reduce the boundary check. _T[0] acts
     like a non-empty list so that _T[0].isEmpty() always returns
     false. Don't insert items to _T[0]. */
}


template <class BPQNode>
inline void GBPQueue<BPQNode>::setUp(int a, int b)
{ 
  assert(b >= a);

  _offset = a - 1;
  _high = b - _offset;
  _max = 0;
  _T = new GDList<BPQNode>[_high + 1];  
  _T[0].makeDummy(); // sentinel

  /* The propose of_T[0] is to reduce the boundary check. _T[0] acts
     like a non-empty list so that _T[0].isEmpty() always returns
     false. Don't insert items to _T[0]. */
}


template <class BPQNode>
inline bool GBPQueue<BPQNode>::isValid() const
{
  unsigned int cur = _max; 
  while (cur > 0) if (!_T[cur--].isValid()) return false;
  return true;
} 

template <class BPQNode>
inline void GBPQueue<BPQNode>::clear()
{ 
  while (_max > 0) _T[_max--].clear();
} 

template <class BPQNode>
inline void GBPQueue<BPQNode>::pushBack(BPQNode& it, int k) 
{ 
  const unsigned int key = k - _offset;
  assert(!(key<1 || key>_high));
  if (_max < key) _max = key;
  it._key = key;
  _T[key].pushBack(it); 
}

template <class BPQNode>
inline void GBPQueue<BPQNode>::pushBackFast(BPQNode& it, int k) 
{ 
  const unsigned int key = k - _offset;
  assert(!(key<1 || key>_high));
  it._key = key;
  _T[key].pushBack(it); 
}

// template <class BPQNode>
// inline void GBPQueue<BPQNode>::pushRandomFast(BPQNode& it, int k) 
// { 
//   const unsigned int key = k - _offset;
//   assert(!(key<1 || key>_high));
//   it._key = key;
//   _T[key].pushRandom(it); 
// }

template <class BPQNode>
inline void GBPQueue<BPQNode>::resync() 
{ 
  _max = _high; 
  while (_T[_max].isEmpty()) _max--; 
}

template <class BPQNode>
inline BPQNode& GBPQueue<BPQNode>::popFront()
{ 
  assert(_max>0);
  BPQNode& res = _T[_max].popFront();
  while (_T[_max].isEmpty()) _max--; 
  return res;

  /* Without the sentinel, the above while loop needs to check the
     boundary condition, i.e., max > 0:
           while (_max > 0 && _T[_max].isEmpty()) _max--; 
     Other functions below may also need the similar check.
   */
}

template <class BPQNode>
void GBPQueue<BPQNode>::decreaseKeyLIFOBy(BPQNode& it, unsigned int m)
{ 
  assert(it._key > m);
  GDList<BPQNode>::detach(it);
  it._key -= m;
  _T[it._key].pushFront(it); 
  while (_T[_max].isEmpty()) _max--;
}

template <class BPQNode>
void GBPQueue<BPQNode>::increaseKeyLIFOBy(BPQNode& it, unsigned int m)
{ 
  GDList<BPQNode>::detach(it);
  unsigned int& key = it._key;
  key += m;
  assert(key<=_high);
  _T[key].pushFront(it); 
  if (key > _max) _max = key;
}

template <class BPQNode>
void GBPQueue<BPQNode>::promote(BPQNode& it)
{ 
  GDList<BPQNode>::detach(it);
  assert(it._key<=_high);
  _T[it._key].pushFront(it); 
}

template <class BPQNode>
void GBPQueue<BPQNode>::demote(BPQNode& it)
{ 
  GDList<BPQNode>::detach(it);
  assert(it._key<=_high);
  _T[it._key].pushBack(it); 
}

template <class BPQNode>
void GBPQueue<BPQNode>::decreaseKeyBy(BPQNode& it, unsigned int m)
{ 
  assert(it._key > m);
  GDList<BPQNode>::detach(it);
  unsigned int& key = it._key;
  key -= m;
  _T[key].pushBack(it); 
  while (_T[_max].isEmpty()) _max--;
}

template <class BPQNode>
void GBPQueue<BPQNode>::increaseKeyBy(BPQNode& it, unsigned int m)
{ 
  GDList<BPQNode>::detach(it);
  unsigned int& key = it._key;
  key += m;
  assert(key<=_high);
  _T[key].pushBack(it); 
  if (key > _max) _max = key;
}

template <class BPQNode>
inline void GBPQueue<BPQNode>::modifyKeyLIFOBy(BPQNode& it, int m)
{ 
  // Experiments showed that "increase key" is more often than "decrease key"
  if (m > 0) increaseKeyLIFOBy(it, m);
  else if (m < 0) decreaseKeyLIFOBy(it, -m);
}

template <class BPQNode>
inline void GBPQueue<BPQNode>::modifyKeyBy(BPQNode& it, int m)
{ 
  // Experiments showed that "increase key" is more often than "decrease key"
  if (m > 0) increaseKeyBy(it, m);
  else if (m < 0) decreaseKeyBy(it, -m);
}

template <class BPQNode>
inline void GBPQueue<BPQNode>::detach(BPQNode& it) 
{ 
  GDList<BPQNode>::detach(it);
  while (_T[_max].isEmpty()) _max--; 
}

template <class BPQNode>
inline void GBPQueue<BPQNode>::pushTo(GDList<BPQNode>& list) 
{
  while (_max > 0) {
    if (!_T[_max].isEmpty()) {
      list.concat(_T[_max]);
    }
    --_max;
  }
  assert(isEmpty());
}


/** @addtogroup group2
 *  @{
 */

/** 
 * Bounded Priority Queue Cursor. Traverse the queue in descending
 * order. Detaching queue items may invalidate the cursor because 
 * the cursor makes a copy of current key. 
 */
template <class BPQNode>
class GBPQueueCursor
{
private:
  GBPQueue<BPQNode>*    _bpq;       /**< the priority queue */
  GDList<BPQNode>*      _T;         /**< bucket, array of lists */
  unsigned int          _curKey;    /**< the current key value */
  GDListCursor<BPQNode> _lc;        /**< list cursor pointed to 
				       the next item. */

private:
  /** @return the key of the current node (unused currently). */
  unsigned int getKey() const 
  { 
    return _curKey + _bpq->_offset; 
  }

public:
  /** Default Constructor */
  GBPQueueCursor() {}
  
  /** Constructor. */
  GBPQueueCursor(GBPQueue<BPQNode>& BPQ) : 
    _bpq(&BPQ), 
    _T(BPQ._T), 
    _curKey(BPQ._max), 
    _lc(_T[_curKey]) 
  {
  } 

  /** Associate to the queue BPQ and reset the cursor. */
  void associate(GBPQueue<BPQNode>& BPQ) 
  { 
    _bpq = &BPQ; 
    _T = BPQ._T; 
    reset(); 
  }

  /** Reset the cursor to the first position. */
  void reset() 
  { 
    _curKey = _bpq->_max; 
    _lc.associate(_T[_curKey]); 
  }

  /** @return true if there is a next item. */
  bool hasNext() const 
  { 
    return _curKey > 0; 
  }

  /** @return the key of the current node (without offset!). 
      For comparsion purpose only. */
  unsigned int getRelKey() const 
  { 
    return _curKey; 
  }

  /** @return the next item. Precondition: next item exists. */
  BPQNode& getNext()
  {
    assert(hasNext());
    BPQNode& res = _lc.getNext();
    while (!_lc.hasNext()) _lc.associate(_T[--_curKey]);
    return res;
  }
};

/** @} */ // end of group2
#endif

