#ifndef G_STACK_H
#define G_STACK_H

#include <cassert>

/** Forward declaration */
template <class SNode> class GStackCursor;

/** @addtogroup group2
 *  @{
 */

/**
 * Stack implemented by singly-linked list. This class simply contains
 * a link of nodes and does not keep the length information as it is not
 * necessary for the FM algorithm (however, it may be useful for
 * debugging). This saves memory and run-time to update the length
 * information. Note that this class does not own the list node. They
 * are supplied by the caller in order to better reuse the nodes.
 *
 * Todo: Stack should be an adaptor of singly linked list.
 */
template <class SNode>
class GStack
{
protected:
  SNode* _head;     ///< head of stack

public:
  /** Constructor */
  GStack() : _head(0) {}

  /** @return ture if stack is empty */
  bool isEmpty() const { return _head == 0; }

  /** Reset to empty stack */
  void clear() { _head = 0; }

  /** @return the top node. */
  SNode* top() { return _head; }

  /** @return the top node. */
  const SNode* top() const { return _head; }

  /** Push s to the stack. Precondition: node is used in ANY lists. */
  void push(SNode* s) { pushFront(s); }

  /** Pop the top node from the stack. Precondition: stack is not empty. */
  SNode* pop() { return popFront(); }

  /** Push s to the stack. Precondition: node is used in ANY lists. */
  void pushFront(SNode* s);

  /** Pop the top node from the stack. Precondition: stack is not empty. */
  SNode* popFront();

  /** Check if the stack loops back to the previous nodes. Take O(n) time. */
  bool isValid() const;
};


template <class SNode>
inline SNode* GStack<SNode>::popFront() 
{
  assert(!isEmpty());
  SNode* const r = _head;
  _head = _head->_next;
  return r;
}

template <class SNode>
inline void GStack<SNode>::pushFront(SNode* s) 
{
  s->_next = _head;
  _head = s;
}


/**
 * @return true if stack contains cycle on its path. The time
 * complexity is O(n) where n is the length of the path. The
 * storage requirement is O(1). If s is empty, return true.
 *
 * The trick is to emit two pointers n1 and n2. Each time
 * n2 goes two steps ahead whereas n1 goes one step ahead. If two
 * pointers meet again, then the path is cyclic (and hence invalid).
 */
template <class SNode>
inline bool GStack<SNode>::isValid() const
{
  const SNode* n1 = _head;
  const SNode* n2 = _head;

  while (n2 != 0) {
    n2 = n2->_next;
    if (n2 == 0) return true;
    n1 = n1->_next;
    n2 = n2->_next;
    if (n1 == n2) return false;
  }
 
  return true;
}

/** @} */




/** @addtogroup group2
 *  @{
 */

/** Stack Cursor. Traverse the stack from the first item. Usually it is
    safe to push stack items during the cursor is active,
    assuming that the detached items are not used in other places or
    are not deallocated.  */
template <class SNode>
class GStackCursor
{
private:
  GStack<SNode>* _stack;
  SNode* _cursor;

public:
  /** Constructor */
  GStackCursor() : _stack(0), _cursor(0) {}

  /** Constructor */
  GStackCursor(GStack<SNode>& S) : _stack(&S) { reset(); } 

  /** Associate to the stack S and reset the cursor. */
  void associate(GStack<SNode>& S) { _stack = &S; reset(); }

  /** @return true if there is a next item. */
  bool hasNext() const { return _cursor != 0; }

  /** Reset the cursor to the first position. */
  void reset() { _cursor = _stack->top(); }
 
  /** @return the next item. Precondition: next item exist. */
  SNode* getNext() 
  { 
    assert(hasNext()); 
    _cursor = _cursor->_next; 
    return _cursor; 
  }
};

/** Stack Cursor. Traverse the stack from the first item. Usually it is
    safe to push stack items during the cursor is active,
    assuming that the detached items are not used in other places or
    are not deallocated.  */
template <class SNode>
class GStackConstCursor
{
private:
  const GStack<SNode>* _stack;
  const SNode* _cursor;

public:
  /** Constructor */
  GStackConstCursor() : _stack(0), _cursor(0) {}

  /** Constructor */
  GStackConstCursor(const GStack<SNode>& S) : _stack(&S) { reset(); } 

  /** Associate to the stack S and reset the cursor. */
  void associate(const GStack<SNode>* &S) { _stack = &S; reset(); }

  /** @return true if there is a next item. */
  bool hasNext() const { return _cursor != 0; }

  /** Reset the cursor to the first position. */
  void reset() { _cursor = _stack->top(); }
 
  /** @return the next item. Precondition: next item exist. */
  const SNode* getNext() 
  { 
    assert(hasNext()); 
    _cursor = _cursor->_next; 
    return _cursor; 
  }
};

/** @} */ // end of group2
#endif
