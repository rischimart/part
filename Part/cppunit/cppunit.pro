TEMPLATE	= app
CONFIG		= warn_on debug thread
CONFIG          -= qt moc

unix:LIBS       += -lcppunit
INCLUDEPATH     += ../FMBiPart ../GDsl ../Netlist ../..
unix:LIBS       += -L../lib -lNetlist -lGDsl -ldl

HEADERS		= CompilerTest.hpp \
                  GDListTest.hpp \
                  GBPQueueTest.hpp \
                  NetlistBaseTest.hpp \
                  NetlistTest.hpp
SOURCES		= CompilerTest.cpp \
                  GDListTest.cpp \
		  GBPQueueTest.cpp \
                  NetlistBaseTest.cpp \
                  NetlistTest.cpp \
		  Main.cpp
INTERFACES	= 
