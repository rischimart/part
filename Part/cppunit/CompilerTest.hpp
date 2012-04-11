#ifndef CPP_UNIT_COMPILER_HPP
#define CPP_UNIT_COMPILER_HPP

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

/** @defgroup cppunit Unit Test Module.
 *  @ingroup Part
 *  @{
 */

/**
 * A test case for Compiler
 */
class CompilerTestCase : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( CompilerTestCase );
  CPPUNIT_TEST( testBitField );
  CPPUNIT_TEST_SUITE_END();

public:

  void setUp();
  void tearDown();

protected:
  /** Test the bit field problem */
  void testBitField();
};

/** @} */
#endif
