add_library( StructFreeMod MODULE
    ST_free.cpp
    statList.cpp
    determinator.cpp
    support_funcs.cpp
    FunctionManager.cpp
    BaseAnalyzer.cpp
    ArgList.cpp
    BBWorklist.cpp
    ValueInformation.cpp
    StructInformation.cpp
    LoopManager.cpp
    StageOneAnalyzer.cpp
    RelationshipInformation.cpp
    UniqueKeyManager.cpp
)
# Use C++11 to compile our pass (i.e., supply -std=c++11).
target_compile_features(StructFreeMod PRIVATE cxx_range_for cxx_auto_type)

# LLVM is (typically) built with no C++ RTTI. We need to match that;
# otherwise, we'll get linker errors about missing RTTI data.
set_target_properties(StructFreeMod PROPERTIES
    COMPILE_FLAGS "-fno-rtti"
)
include_directories(./include)

add_definitions(-DSTAGE_ONE)

# Get proper shared-library behavior (where symbols are not necessarily
# resolved when the shared library is linked) on OS X.
if(APPLE)
    set_target_properties(StructFreeMod PROPERTIES
        LINK_FLAGS "-undefined dynamic_lookup"
    )
endif(APPLE)
