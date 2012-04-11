#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>


int main()
{
  srand((unsigned)time(NULL));
  CppUnit::TextUi::TestRunner runner;
  runner.addTest( CppUnit::TestFactoryRegistry::getRegistry().makeTest() );
  runner.run( "", false ); // Run all tests and wait
  return 0;
}

