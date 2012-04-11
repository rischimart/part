#ifndef G_BTREE_HPP
#define G_BTREE_HPP

#ifndef G_DLIST_HPP
#include "GDList.hpp"
#endif

/** @addtogroup group2
 *  @{
 */

/** A simple binary tree (under construction) */
template <class TNode>
class GBTree 
{
protected:
  TNode* _root;   //!< root of tree

private:
  void traverseInfix(TNode& tnode, GDList<TNode>* L);
  void traversePrefix(TNode& tnode, GDList<TNode>* L);
  void traverseSuffix(TNode& tnode, GDList<TNode>* L);

protected:
  /** Perform left rotation */
  void leftRotation(TNode*& t);

  /** Perform right rotation */
  void rightRotation(TNode*& p);

public:
  /** Constructor */
  GBTree() : _root(0) {}

  /** Destructor */
  ~GBTree() {}

  /** @return true if the tree is empty */
  bool isEmpty() const { return _root == 0; };

  /** @return the root */
  TNode& getRoot() { return *_root; }

  /** Set the root */
  void setRoot(TNode& node) { _root = &node; }

  /** Convert this binary tree to a linked list in infix order. This 
      binary tree will be invalid after this operation. Caller need to
      delete the list after use */
  GDList<TNode>* infixOrder();

  /** Convert this binary tree to a linked list in prefix order. This 
      binary tree will be invalid after this operation. Caller need to
      delete the list after use */
  GDList<TNode>* prefixOrder();

  /** Convert this binary tree to a linked list in suffix order. This 
      binary tree will be invalid after this operation. Caller need to
      delete the list after use */
  GDList<TNode>* suffixOrder();

  /** Delete all tree node */
  void clear();
};

#include "GBTree.cpp"

/** @} */ // end of group2
#endif
