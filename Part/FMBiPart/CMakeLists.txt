set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_FLAGS "-Wno-deprecated")

set(CMAKE_CXX_FLAGS_RELEASE "-DNDBUG -finline-limit=6000")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

set(SRC_FILES FMBiGainMgr.cpp FMBiGainMgr2.cpp FMBiPartComm.cpp FMBiPartCore.cpp FMBiPartCore4.cpp FMBiPartMgrBase.cpp FMException.cpp FMPartMgrBase.cpp)

ADD_LIBRARY(FMBiPart ${SRC_FILES})

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/GDsl ${PROJECT_SOURCE_DIR}/Netlist) 
