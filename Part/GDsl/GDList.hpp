#ifndef G_DLIST_HPP
#define G_DLIST_HPP

#include <cassert>
#include <cstdlib>    // for rand()

/** @addtogroup group2
 *  @{
 */

/* Forward declaration */
template <class DNode> class GDListCursor;
template <class DNode> class GDListConstCursor;
template <class DNode> class GBPQueue;
template <class DNode> class GBPQueue2;

/**
 * A Doubly-linked List class. This class simply contains a link of
 * node's. By adding a dummy node (sentinel) nil, deleting a node is
 * extremely fast (see "Introduction to Algorithm"). This class does
 * not keep the length information as it is not necessary for the FM
 * algorithm. This saves memory and run-time to update the length
 * information. Note that this class does not own the list node. They
 * are supplied by the caller in order to better reuse the nodes.
 * 
 * From "Introduction to Algorithm":
 *
 * A sentinel is a dummy object that allows us to simplify boundary
 * conditions. For example, suppose that we provide with list L an object
 * nil[L] that represents NIL but has all the fields of the other list
 * elements. Wherever we have a reference to NIL in list code, we replace
 * it by a reference to the sentinel nil[L]. As shown in Figure 11.4,
 * this turns a regular doubly linked list into a circular list, with the
 * sentinel nil[L] placed between the head and tail; the field
 * next[nil[L]] points to the head of the list, and prev[nil[L]] points
 * to the tail. Similarly, both the next field of the tail and the prev
 * field of the head point to nil[L]. Since next[nil[L]] points to the
 * head, we can eliminate the attribute head[L] altogether, replacing
 * references to it by references to next[nil[L]]. An empty list consists
 * of just the sentinel, since both next[nil[L]] and prev[nil[L]] can be
 * set to nil[L]. 
 *
 */
template <class DNode>
class GDList
{
  friend class GDListCursor<DNode>;
  friend class GDListConstCursor<DNode>;
  friend class GBPQueue<DNode>;
  friend class GBPQueue2<DNode>;

public:
  /** Constructor */
  GDList() : _head(nil()), _tail(_head) {}
 
  /** @return true if list is empty. */
  bool isEmpty() const { return _head == nil(); }

  /** Reset to empty list. */
  void clear() { _head = _tail = nil(); }

  /** @return the first item. Precondition: list is not empty */
  DNode& first() { assert(!isEmpty()); return *_head; }

  /** @return the last item. Precondition: list is not empty */
  DNode& last() { assert(!isEmpty()); return *_tail; }

  /** @return the first item. Precondition: list is not empty */
  const DNode& first() const { assert(!isEmpty()); return *_head; }

  /** @return the last item. Precondition: list is not empty */
  const DNode& last() const { assert(!isEmpty()); return *_tail; }

  /** Detach item from this list. Precondition: list contains the node */
  static void detach(DNode& ptr)
  {
    DNode* const n = ptr._next;
    DNode* const p = ptr._prev;
    assert(n != 0 && n->_prev == &ptr);
    assert(p != 0 && p->_next == &ptr);
    p->_next = n;
    n->_prev = p;
  }

  /** Insert item at the front of this list.
      Precondition: list does not contain the node */
  void pushFront(DNode& ptr) 
  {
    ptr._next = _head;
    _head->_prev = &ptr;
    _head = &ptr;
    ptr._prev = nil();
  }  

  /** Insert item at the end of this list 
      Precondtion: list does not contain the node */
  void pushBack(DNode& ptr) 
  {
    ptr._prev = _tail;
    _tail->_next = &ptr;
    _tail = &ptr;
    ptr._next = nil();
  }  

  /** Arbitrarily put item at the front or at the end of this list
      Precondition: list does not contain the node */
  void pushRandom(DNode& ptr)
  {
    if (rand() & 1 == 0) pushBack(ptr);
    else pushFront(ptr);
  }

  /** Pop an item from the front of the list. 
      Precondition: list is not empty */
  DNode& popFront()
  {
    assert(!isEmpty());
    DNode* res = _head;
    _head = res->_next;
    _head->_prev = nil();
    return *res;        
  }

  /** Pop an item from the end of the list. 
      Precondition: list is not empty */
  DNode& popBack()
  {
    assert(!isEmpty());
    DNode* res = _tail;
    _tail = res->_prev;
    _tail->_next = nil();
    return *res;        
  }

