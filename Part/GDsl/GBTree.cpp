#include <cassert>

template <class TNode>
GDList<TNode>* GBTree<TNode>::infixOrder()
{
  GDList<TNode>* L = new GDList<TNode>;
  traverseInfix(*_root, L);
  _root = 0;
  return L;
}

// Recusive version. Too slow?
template <class TNode>
void GBTree<TNode>::traverseInfix(TNode& node, GDList<TNode>* L)
{
  GDList<TNode> leftList, rightList;
  if (node._prev != 0) traverseInfix(*node._prev, &leftList);
  if (node._next != 0) traverseInfix(*node._next, &rightList);
  L->concat(leftList);
  L->pushBack(node);
  L->concat(rightList);
}

template <class TNode>
GDList<TNode>* GBTree<TNode>::prefixOrder()
{
  GDList<TNode>* L = new GDList<TNode>;
  traversePrefix(*_root, L);
  _root = 0;
  return L;
}

// Recusive version. Too slow?
template <class TNode>
void GBTree<TNode>::traversePrefix(TNode& node, GDList<TNode>* L)
{
  GDList<TNode> leftList, rightList;
  if (node._prev != 0) traversePrefix(*node._prev, &leftList);
  if (node._next != 0) traversePrefix(*node._next, &rightList);
  L->pushBack(node);
  L->concat(leftList);
  L->concat(rightList);
}

template <class TNode>
GDList<TNode>* GBTree<TNode>::suffixOrder()
{
  GDList<TNode>* L = new GDList<TNode>;
  traverseSuffix(*_root, L);
  _root = 0;
  return L;
}

// Recusive version. Too slow?
template <class TNode>
void GBTree<TNode>::traverseSuffix(TNode& node, GDList<TNode>* L)
{
  GDList<TNode> leftList, rightList;
  if (node._prev != 0) traverseSuffix(*node._prev, &leftList);
  if (node._next != 0) traverseSuffix(*node._next, &rightList);
  L->concat(leftList);
  L->concat(rightList);
  L->pushBack(node);
}

template <class TNode>
void GBTree<TNode>::clear()
{
  //xxx if (_root != 0) { clearRecur(*_root); _root = 0; } 
}


/**
 *              p                                       t          
 *            /   \    	      right rotation 	      /   \      
 *           t     c   	      -------------> 	     a     p     
 *         /   \       	      <------------  	         /   \   
 *        a     b      	      left rotation  	        b     c  
 *
 */

/** Precondition: right child of p is not NULL. Take O(1) time. */
template <class TNode>
void GBTree<TNode>::leftRotation(TNode*& t)
{
  assert(t->_next != 0);

  TNode* p = t->_next;
  t->_next = p->_prev;
  p->_prev = t;
  t = p;
}

/* Precondition: left child of p is not NULL. Take O(1) time */
template <class TNode>
void GBTree<TNode>::rightRotation(TNode*& p)
{
  assert(p->_prev != 0);

  TNode* t = p->_prev;
  p->_prev = t->_next;
  t->_next = p;
  p = t;
}

