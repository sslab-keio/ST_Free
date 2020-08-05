option(BUILD_STAGE_ONE "build with stage one" on)
option(BUILD_STAGE_PRIMITIVE "build with stage primitive" off)
option(BUILD_OPTION_BIDIRECTIONAL "build with bidirectional" off)
option(BUILD_STAGE_TWO "build with stage two" off)

option(BUILD_OPTION_NESTED "build with nested option" off)

if (BUILD_STAGE_ONE)
    add_definitions(-DSTAGE_ONE)
endif()

if (BUILD_STAGE_TWO)
    add_definitions(-DSTAGE_TWO)
endif()

if (BUILD_OPTION_BIDIRECTIONAL)
    add_definitions(-DSTAGE_BIRIDRECTIONAL)
endif()

if (BUILD_STAGE_PRIMITIVE)
    add_definitions(-DSTAGE_PRIMITIVE)
endif()

if (BUILD_OPTIONAL_NESTED)
    add_definitions(-DOPTION_NESTED)
endif()

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
    StageOneAnalyzer.cpp
    RelationshipInformation.cpp
    UniqueKeyManager.cpp
    FreedStruct.cpp)
#Use C++ 11 to compile our pass(i.e., supply - std = c++ 11).
target_compile_features(StructFreeMod PRIVATE cxx_range_for cxx_auto_type)

#LLVM is(typically) built with no C++ RTTI.We need to match that;
#otherwise, we'll get linker errors about missing RTTI data.
set_target_properties(StructFreeMod PROPERTIES COMPILE_FLAGS "-fno-rtti")
include_directories(./ include)

# set(STAGE_ONE 0 CACHE BOOL "STAGE_ONE OPTION")
# set(STAGE_TWO 0 CACHE BOOL "STAGE_TWO OPTION")
# set(STAGE_PRIMITIVE 0 CACHE BOOL "STAGE_PRIMITIVE OPTION")
# set(STAGE_BIDIRECTIONAL 0 CACHE BOOL "STAGE_BIDIRECTIONAL OPTION")
# set(OPTION_NESTED 0 CACHE BOOL "OPTION_NESTED")

#if (STAGE_ONE)
# add_definitions(-DOPTION_NESTED)
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
