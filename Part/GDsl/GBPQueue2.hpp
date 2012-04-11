#ifndef G_BPQUEUE2_HPP
#define G_BPQUEUE2_HPP
#include <cassert>  //added by Zhou

#ifndef G_DLIST_HPP
#include "GDList.hpp"
#endif
#ifndef G_BPQUEUE_HPP
#include "GBPQueue.hpp"
#endif

/** @addtogroup group2
 *  @{
 */
 
/* Forward declaration */
template <class BPQNode> class GBPQueue2Cursor;
template <class BPQNode> class GBPQueue2ConstCursor;


/**
 * (Experimental) Bounded Priority Queue with secondary key. 
 */
template <class BPQNode>
class GBPQueue2
{
  friend class GBPQueue2Cursor<BPQNode>;
  friend class GBPQueue2ConstCursor<BPQNode>;

private:
  unsigned int _high;   /**< b - a + 1 */
  unsigned int _max;    /**< max value */
  int _offset;          /**< a - 1 */
  GBPQueue<BPQNode>* _T;  /**< bucket, array of buckets */

  // Unimplemented
  GBPQueue2(const GBPQueue2<BPQNode>&);
  GBPQueue2<BPQNode>& operator=(const GBPQueue2<BPQNode>&);

public:
  GBPQueue2() : _high(0), _max(0), _offset(0), _T(0) {}

  /** 
   * Constructor.
   *   @param   a   lower bound
   *   @param   b   upper bound
   *   @param   a2  lower bound of second key
   *   @param   b2  upper bound of second key
   * Precondition: a <= b 
   */
  GBPQueue2(int a, int b, int a2, int b2) { setUp(a, b, a2, b2); }

  /** Destructor. */
  ~GBPQueue2() { if (_T != 0) delete [] _T; }

  /** Setup */
  void setUp(int a, int b, int a2, int b2);

  /** @return the key of it. */
  int getKey(const BPQNode& it) const { return it._key2 + _offset; }

  /** @return the max value. */
  int getMax() const { return _max + _offset; }

  /** @return true if the PQ is empty. */
  bool isEmpty() const { return _max < 1; }

  /** Make dummy */
  void makeDummy() { _max = 1; }

  /** Reset the PQ. */
  void clear();

  /** Insert item at the end of this queue. 
      Precondition: PQ does not contain it; a <= k <= b. */
  void pushBack(BPQNode& it, int k, int k2);

  /** Insert item at the front of this queue.
      Precondition: PQ does not contain it; a <= k <= b. */
  void pushFront(BPQNode& it, int k, int k2);

  /** Arbitrarily put item at the front or at the end of this queue
      without update the max value. 
      Precondition: PQ does not contain it; a <= k <= b. */
  void pushRandom(BPQNode& it, int k, int k2);

  /** Insert item at the end of this queue without update the max
      value. Call resync() or resyncFast() after calling this method.
      Precondition: PQ does not contain it; a <= k <= b. */
  void pushBackFast(BPQNode& it, int k, int k2);

  /** Insert item at the front of this queue without update the max
      value. Call resync() or resyncFast() after calling this method.
      Precondition: PQ does not contain it; a <= k <= b. */
  void pushFrontFast(BPQNode& it, int k, int k2);

  /** Arbitrarily put item at the front or at the end of this queue
      without update the max
      value. Call resync() or resyncFast() after calling this method.
      Precondition: PQ does not contain it; a <= k <= b. */
  void pushRandomFast(BPQNode& it, int k, int k2);

  /** Resync the max value */
  void resync();

  /** Pop an item with highest key.
      Precondition: PQ is not empty.*/
  BPQNode& popFront();

  /** Pop an item with highest key without returning the item.
      Precondition: PQ is not empty.*/
  void pop();

  /** Peek the top item with highest key.
      Precondtion: PQ is not empty.*/
  BPQNode& top();

  /** Decrease the key of item to k in Last-In-First-Out fashion.
      Precondition: PQ contains it; a <= k <= b.
      Note that order of items with same key will not preserved.
      For FM algorithm, this is a prefered behavior. */
  void decreaseKeyLIFO(BPQNode& it, int k, int k2);

