######################################################################
# Automatically generated by qmake (1.03a) Sat Mar 15 13:34:51 2003
######################################################################

TEMPLATE = app
CONFIG -= moc qt
CONFIG += debug
QMAKE_CXXFLAGS_RELEASE += -DNDEBUG

# QMAKE_CXXFLAGS += -pg
# QMAKE_LFLAGS += -pg

# Input
SOURCES += main.cpp
INCLUDEPATH += ../../Netlist ../../FMPart ../../FMPWPart ../../FMKWayPart \
               ../../FMBiPart ../../GDsl
unix:LIBS += -L../../lib -lFMPWPart -lFMPart -lFMKWayPart \
               -lFMBiPart -lNetlist -lGDsl 

INCLUDEPATH += ../../..

# INCLUDEPATH += ../../loki ../../STLport-4.5.3/stlport
# 
# release {
# unix:LIBS += -L../../STLport-4.5.3/lib -lstlport_gcc -lpthread
# }
# 
# debug {
# unix:LIBS += -L../../STLport-4.5.3/lib -lstlport_gcc_stldebug -lpthread
# }
