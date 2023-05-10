cmake_minimum_required (VERSION 2.8.12)
project (pfe)
set(NAME "pfe")

set(CMAKE_FOLDER ./)
include(${CMAKE_FOLDER}/cmake/pfe-base.cmake)


find_package(RELIC QUIET)
if(RELIC_FOUND)
	message(STATUS "Found RELIC")
elseif(NOT RELIC_FOUND AND NOT TARGET RELIC::relic)
	message("RELIC was not found: add RELIC subdirectory")
	if(NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/relic/CMakeLists.txt")
		find_package(Git REQUIRED)
		message("initialize Git submodule: extern/relic")
		execute_process(COMMAND git submodule update --init extern/relic
						WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
	else() 
	    message("find submodule: extern/relic")
	endif()
endif()

set(ecclvl 251) #set this for the ecc security level

#relic library configuration
set(DEBUG off CACHE BOOL "Build relic with debugging support")
set(PROFL off CACHE BOOL "Build relic with profiling support")
set(CHECK off CACHE BOOL "Build relic with error-checking support")
set(ALIGN "16" CACHE STRING "Relic align")
set(ARCH "X64" CACHE STRING "Architecture to be used in relic")
set(ARITH "curve2251-sse" CACHE STRING "arithmetic utils used in relic")
set(FB_POLYN ${ecclvl} CACHE INTEGER "security level of the ecc binary curve in relic")
set(FB_METHD "INTEG;INTEG;QUICK;QUICK;QUICK;QUICK;LOWER;SLIDE;QUICK" CACHE STRING "Methods for fb in relic")
set(FB_PRECO on CACHE BOOl "fb preco for relic")
set(FB_SQRTF off CACHE BOOL "sqrtf for relic")
set(EB_METHD "PROJC;LODAH;COMBS;INTER" CACHE STRING "Methods for eb in relic")
set(EC_METHD "CHAR2" CACHE STRING "Methods for ec in relic")
set(CFLAGS "-O3 -funroll-loops -fomit-frame-pointer -march=core2 -msse4.2 -mpclmul" CACHE STRING "Relic cflags")
set(TIMER "CYCLE" CACHE STRING "Relic timer")
set(WITH "MD;DV;BN;FB;EB;EC" CACHE STRING "Relic algorithms")
set(WSIZE "64" CACHE STRING "Relic word size in bits")
set(TESTS "0" CACHE STRING "Relic amount of random tests, 0 for disable")
set(BENCH "0" CACHE STRING "Relic amount of benchmarks on random values, 0 for disable")
set(SHLIB off CACHE BOOL "Relic shared library")
set(STLIB on CACHE BOOL "Relic static library")
add_subdirectory(extern/relic)

#looks dirty, but since the relic library is poorly exported let's treat it as an external library
if(CMAKE_LIBRARY_OUTPUT_DIRECTORY)
	set(RELIC_LIB_DIR "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/librelic_s.a")
else()
	set(RELIC_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/extern/relic/lib/librelic_s.a")
endif()

add_library(RELIC::relic STATIC IMPORTED)
set_target_properties(RELIC::relic PROPERTIES
	IMPORTED_LOCATION ${RELIC_LIB_DIR}
	INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/extern/relic/include;${CMAKE_CURRENT_SOURCE_DIR}/extern/relic/include/low;${CMAKE_CURRENT_BINARY_DIR}/extern/relic/include"
)

target_link_libraries(RELIC::relic INTERFACE relic_s)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/extern/relic/include)


# SET(CMAKE_BUILD_TYPE "Debug")

# SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")

# SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
# add_definitions("-Wall -g")
add_definitions(-D_DEBUG)
add_definitions(-DDEBUG)
add_definitions("$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
set(sources
pfe/pfe.cpp
)


find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
FIND_PACKAGE( OpenMP REQUIRED)
if(OPENMP_FOUND)
    message("OPENMP FOUND")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()
include(${CMAKE_FOLDER}/cmake/pfe-base.cmake)

add_library(${NAME} SHARED ${sources})
install(DIRECTORY pfe DESTINATION include/)
install(DIRECTORY cmake/ DESTINATION cmake/)
install(TARGETS ${NAME} DESTINATION lib)

ENABLE_TESTING()
ADD_SUBDIRECTORY(test)

target_link_libraries(pfe
    PUBLIC
        GMP::GMP
        GMP::GMPXX
        OpenSSL::Crypto
        RELIC::relic
)

