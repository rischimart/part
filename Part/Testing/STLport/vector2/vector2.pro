######################################################################
# Automatically generated by qmake (1.03a) Sat Mar 15 12:44:37 2003
######################################################################

TEMPLATE = app
CONFIG += debug
CONFIG -= moc qt
#QMAKE_CXXFLAGS_DEBUG += -D_STLP_DEBUG

# Input
SOURCES += main.cpp

INCLUDEPATH += ../../../../loki ../../../../STLport-4.5.3/stlport

release {
unix:LIBS += -L../../../../STLport-4.5.3/lib -lstlport_gcc -lpthread
}

debug {
unix:LIBS += -L../../../../STLport-4.5.3/lib -lstlport_gcc_stldebug -lpthread
}
