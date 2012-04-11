#ifndef G_NODE_HPP
#define G_NODE_HPP

/** @addtogroup group2
 *  @{
 */

/** Generic Node for GDList */ 
class GDNode
{
public:
  GDNode* _next;  /**< point to the next node */
  GDNode* _prev;  /**< point to the prev node */
};


/** Generic Node. For efficiency, the objects of this class can be
    attached to GBPQueue (bounded priority queue) and GDList (list). 
    In the FM algorithm, a node is either attached to the gain
    bucket, or the freeVertexList. */
class GNode
{
public:
  /** Constructor */
  GNode() : _next(0), _prev(0), _key(0) {}

  /** @return the key. Note that when the node is attached by GDList, the
      key may carry other information such as node index. */
  unsigned int getKey() const { return _key; }

  /** Set the key.  */
  void setKey(unsigned int key) { _key = key; }

public:
  GNode* _next;  /**< point to the next node */
  GNode* _prev;  /**< point to the prev node */
  unsigned int _key; /**< valid key always starts from 1 in GBPQueue */
};



//xxx /** (Experimental) Generic Node with secondary key. @see GNode. */
//xxx class GNode2
//xxx {
//xxx public:
//xxx   /** Constructor */
//xxx   GNode2() : _next(0), _prev(0), _key2(0), _key(0) {}
//xxx 
//xxx   /** @return the key */
//xxx   unsigned int getKey() const { return _key2; }
//xxx 
//xxx   /** Set the key */
//xxx   void setKey(unsigned int key2) { _key2 = key2; }
//xxx 
//xxx   /** @return the second key */
//xxx   unsigned int getSecondKey() const { return _key; }
//xxx 
//xxx   /** Set the key */
//xxx   void setSecondKey(unsigned int key) { _key = key; }
//xxx 
//xxx   /** @return info */
//xxx   unsigned int getInfo() const { return _info; }
//xxx  
//xxx   /** Set info */
//xxx   void setInfo(unsigned int info) { _info = info; }
//xxx 
//xxx public:
//xxx   GNode2* _next;  /**< point to the next node */
//xxx   GNode2* _prev;  /**< point to the prev node */
//xxx   union {
//xxx     unsigned int _info;
//xxx     struct {
//xxx       unsigned short _key2; /**< valid key always starts from 1 in GBPQueue */
//xxx       unsigned short _key;
//xxx     };
//xxx   };
//xxx };


/** Generic KWay Node. For efficiency, the objects of this class can be
    attached to GBPQueue (bounded priority queue) and GDList (list). 
    In the FM algorithm, a node is either attached to the gain
    bucket, or the freeVertexList. */
class GNodeKWay
{
public:
  /** Constructor */
  GNodeKWay() : _next(0), _prev(0), _key(0), _toPart(255), _locked(false) {}

  /** @return the key. Note that when the node is attached by GDList, the
      key may carry other information such as node index. */
  unsigned int getKey() const { return _key; }

  /** Set the key.  */
  void setKey(unsigned int key) { _key = key; }

  bool isLocked() const { return _locked; }

public:
  GNodeKWay* _next;  /**< point to the next node */
  GNodeKWay* _prev;  /**< point to the prev node */
  unsigned short _key; /**< valid key always starts from 1 in GBPQueue */
  unsigned char  _toPart;
  unsigned char  _locked;
};


/** Generic KWay Node. For efficiency, the objects of this class can be
    attached to GBPQueue (bounded priority queue) and GDList (list). 
    In the FM algorithm, a node is either attached to the gain
    bucket, or the freeVertexList. */
class GNodeK
{
public:
  /** Constructor */
  GNodeK() : _next(0), _prev(0), _key(0), _toPart(255), _locked(false) {}

  /** @return the key. Note that when the node is attached by GDList, the
      key may carry other information such as node index. */
  unsigned int getKey() const { return _key; }

  /** Set the key.  */
  void setKey(unsigned int key) { _key = key; }

  bool isLocked() const { return _locked; }

public:
  GNodeK* _next;  /**< point to the next node */
  GNodeK* _prev;  /**< point to the prev node */
  unsigned short _key; /**< valid key always starts from 1 in GBPQueue */
  unsigned char  _toPart;
  unsigned char  _locked;
};




/** @} */ // end of group2
#endif
