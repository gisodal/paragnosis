cmake_minimum_required(VERSION 3.0)

message(STATUS "Prefix: ${CMAKE_INSTALL_PREFIX}")

# set directory as project name
get_filename_component(PROJECT ${CMAKE_CURRENT_LIST_DIR} NAME)
string(REPLACE " " "_" PROJECT ${PROJECT})
project(${PROJECT} C CXX)

# set compiler flags
get_filename_component(WMC "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

message(STATUS "Build type: '${CMAKE_BUILD_TYPE}'")
set(CMAKE_OPTIMIZATION "-O3")
if (CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES DEBUG)
    set(CMAKE_OPTIMIZATION "-O0")
else()
    set(W "-w")
endif()
set(CMAKE_C_FLAGS "${W} ${CMAKE_OPTIMIZATION}")
set(CMAKE_CXX_FLAGS "--std=c++11 ${CMAKE_C_FLAGS}")
set(CMAKE_C_FLAGS_RELEASE "-DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
set(CMAKE_C_FLAGS_DEBUG "-ggdb3 -Wall -Wextra -DDEBUG   \
    -Wno-format -Wno-write-strings -Wno-unused-function \
    -Wno-unused-parameter -Wno-system-headers           \
    -Wno-format-security -Wno-ignored-qualifiers        \
    -fno-inline-functions")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")

# set source files
set(SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)

file(GLOB HEADERS "${INCLUDE_DIR}/*.h")
file(GLOB SOURCES "${SOURCE_DIR}/*.cc")

set(MAIN ${SOURCE_DIR}/main.cc)
list(REMOVE_ITEM SOURCES ${MAIN})

## set include directories
include_directories( ${INCLUDE_DIR} ${CMAKE_INSTALL_PREFIX}/include ${CMAKE_INSTALL_PREFIX}/usr/include)
link_directories( ${CMAKE_INSTALL_PREFIX}/lib ${CMAKE_INSTALL_PREFIX}/usr/lib64)

# create executable
find_package(Threads)  # includes pthread
find_package(Boost REQUIRED COMPONENTS system )
find_package(Boost REQUIRED COMPONENTS program_options thread )
include_directories( ${Boost_INCLUDE_DIRS} )
add_library(daooptall INTERFACE)
target_link_libraries(daooptall INTERFACE daoopt ${CMAKE_THREAD_LIBS_INIT} ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY})

# create library
add_library(${PROJECT} STATIC ${SOURCES})

# create binary
add_executable(${PROJECT}-bin ${MAIN})
set_target_properties(${PROJECT}-bin PROPERTIES OUTPUT_NAME ${PROJECT})
target_link_libraries(${PROJECT}-bin ${PROJECT} bnc daooptall bn-to-cnf quine-mccluskey gsl gslcblas gmp gmpxx dl pthread readline termcap)

# set install locations
install(TARGETS ${PROJECT}-bin
    RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/bin"
)

set_target_properties(${PROJECT} PROPERTIES PUBLIC_HEADER "${HEADERS}")
install(TARGETS ${PROJECT}
    ARCHIVE DESTINATION "${CMAKE_INSTALL_PREFIX}/lib"
    LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}/lib"
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_PREFIX}/include/${PROJECT}"
)

