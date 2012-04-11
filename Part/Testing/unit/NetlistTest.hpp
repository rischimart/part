#ifndef CPP_UNIT_NETLIST_HPP
#define CPP_UNIT_NETLIST_HPP

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class Netlist;

/** @addtogroup qtcppunit
 *  @{
 */

/**
 * A test case for Netlist
 *
 */
class NetlistTestCase : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( NetlistTestCase );
  CPPUNIT_TEST( testMNC );
  CPPUNIT_TEST( testMIS );
  //xxx CPPUNIT_TEST( testContraction );
  //xxx CPPUNIT_TEST( testProjection );
  CPPUNIT_TEST_SUITE_END();

protected:
  Netlist* _H;

public:

  void setUp();
  void tearDown();

protected:
  /** Test minimum net cover */
  void testMNC();

  /** Test maximum independent set */
  void testMIS();

  /** Test contraction methods */
  void testContraction();

  /** Test projection methods */
  void testProjection();
};

/** @} */
#endif
