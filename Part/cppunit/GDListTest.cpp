#include "GDListTest.hpp"
#include <GDList.hpp>
#include <GNode.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION( GDListTestCase );

void GDListTestCase::setUp ()
{
  _L1 = new GDList<GNode>;
  _L2 = new GDList<GNode>;
}

void GDListTestCase::tearDown()
{
  delete _L1;
  delete _L2;
}

void GDListTestCase::testPushPop()
{
  CPPUNIT_ASSERT (_L1->isEmpty());
  _L1->pushBack(_d[1]);
  _L1->pushBack(_d[3]);
  _L1->pushBack(_d[2]);
  _L1->popFront();
  _L1->popFront();
  _L1->popFront();
  CPPUNIT_ASSERT (_L1->isEmpty());
}

void GDListTestCase::testCursor()
{
  CPPUNIT_ASSERT (_L1->isEmpty());
  _L1->pushBack(_d[1]);
  _L1->pushBack(_d[2]);
  _L1->pushBack(_d[3]);
  int count = 0;
  GDListCursor<GNode> pc(*_L1);
  while (pc.hasNext()) {
    GNode& aNode = pc.getNext();
    _L1->detach(aNode); // should not invalidate the cursor
    count++;
  }
  CPPUNIT_ASSERT (count == 3);
  CPPUNIT_ASSERT (_L1->isEmpty());  
}

void GDListTestCase::testConstCursor()
{
  CPPUNIT_ASSERT (_L1->isEmpty());
  _L1->pushBack(_d[1]);
  _L1->pushBack(_d[0]);
  _L1->pushBack(_d[2]);
  int count = 0;
  const GDList<GNode>* L = _L1;
  GDListConstCursor<GNode> pc(*L);
  while (pc.hasNext()) {
    pc.getNext();
    count++;
  }
  CPPUNIT_ASSERT (count == 3);
  _L1->clear();
}
