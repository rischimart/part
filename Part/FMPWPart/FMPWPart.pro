######################################################################
# Automatically generated by qmake (1.03a) Thu Nov 27 23:19:14 2003
######################################################################

TEMPLATE = lib
CONFIG += staticlib release
CONFIG -= moc qt
QMAKE_CXXFLAGS += -Wno-deprecated
QMAKE_CXXFLAGS_RELEASE += -DNDEBUG -finline-limit=6000

# QMAKE_CXXFLAGS += -pg -fno-inline -fno-implement-inlines

# Input
HEADERS += FMPWPartTmpl.hpp 
SOURCES += FMPWPartTmpl.cpp  

INCLUDEPATH += ../FMPart ../FMKWayPart ../FMBiPart ../Netlist ../GDsl ../..
#INCLUDEPATH += ../../loki ../../STLport-4.5.3/stlport

DESTDIR = ../lib
