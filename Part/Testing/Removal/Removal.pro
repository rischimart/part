######################################################################
# Automatically generated by qmake (1.03a) Sat Mar 15 13:34:51 2003
######################################################################

TEMPLATE = app
CONFIG -= moc qt
CONFIG += release
QMAKE_CXXFLAGS += -Wno-deprecated
QMAKE_CXXFLAGS_RELEASE += -pg -DNDEBUG 

# Input
SOURCES += main.cpp
INCLUDEPATH += ../../.. ../../Path ../../FMBiPart ../../Netlist ../../GDsl
unix:LIBS += -L../../lib -lPath -lNetlist 
# INCLUDEPATH += ../../loki ../../STLport-4.5.3/stlport
# 
# release {
# unix:LIBS += -L../../STLport-4.5.3/lib -lstlport_gcc -lpthread
# }
# 
# debug {
# unix:LIBS += -L../../STLport-4.5.3/lib -lstlport_gcc_stldebug -lpthread
# }
