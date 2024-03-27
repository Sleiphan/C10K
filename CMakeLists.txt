cmake_minimum_required(VERSION 3.7...3.26)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(Nettverksprogrammering
    VERSION 0.1
    DESCRIPTION "Verktøy jeg laget i faget IDATT2104_Nettverksprogrammering"
    LANGUAGES C CXX)

enable_testing()
find_package( Threads )


set(CMAKE_PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/pdb)


set(LIBRARY "Nettverksprogrammering")
add_library(${LIBRARY} STATIC)
target_include_directories(${LIBRARY} PUBLIC include)
set_target_properties(${LIBRARY}
    PROPERTIES 
    LINKER_LANGUAGE CXX
    DESCRIPTION "Verktøy jeg laget i faget IDATT2104_Nettverksprogrammering"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/../bin
)



set(UNIX_LIBRARY "Nettverksprogrammering_unix")
add_library(${UNIX_LIBRARY} STATIC)
target_include_directories(${UNIX_LIBRARY} PUBLIC include)
target_link_libraries(${UNIX_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(${UNIX_LIBRARY}
    PROPERTIES 
    LINKER_LANGUAGE CXX
    DESCRIPTION "Verktøy jeg laget i faget IDATT2104_Nettverksprogrammering"
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/../bin
)



# Add Google C++ Testing Framework as part of this project
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)



add_subdirectory(src)
add_subdirectory(apps)
add_subdirectory(test)