  /** Increase the key of item to k in Last-In-First-Out fashion.
      Precondition: PQ contains it.
      Note that order of items with same key will not preserved.
      For FM algorithm, this is a prefered behavior. */
  void increaseKeyLIFO(BPQNode& it, int k, int k2);
  
  /** Modify the key of item to k in Last-In-First-Out fashion.
      Precondition: PQ contains it.
      Note that order of items with same key will not preserved.
      For FM algorithm, this is a prefered behavior. */
  void modifyKeyLIFO(BPQNode& it, int k, int k2);
  
  /** Decrease key by one. Precondition: PQ contains it; a <= k <= b */
  void decreaseKey(BPQNode& it);
  
  /** Increase key by one. Precondition: PQ contains it; a <= k <= b */
  void increaseKey(BPQNode& it); 
  
  /** Decrease key by m. Precondition: PQ contains it.
      Note that order of items with same key will not preserved.
      For FM algorithm, this is a prefered behavior. */
  void decreaseKeyLIFOBy(BPQNode& it, unsigned int m, int m2);

  /** Increase key by m. Precondition: PQ contains it.
      Note that order of items with same key will not preserved.
      For FM algorithm, this is a prefered behavior. */
  void increaseKeyLIFOBy(BPQNode& it, unsigned int m, int m2);

  /** Modify key by m.
      Note that order of items with same key will not preserved.
      For FM algorithm, this is a prefered behavior. */
  void modifyKeyLIFOBy(BPQNode& it, int m, int m2);

  /** Detach the item from this queue. Precondition: PQ contains it */
  void detach(BPQNode& it);

  /** Push all the items into a given list. This BPQueue becomes empty
      after this operation. */
  void pushTo(GDList<BPQNode>& list);
};

/** @} */ // end of group2


template <class BPQNode>
inline void GBPQueue2<BPQNode>::setUp(int a, int b, int a2, int b2)
{ 
  assert(b >= a);

  _T = new GBPQueue<BPQNode>[b - a + 2];  
  _offset = a - 1;
  _high = b - a + 1;
  _max = 0;

  for (unsigned int i=0; i<=_high; i++) {
    _T[i].setUp(a2, b2);
  }

  _T[0].makeDummy(); // sentinel

  /* The propose of_T[0] is to reduce the boundary check. _T[0] acts
     like a non-empty list so that _T[0].isEmpty() always returns
     false. Don't insert items to _T[0]. */
}

template <class BPQNode>
inline void GBPQueue2<BPQNode>::clear()
{ 
  for (unsigned int i=1; i<=_max; i++) _T[i].clear();
  _max = 0;
} 

template <class BPQNode>
inline void GBPQueue2<BPQNode>::pushBackFast(BPQNode& it, int k, int k2) 
{ 
  const unsigned int key2 = k - _offset;
  assert(!(key2<1 || key2>_high));
  it._key2 = key2;
  _T[key2].pushBack(it, k2); 
}

template <class BPQNode>
inline void GBPQueue2<BPQNode>::pushFrontFast(BPQNode& it, int k, int k2) 
{ 
  const unsigned int key2 = k - _offset;
  assert(!(key2<1 || key2>_high));
  it._key2 = key2;
  _T[key2].pushFront(it, k2); 
}

template <class BPQNode>
inline void GBPQueue2<BPQNode>::pushRandomFast(BPQNode& it, int k, int k2) 
{ 
  const unsigned int key2 = k - _offset;
  assert(!(key2<1 || key2>_high));
  it._key2 = key2;
  _T[key2].pushRandom(it, k2); 
}

template <class BPQNode>
inline void GBPQueue2<BPQNode>::resync() 
{ 
  _max = _high; 
  while (_T[_max].isEmpty()) _max--; 
}

template <class BPQNode>
inline void GBPQueue2<BPQNode>::pushBack(BPQNode& it, int k, int k2) 
{ 
  const unsigned int key2 = k - _offset;
  assert(!(key2<1 || key2>_high));
  if (_max < key2) _max = key2;
  it._key2 = key2;
  _T[key2].pushBack(it, k2); 
}

