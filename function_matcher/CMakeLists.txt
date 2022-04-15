find_package(Rizin COMPONENTS Core)
include_directories(${rz_core_INCLUDE_DIRS})
add_library(function_matcher SHARED
        function_matcher.cpp
        command_help.h
        matcher_types.h matcher_types.cpp
        command_handlers.h command_handlers.cpp)
target_link_directories(function_matcher PUBLIC ${rz_core_LIBRARY_DIRS})
target_link_libraries(function_matcher ${rz_core_LIBRARIES})
