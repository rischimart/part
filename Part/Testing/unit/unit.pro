TEMPLATE	= app
CONFIG		= warn_on release thread
CONFIG          -= qt
win32-msvc:LIBS			= ../../lib/cppunit.lib ../../lib/qttestrunner.lib
win32-msvc:TMAKE_CXXFLAGS	= /GX /GR
win32-msvc:DEFINES		= QT_DLL QTTESTRUNNER_DLL

unix:LIBS       += -lcppunit

INCLUDEPATH     += ../../FMBiPart ../../GDsl ../../Netlist ../../..
unix:LIBS       += -L../../lib -lNetlist -lGDsl -ldl

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
TARGET		= unittest