template <class BPQNode>
inline void GBPQueue2<BPQNode>::pushFront(BPQNode& it, int k, int k2) 
{ 
  const unsigned int key2 = k - _offset;
  assert(!(key2<1 || key2>_high));
  if (_max < key2) _max = key2;
  it._key2 = key2;
  _T[key2].pushFront(it, k2); 
}

template <class BPQNode>
inline void GBPQueue2<BPQNode>::pushRandom(BPQNode& it, int k, int k2) 
{ 
  const unsigned int key2 = k - _offset;
  assert(!(key2<1 || key2>_high));
  if (key2 > _max) _max = key2;
  it._key2 = key2;
  _T[key2].pushRandom(it, k2); 
}

template <class BPQNode>
inline BPQNode& GBPQueue2<BPQNode>::popFront()
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
inline void GBPQueue2<BPQNode>::pop()
{ 
  assert(_max>0);
  _T[_max].popFront();
  while (_T[_max].isEmpty()) _max--; 
}

template <class BPQNode>
inline BPQNode& GBPQueue2<BPQNode>::top()
{ 
  assert(_max>0);
  return _T[_max].top();
}

template <class BPQNode>
inline void GBPQueue2<BPQNode>::decreaseKeyLIFO(BPQNode& it, int k, int k2)
{ 
  const unsigned int key2 = k - _offset;
  assert(!(key2<1 || key2>_high));
  assert(it._key2 >= key2);
  _T[it._key2].detach(it);
  it._key2 = key2;
  _T[key2].pushFront(it, k2); 
  while (_T[_max].isEmpty()) _max--; 
}

template <class BPQNode>
inline void GBPQueue2<BPQNode>::increaseKeyLIFO(BPQNode& it, int k, int k2)
{ 
  const unsigned int key2 = k - _offset;
  assert(!(key2<1 || key2>_high));
  assert(it._key2 <= key2);
  _T[it._key2].detach(it);
  it._key2 = key2;
  _T[key2].pushFront(it, k2); 
  if (_max < key2) _max = key2;
}

template <class BPQNode>
inline void GBPQueue2<BPQNode>::modifyKeyLIFO(BPQNode& it, int k, int k2)
{ 
  int m = getKey(it);
  if (m < k) increaseKeyLIFO(it, k, k2);
  else if (m > k) decreaseKeyLIFO(it, k, k2);
  else _T[it._key2].modifyKeyLIFO(it, k2);
}

template <class BPQNode>
inline void GBPQueue2<BPQNode>::decreaseKey(BPQNode& it)
{ 
  assert(it._key2 > 1);
  _T[it._key2--].detach(it);
  _T[it._key2].pushBack(it); 
  if (_T[_max].isEmpty()) _max--;
}

template <class BPQNode>
inline void GBPQueue2<BPQNode>::increaseKey(BPQNode& it)
{ 
  assert(it._key2<_high);
  _T[it._key2++].detach(it);
  _T[it._key2].pushBack(it); 
  if (_max < it._key2) _max = it._key2;
}

template <class BPQNode>
void GBPQueue2<BPQNode>::
decreaseKeyLIFOBy(BPQNode& it, unsigned int m, int m2)
{ 
  _T[it._key2].detach(it);
  it._key2 -= m;
  assert(it._key2 >= 1);
  it._key += m2;
  _T[it._key2].pushFront(it); 
  while (_T[_max].isEmpty()) _max--;
}

template <class BPQNode>
void GBPQueue2<BPQNode>::
increaseKeyLIFOBy(BPQNode& it, unsigned int m, int m2)
{ 
  _T[it._key2].detach(it);
  it._key2 += m;
  assert(it._key2<=_high);
  it._key += m2;
  _T[it._key2].pushFront(it); 
  if (it._key2 > _max) _max = it._key2;
}

template <class BPQNode>
inline void GBPQueue2<BPQNode>::
modifyKeyLIFOBy(BPQNode& it, int m, int m2)
{ 
  // Experiments showed that "increase key" is more often than "decrease key"
  if (m > 0) increaseKeyLIFOBy(it, m, m2);
  else if (m < 0) decreaseKeyLIFOBy(it, -m, m2);
  else _T[it._key2].modifyKeyLIFOBy(it, m2);
}

