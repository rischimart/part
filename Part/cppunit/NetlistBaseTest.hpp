#ifndef CPP_UNIT_NETLIST_BASE_HPP
#define CPP_UNIT_NETLIST_BASE_HPP

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class NetlistBase;

/** @addtogroup qtcppunit
 *  @{
 */

/**
 * A test case for NetlistBase.
 *
 */
class NetlistBaseTestCase : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( NetlistBaseTestCase );
  CPPUNIT_TEST( testCreate );
  CPPUNIT_TEST( testCell );
  CPPUNIT_TEST( testNet );
  CPPUNIT_TEST( testPin );
  CPPUNIT_TEST( testCellCursor );
  CPPUNIT_TEST( testNetCursor );
  CPPUNIT_TEST( testPinCursor );
  CPPUNIT_TEST( testCutCost );
  CPPUNIT_TEST_SUITE_END();

protected:
  NetlistBase* _H;

public:
  void setUp();
  void tearDown();

protected:
  /** Test creation */
  void testCreate();

  /** Test Cell */
  void testCell();

  /** Test Net */
  void testNet();

  /** Test Pin */
  void testPin();

  /** Test CellCursor */
  void testCellCursor();

  /** Test NetCursor */
  void testNetCursor();

  /** Test PinCursor */
  void testPinCursor();

  /** Test cut-cost */
  void testCutCost();
};

/** @} */
#endif
