#ifndef G_MULTISET_HPP
#define G_MULTISET_HPP

#ifndef G_NODE_HPP
#include "GNode.hpp"
#endif
#ifndef G_BTREE_HPP
#include "GBTree.hpp"
#endif
#ifndef G_DLIST_HPP
#include "GDList.hpp"
#endif

/** Forward declaration */
template <class T> class GDList;
template <class T> class GDListCursor;
template <class T> class GDListConstCursor;
template <class T> class GMultiSet;

/** @addtogroup group2 
 *  @{
 */

/** 
 * Tree Node (under contruction). For efficiency, the objects of this class can be
 * attached to GMultiSet, GBTree and GDList. 
 */
template <class T>
class GTreeNode
{
  friend class GMultiSet<T>;
  friend class GBTree<GTreeNode<T> >;
  friend class GDList<GTreeNode<T> >;
  friend class GDListCursor<GTreeNode<T> >;
  friend class GDListConstCursor<GTreeNode<T> >;

private:
  GTreeNode<T>*             _next;   ///< right
  GTreeNode<T>*             _prev;   ///< left
  T                         _key;
  GDList<GNode>             _list;

public:
  /** Constructor */
  GTreeNode() : _next(0), _prev(0) {}
};


/** Multiset item (under construction) */
typedef GNode GMultiSetItem;

/** A MutliSet (under construction). */
template <class T>
class GMultiSet : private GBTree<GTreeNode<T> >
{
  typedef GBTree<GTreeNode<T> > _Base;

public:
  /** Tree item */
  typedef GTreeNode<T>  GTreeItem;

  /** Constructor */
  GMultiSet() {}

  /** Destructor */
  ~GMultiSet() {}

  /** @return the key of it */
  T getKey(const GMultiSetItem& it) const { return it._key; }

  /** @return true if the tree is empty */
  bool isEmpty() const { return GBTree<GTreeItem>::isEmpty(); }

  /** Reset the MultiSet */
  void clear() { GBTree<GTreeItem>::clear(); }

  /** Insert an element */
  void insert(GMultiSetItem& elem, T k);

  /** Insert an element in LIFO fashion */
  void insertFront(GMultiSetItem& elem, T k);

  /** Convert this set to a sorted list in decending order. This 
      set will be invalid after this operation. Caller need to
      delete the list after use */
  GDList<GMultiSetItem>* decendingOrder();

  /** Convert this set to a sorted list in ascending order. This 
      set will be invalid after this operation. Caller need to
      delete the list after use */
  GDList<GMultiSetItem>* ascendingOrder();
};

/** @} */

/* The following codes should be put in a .cpp file. However, keyword
   'export' is not supported by most compilers. 
 */
template <class T>
void GMultiSet<T>::insertFront(GMultiSetItem& elem, T k)
{
  if (_Base::_root == 0) {
    _Base::_root = new GTreeItem;
    _Base::_root->_key = k;
    _Base::_root->_list.pushFront(elem);
  }

  GTreeItem& cur = _Base::_root;
  while (1) {
    if (cur._key > k) {
      if (cur._prev != 0) {
        cur = *cur._prev;
        continue;
      }
      else {
        cur._prev = new GTreeItem;
        cur._prev->_key = k;
        cur._prev->_list.pushFront(elem);
        break;
      }
    }
    else if (cur._key < k) {
      if (cur._next != 0) {
        cur = *cur._next;
        continue;
      }
      else {
        cur._next = new GTreeItem;
        cur._next->_key = k;
        cur._next->_list.pushFront(elem);
        break;
      }
    }
    else {
      cur._list.pushFront(elem);
      break;
    }
  }
  // re-balancing?
}

template <class T>
void GMultiSet<T>::insert(GMultiSetItem& elem, T k)
{
  if (_Base::_root == 0) {
    _Base::_root = new GTreeItem;
    _Base::_root->_key = k;
    _Base::_root->_list.pushBack(elem);
  }

  GTreeItem* cur = _Base::_root;
  while (1) {
    if (cur->_key > k) {
      if (cur->_prev != 0) {
        cur = cur->_prev;
        continue;
      }
      else {
        cur->_prev = new GTreeItem;
        cur->_prev->_key = k;
        cur->_prev->_list.pushBack(elem);
        break;
      }
    }
    else if (cur->_key < k) {
      if (cur->_next != 0) {
        cur = cur->_next;
        continue;
      }
      else {
        cur->_next = new GTreeItem;
        cur->_next->_key = k;
        cur->_next->_list.pushBack(elem);
        break;
      }
    }
    else {
      cur->_list.pushBack(elem);
      break;
    }
  }
  // re-balancing?
}

template <class T>
GDList<GMultiSetItem>* GMultiSet<T>::decendingOrder()
{ 
  GDList<GTreeItem>* list = GBTree<GTreeItem>::infixOrder();
  GDListCursor<GTreeItem> lc(list);
  GDList<GMultiSetItem>* res = new GDList<GMultiSetItem>;
  while (lc.hasPrev()) {
    GTreeItem& ti = lc.getPrev();
    res->concat(ti._list);
    delete &ti;
  }
  delete list;
  return res;
}

template <class T>
GDList<GMultiSetItem>* GMultiSet<T>::ascendingOrder()
{ 
  GDList<GTreeItem>* list = GBTree<GTreeItem>::infixOrder();
  GDListCursor<GTreeItem> lc(*list);
  GDList<GMultiSetItem>* res = new GDList<GMultiSetItem>;
  while (lc.hasNext()) {
    GTreeItem& ti = const_cast<GTreeItem&>(lc.getNext());
    res->concat(ti._list);
    delete &ti;
  }
  delete list;
  return res;
}

#endif