template <class BPQNode>
inline void GBPQueue2<BPQNode>::detach(BPQNode& it) 
{ 
  _T[it._key2].detach(it); 
  while (_T[_max].isEmpty()) _max--; 
}

template <class BPQNode>
inline void GBPQueue2<BPQNode>::pushTo(GDList<BPQNode>& list) 
{
  while (_max > 0) {
    if (!_T[_max].isEmpty()) _T[_max].pushTo(list);
    --_max;
  }
  assert(isEmpty());
}


/** @addtogroup group2
 *  @{
 */

/** 
 * Bounded Priority Queue2 Cursor. Traverse the queue in descending
 * order. Detaching queue items may invalidate the cursor because 
 * the cursor make a copy of current key. 
 */
template <class BPQNode>
class GBPQueue2Cursor
{
private:
  GBPQueue2<BPQNode>* _bpq;
  unsigned int _curKey;
  GBPQueueCursor<BPQNode> _bpqc;

public:
  /** Default Constructor */
  GBPQueue2Cursor() {}
  
  /** Constructor. */
  GBPQueue2Cursor(GBPQueue2<BPQNode>& BPQ) 
  : _bpq(&BPQ), _curKey(BPQ._max), _bpqc(BPQ._T[_curKey]) {} 

  /** Associate to the queue BPQ and reset the cursor. */
  void associate(GBPQueue2<BPQNode>& BPQ) { _bpq = &BPQ; reset(); }

  /** @return true if there is a next item. */
  bool hasNext() const { return _curKey > 0; }

  /** Reset the cursor to the first position. */
  void reset() { _curKey = _bpq->_max; _bpqc.associate(_bpq->_T[_curKey]); }

  /** @return the key of the next node (without offset!) */
  unsigned int getKey() const { return _curKey; }

  /** @return the key of the next node (without offset!) */
  unsigned int getSecondKey() const { return _bpqc->getKey(); }

  /** @return the next item. Precondition: next item exists. */
  BPQNode& getNext()
  {
    assert(hasNext());
    BPQNode& res = _bpqc.getNext();
    while (!_bpqc.hasNext()) _bpqc.associate(_bpq->_T[--_curKey]);
    return res;
  }
};


/** 
 * Bounded Priority Queue Cursor. Traverse the queue in descending
 * order. It is NOT safe to detach queue items during
 * the cursor is active because the cursor make a copy of current key. 
 */
template <class BPQNode>
class GBPQueue2ConstCursor
{
private:
  const GBPQueue2<BPQNode>* _bpq;
  unsigned int _curKey;
  GBPQueueConstCursor<BPQNode> _bpqc;

public:
  /** Default Constructor */
  GBPQueue2ConstCursor() {}

  /** Constructor. */
  GBPQueue2ConstCursor(const GBPQueue2<BPQNode>& BPQ) 
  : _bpq(&BPQ), _curKey(BPQ._max), _bpqc(BPQ._T[_curKey]) {} 

  /** Associate to the queue BPQ and reset the cursor. */
  void associate(const GBPQueue2<BPQNode>& BPQ) { _bpq = BPQ; reset(); }

  /** @return true if there is a next item. */
  bool hasNext() const { return _curKey > 0; }

  /** Reset the cursor to the first position. */
  void reset() { _curKey = _bpq->_max; _bpqc.associate(_bpq->_T[_curKey]); }

  /** @return the key of the next node (without offset!) */
  unsigned int getKey() const { return _curKey; }

  /** @return the second key of the next node (without offset!) */
  unsigned int getSecondKey() const { return _bpqc->getKey(); }

  /** @return the next item. Precondition: next item exists. */
  const BPQNode& getNext()
  {
    assert(hasNext());
    const BPQNode& res = _bpqc.getNext();
    while (!_bpqc.hasNext()) _bpqc.associate(_bpq->_T[--_curKey]);
    return res;
  }
};

/** @} */ // end of group2
#endif

