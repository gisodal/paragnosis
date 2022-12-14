cmake_minimum_required(VERSION 3.0)

if (NOT UNIX)
    message( FATAL_ERROR "This program is unix specific." )
endif ()

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_LIST_DIR}")
endif()

include (ExternalProject)

ExternalProject_Add(exception
    SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/src/exception"
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
    CMAKE_ARGS
        -DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}
        -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
        -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    INSTALL_COMMMAND ""
)

file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_LIST_DIR}/src/exception/include/*")
ExternalProject_Add_Step(exception forceconfigure
    COMMAND ${CMAKE_COMMAND} -E echo -n "-- Reconfigure"
    DEPENDS ${SOURCES}
    DEPENDERS configure
)

# ==============================================================================
set(PROJECT quine-mccluskey)
ExternalProject_Add(${PROJECT}
    SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/src/${PROJECT}"
    DEPENDS exception
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
    CMAKE_ARGS
        -DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}
        -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
        -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
)

file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_LIST_DIR}/src/${PROJECT}/include/*" "${CMAKE_CURRENT_LIST_DIR}/src/${PROJECT}/src/*")
ExternalProject_Add_Step(quine-mccluskey forceconfigure
    COMMAND ${CMAKE_COMMAND} -E echo -n "-- Reconfigure"
    DEPENDS ${SOURCES}
    DEPENDERS configure
)

set(BIN ${CMAKE_BINARY_DIR}/${PROJECT}-prefix/src/${PROJECT}-build/${PROJECT})
install(PROGRAMS "${BIN}" DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")


# ==============================================================================
set(PROJECT bn-to-cnf)
ExternalProject_Add(${PROJECT}
    SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/src/${PROJECT}"
    DEPENDS exception quine-mccluskey
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
    CMAKE_ARGS
        -DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}
        -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
        -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
)

file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_LIST_DIR}/src/${PROJECT}/include/*" "${CMAKE_CURRENT_LIST_DIR}/src/${PROJECT}/src/*")
ExternalProject_Add_Step(${PROJECT} forceconfigure
    COMMAND ${CMAKE_COMMAND} -E echo -n "-- Reconfigure"
    DEPENDS ${SOURCES}
    DEPENDERS configure
)

set(BIN ${CMAKE_BINARY_DIR}/${PROJECT}-prefix/src/${PROJECT}-build/${PROJECT})
install(PROGRAMS "${BIN}" DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")

# ==============================================================================
set(PROJECT pseudotree)
ExternalProject_Add(${PROJECT}
    SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/src/${PROJECT}"
    DEPENDS exception quine-mccluskey bn-to-cnf
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
    CMAKE_ARGS
        -DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}
        -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
        -DBOOST_ROOT=${BOOST_ROOT}
        -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
)

file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_LIST_DIR}/src/${PROJECT}/include/*" "${CMAKE_CURRENT_LIST_DIR}/src/${PROJECT}/source/*")
ExternalProject_Add_Step(${PROJECT} forceconfigure
    COMMAND ${CMAKE_COMMAND} -E echo -n "-- Reconfigure"
    DEPENDS ${SOURCES}
    DEPENDERS configure
)

# ==============================================================================
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/src/bnc/CMakeLists.txt")

    set(PROJECT bnc)
    ExternalProject_Add(bnc
        SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/src/${PROJECT}"
        DEPENDS exception quine-mccluskey bn-to-cnf pseudotree
        INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
        CMAKE_ARGS
            -DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}
            -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
            -DBOOST_ROOT=${BOOST_ROOT}
            -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
            -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
            -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
            -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    )

    file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_LIST_DIR}/src/${PROJECT}/include/*" "${CMAKE_CURRENT_LIST_DIR}/src/${PROJECT}/src/*")
    ExternalProject_Add_Step(${PROJECT} forceconfigure
        COMMAND ${CMAKE_COMMAND} -E echo -n "-- Reconfigure"
        DEPENDS ${SOURCES}
        DEPENDERS configure
    )

    set(BIN ${CMAKE_BINARY_DIR}/${PROJECT}-prefix/src/${PROJECT}-build/${PROJECT})
    install(PROGRAMS "${BIN}" DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")

endif()

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/src/bnmc/CMakeLists.txt")

    # ==============================================================================
    set(PROJECT bnmc)
    ExternalProject_Add(${PROJECT}
        SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/src/${PROJECT}"
        DEPENDS exception quine-mccluskey bn-to-cnf bnc pseudotree
        INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
        UPDATE_COMMAND ""
        CMAKE_ARGS
            -DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}
            -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
            -DBOOST_ROOT=${BOOST_ROOT}
            -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
            -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
            -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
            -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    )

    file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_LIST_DIR}/src/${PROJECT}/include/*" "${CMAKE_CURRENT_LIST_DIR}/src/${PROJECT}/src/*")
    ExternalProject_Add_Step(${PROJECT} forceconfigure
        COMMAND ${CMAKE_COMMAND} -E echo -n "-- Reconfigure"
        DEPENDS ${SOURCES}
        DEPENDERS configure
    )

    set(BIN ${CMAKE_BINARY_DIR}/${PROJECT}-prefix/src/${PROJECT}-build/${PROJECT})
    install(PROGRAMS "${BIN}" DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")

endif()