  /** Pop an item from the front of the list. 
      Precondition: list is not empty */
  void popFrontFast()
  {
    assert(!isEmpty());
    _head = _head->_next;
    _head->_prev = nil(); 
  }

  /** Pop an item from the end of the list. 
      Precondition: list is not empty */
  void popBackFast()
  {
    assert(!isEmpty());
    _tail = _tail->_prev;
    _tail->_next = nil();       
  }

  /** Concatenate L to the end of this list. L becomes an empty list
      after the concatenation. */
  void concat(GDList<DNode>& L)
  {
    if (L.isEmpty()) return;
    _tail->_next = L._head;
    L.first()._prev = _tail;
    L.last()._next = nil();
    _tail = L._tail;
    L.clear();
  }

//xxx   /** @return true if list contains ptr. Take O(n) time. */
//xxx   bool contains(DNode& ptr) const
//xxx   {
//xxx     const DNode* cur = nil();
//xxx     while ((cur = cur->_next) != nil()) {
//xxx       if (&ptr == cur) return true;
//xxx     }
//xxx     return false;
//xxx   }

  /** @return true if the list is valid. Take O(n) time */
  bool isValid() const
  {
    const DNode* cur = nil();
    do {
      if (cur->_next->_prev != cur) return false;
      if (cur->_prev->_next != cur) return false;
      cur = cur->_next;
    } while (cur != nil());
    return true;
  }

private:
  ///@{
  DNode* nil() { return reinterpret_cast<DNode*>(this); }
  const DNode* nil() const { return reinterpret_cast<const DNode*>(this); }
  ///@}

  /** (for restricted use only) Make the list act like a non-empty list 
      (Function isEmpty() always returns false). */
  void makeDummy() { _head = 0; }

  // Unimplemented (no way to implement these)
  GDList(const GDList<DNode>&);
  GDList<DNode>& operator=(const GDList<DNode>&);

private:
  DNode* _head;  /**< pointer of the head, also equal to _nil.next */
  DNode* _tail;  /**< pointer of the tail, also equal to _nil.prev */
};

/** @} */ // end of group2


/** @addtogroup group2
 *  @{
 */

/** 
 * List Cursor. Traverse the list from the first item. Usually it is
 * safe to attach/detach list items during the cursor is active.
 */
template <class DNode>
class GDListCursor
{
private:
  DNode* _nil;   /**< pointer to the sentinel node */
  DNode* _cur;   /**< pointer to the next item */

public:
  /** Default Constructor */
  GDListCursor() {}

  /** Constructor */
  GDListCursor(GDList<DNode>& L) : 
    _nil(L.nil()), 
    _cur(_nil->_next) 
  {
  }

  /** Associate to the list L and reset the cursor. */
  void associate(GDList<DNode>& L) { _nil = L.nil(); reset(); }

  /** @return true if there is a next item. */
  bool hasNext() const { return _cur != _nil; }

  /** Reset the cursor to the first position. */
  void reset() { _cur = _nil->_next; }
 
  /** @return the next item. Precondition: next item exist */
  DNode& getNext() 
  { 
    assert(hasNext());
    DNode& res = *_cur;
    _cur = _cur->_next; 
    return res; 
  }
};


/** 
 * List Cursor. Traverse the list from the first item. Usually it is
 * safe to attach/detach list items during the cursor is active.
 */
template <class DNode>
class GDListConstCursor
{
private:
  const DNode* _nil;   /**< pointer to the sentinel node */
  const DNode* _cur;   /**< pointer to the next item */

public:
  /** Default Constructor */
  GDListConstCursor() {}

  /** Constructor */
  GDListConstCursor(const GDList<DNode>& L) : 
    _nil(L.nil()), 
    _cur(_nil->_next) 
  {
  }

  /** Associate to the list L and reset the cursor. */
  void associate(const GDList<DNode>& L) { _nil = L.nil(); reset(); }

  /** @return true if there is a next item. */
  bool hasNext() const { return _cur != _nil; }

  /** Reset the cursor to the first position. */
  void reset() { _cur = _nil->_next; }
 
  /** @return the next item. Precondition: next item exist */
  const DNode& getNext()
  { 
    assert(hasNext()); 
    const DNode& res = *_cur;
    _cur=_cur->_next; 
    return res; 
  }
};

/** @} */ // end of group2
#endif

