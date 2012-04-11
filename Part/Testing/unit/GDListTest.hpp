#ifndef CPP_UNIT_GDLIST_HPP
#define CPP_UNIT_GDLIST_HPP

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <GNode.hpp>

template <class Node> class GDList;

/** @addtogroup qtcppunit
 *  @{
 */

/**
 * A test case for GDList
 *
 */
class GDListTestCase : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( GDListTestCase );
  CPPUNIT_TEST( testPushPop );
  CPPUNIT_TEST( testCursor );
  CPPUNIT_TEST( testConstCursor );
  CPPUNIT_TEST_SUITE_END();

protected:
  GNode _d[4];
  GDList<GNode>* _L1;
  GDList<GNode>* _L2;

public:
  void setUp();
  void tearDown();

protected:
  /** Test push/pop feature */
  void testPushPop();

  /** Test cursor */
  void testCursor();

  /** Test constant cursor */
  void testConstCursor();
};

/** @} */
#endif
