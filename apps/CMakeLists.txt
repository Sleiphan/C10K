
set(EXECUTABLE "Run")
add_executable(${EXECUTABLE} Run.c)
# target_link_libraries(${EXECUTABLE} ${LIBRARY})
set_target_properties(${EXECUTABLE}
    PROPERTIES 
    LINKER_LANGUAGE CXX
    CXX_STANDARD 17
)

set(EXECUTABLE "Run_unix_srv")
add_executable(${EXECUTABLE} Run_unix_srv.c)
target_link_libraries(${EXECUTABLE} ${UNIX_LIBRARY})
set_target_properties(${EXECUTABLE}
    PROPERTIES 
    LINKER_LANGUAGE CXX
    CXX_STANDARD 17
)

set(EXECUTABLE "Epoll_test")
add_executable(${EXECUTABLE} Epoll_test.c)
target_link_libraries(${EXECUTABLE} ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(${EXECUTABLE}
    PROPERTIES 
    LINKER_LANGUAGE CXX
    CXX_STANDARD 17
)

set(EXECUTABLE "Server_example")
add_executable(${EXECUTABLE} Server_example.c)
target_link_libraries(${EXECUTABLE} ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(${EXECUTABLE}
    PROPERTIES 
    LINKER_LANGUAGE CXX
    CXX_STANDARD 17
)

set(EXECUTABLE "Run_calc_srv")
add_executable(${EXECUTABLE} Run_calc_srv.c)
target_link_libraries(${EXECUTABLE} ${UNIX_LIBRARY})
set_target_properties(${EXECUTABLE}
    PROPERTIES 
    LINKER_LANGUAGE CXX
    CXX_STANDARD 17
)

set(EXECUTABLE "Run_calc_udp_srv")
add_executable(${EXECUTABLE} Run_calc_udp_srv.c)
target_link_libraries(${EXECUTABLE} ${UNIX_LIBRARY})
set_target_properties(${EXECUTABLE}
    PROPERTIES 
    LINKER_LANGUAGE CXX
    CXX_STANDARD 17
)