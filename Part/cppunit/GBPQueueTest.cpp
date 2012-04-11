#include "GBPQueueTest.hpp"
#include <GBPQueue.hpp>
#include <GNode.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION( GBPQueueTestCase );

void GBPQueueTestCase::setUp ()
{
  _PQ1 = new GBPQueue<GNode>(-10, 10);
  _PQ2 = new GBPQueue<GNode>(-10, 10);
}

void GBPQueueTestCase::tearDown()
{
  delete _PQ1;
  delete _PQ2;
}

void GBPQueueTestCase::testPushPop()
{
  CPPUNIT_ASSERT (_PQ1->isEmpty());

  _PQ1->pushBack(_d[1], 3);
  _PQ1->pushBack(_d[2], 8);
  _PQ1->pushBack(_d[3], -1);

  _PQ1->popFront();
  _PQ1->popFront();
  _PQ1->popFront();

  CPPUNIT_ASSERT (_PQ1->isEmpty());  
}

void GBPQueueTestCase::testCursors()
{
  _PQ1->pushBack(_d[2], 3);
  //xxx _PQ1->pushFront(_d[1], -10);
  _PQ1->pushBack(_d[0], 5);

  // Const Cursor
  int count = 0;
  GBPQueueCursor<GNode> pqc2(*_PQ1);
  while (pqc2.hasNext()) {
    pqc2.getNext();
    count++;
  }
  CPPUNIT_ASSERT (count == 2);

  count = 0;
  GBPQueueCursor<GNode> pqc(*_PQ1);
  while (pqc.hasNext()) {
    GNode& aNode = pqc.getNext();
    _PQ1->detach(aNode); // should not invalidate the cursor
    count++;
  }
  CPPUNIT_ASSERT (count == 2);
  CPPUNIT_ASSERT (_PQ1->isEmpty());  
}
