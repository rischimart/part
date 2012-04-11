#ifndef CPP_UNIT_GBPQUEUE_HPP
#define CPP_UNIT_GBPQUEUE_HPP

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <GNode.hpp>

template <class Node> class GBPQueue;

/** @addtogroup qtcppunit
 *  @{
 */

/**
 * A test case for GBPQueue
 */
class GBPQueueTestCase : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( GBPQueueTestCase );
  CPPUNIT_TEST( testPushPop );
  CPPUNIT_TEST( testCursors );
  CPPUNIT_TEST_SUITE_END();

protected:
  GNode            _d[6];
  GBPQueue<GNode>* _PQ1;
  GBPQueue<GNode>* _PQ2;

public:
  void setUp();
  void tearDown();

protected:
  /** Test push/pop feature */
  void testPushPop();

  /** Test cursors */
  void testCursors();
};

/** @} */
#endif
