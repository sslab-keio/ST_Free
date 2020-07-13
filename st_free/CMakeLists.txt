add_library(StructFreeMod MODULE
    ST_free.cpp 
    statList.cpp 
    determinator.cpp 
    support_funcs.cpp
    FunctionManager.cpp
    BaseAnalyzer.cpp
    ArgList.cpp
    BasicBlockManager.cpp
    ValueInformation.cpp
    StructInformation.cpp
    LoopManager.cpp
    StageOneAnalyzer.cpp
    RelationshipInformation.cpp
    UniqueKeyManager.cpp
    FreedStruct.cpp)
#Use C++ 11 to compile our pass(i.e., supply - std = c++ 11).
target_compile_features(StructFreeMod PRIVATE cxx_range_for cxx_auto_type)
# if(OPENMP_FOUND)
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
# endif()
#LLVM is(typically) built with no C++ RTTI.We need to match that;
#otherwise, we'll get linker errors about missing RTTI data.
set_target_properties(StructFreeMod PROPERTIES COMPILE_FLAGS "-fno-rtti")
include_directories(./ include)

set(STAGE_ONE 0 CACHE BOOL "STAGE_ONE OPTION")
set(STAGE_TWO 0 CACHE BOOL "STAGE_TWO OPTION")
set(STAGE_PRIMITIVE 0 CACHE BOOL "STAGE_PRIMITIVE OPTION")
set(STAGE_BIDIRECTIONAL 0 CACHE BOOL "STAGE_BIDIRECTIONAL OPTION")
set(OPTION_NESTED 0 CACHE BOOL "OPTION_NESTED")

#if (STAGE_ONE)
add_definitions(-DSTAGE_ONE)
# add_definitions(-DSTAGE_PRIMITIVE)
#endif(STAGE_ONE)

#if (STAGE_TWO)
# add_definitions(-DSTAGE_TWO)
#endif(STAGE_TWO)

#if (STAGE_PRIMITIVE)
#add_definitions(-DSTAGE_PRIMITIVE)
#endif(STAGE_PRIMITIVE)

#if (OPTION_NESTED)
#add_definitions(-DOPTION_NESTED)
#endif(OPTION_NESTED)

#Get proper shared - library behavior(where symbols are not necessarily
#resolved when the shared library is linked) on OS X.
if (APPLE)
set_target_properties(
    StructFreeMod PROPERTIES LINK_FLAGS
    "-undefined dynamic_lookup")
endif(APPLE)
