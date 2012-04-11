#include "CompilerTest.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION( CompilerTestCase );

void CompilerTestCase::setUp ()
{
}

void CompilerTestCase::tearDown()
{
}


void CompilerTestCase::testBitField()
{
  struct BitField
  {
    int id : 29;
    bool visited : 1;
  };

  BitField B;
  B.id = -1;
  int myid = B.id;

  /* Note that some old compilers may (mistakenly) convert the signed
     bit field value to unsigned value. */
  CPPUNIT_ASSERT (myid == -1);
}
