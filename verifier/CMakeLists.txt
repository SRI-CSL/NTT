cmake_minimum_required(VERSION 3.3)

project(NTTVerifier)

if(CMAKE_VERSION VERSION_GREATER "3.13.0")
cmake_policy(SET CMP0074 NEW)
cmake_policy(SET CMP0077 NEW)
endif()

## LLVM 11 requires c++14
set(CMAKE_CXX_STANDARD 14 CACHE STRING "C++ standard to conform to")
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR )
  message (FATAL_ERROR
    "In-source builds are not allowed. Please clean your source tree and try again.")
endif()

# Default is release with debug info
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
    "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install CACHE PATH "Forcing installation directory" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# Add path for custom modules
list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

## Boost
find_package (Boost 1.65 REQUIRED)
if (Boost_FOUND)
  include_directories (${Boost_INCLUDE_DIRS})
  if(NOT LLVM_ENABLE_EH)
    add_definitions(-DBOOST_NO_EXCEPTIONS)
  endif()
else()
  message (FATAL_ERROR
    "No boost found. Need to install boost >= 1.65")
endif ()

## Llvm  
find_package (LLVM 11 QUIET CONFIG)
if (NOT LLVM_FOUND)
  find_package (LLVM 11.1 CONFIG)
endif()

if (NOT LLVM_FOUND)
  message (FATAL_ERROR
    "No llvm 11 found. If you installed it in a non-standard location "
    " then set -DLLVM_DIR=<path>/lib/cmake/llvm")
else()
  message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
  message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

  # We incorporate the CMake features provided by LLVM:
  list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
  include(AddLLVM)
  include(HandleLLVMOptions)
  set(LLVM_RUNTIME_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/bin)
  set(LLVM_LIBRARY_OUTPUT_INTDIR ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/lib)

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LLVM_CXXFLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LLVM_LDFLAGS}")

  include_directories(${LLVM_INCLUDE_DIRS})
  link_directories(${LLVM_LIBRARY_DIRS})
  add_definitions(${LLVM_DEFINITIONS})

  if (NOT LLVM_BUILD_TYPE STREQUAL CMAKE_BUILD_TYPE)
    message(WARNING
      "LLVM_BUILD_TYPE and CMAKE_BUILD_TYPE differ:\n"
      "\tLLMV_BUILD_TYPE=${LLVM_BUILD_TYPE}\n"
      "\tCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}\n"
      "Runtime errors might occur.")
    if (LLVM_BUILD_TYPE MATCHES "Release|RelWithDebInfo" AND 
        CMAKE_BUILD_TYPE MATCHES "Release|RelWithDebInfo|Coverage")
      message(STATUS "Assuming that mixing Release and RelWithDebInfo or Coverage is allowed.")
    else()
      message(FATAL_ERROR "Incompatible mix of LLVM_BUILD_TYPE and CMAKE_BUILD_TYPE")
    endif()
  endif()
endif()

##------------------------------------------------------------------##
#       Clam and its requirements (sea-dsa and llvm-seahorn)
##------------------------------------------------------------------##
set(CRAB_SOURCE_DIR    "${CMAKE_SOURCE_DIR}/clam/crab" CACHE STRING "crab source directory" FORCE)
set(CLAM_SOURCE_DIR    "${CMAKE_SOURCE_DIR}/clam" CACHE STRING "clam source directory" FORCE)
set(SEADSA_SOURCE_DIR  "${CMAKE_SOURCE_DIR}/clam-ext/sea-dsa" CACHE STRING "sea-dsa source directory" FORCE)
set(SEALLVM_SOURCE_DIR "${CMAKE_SOURCE_DIR}/clam-ext/llvm-seahorn" CACHE STRING "llvm-seahorn source directory" FORCE)

find_package(Git)
if (GIT_FOUND)
  set(CRAB_REPO "https://github.com/seahorn/crab" CACHE STRING "crab repo")  
  set(CLAM_REPO "https://github.com/seahorn/crab-llvm" CACHE STRING "clam repo")
  set(SEADSA_REPO "https://github.com/seahorn/sea-dsa" CACHE STRING "sea-dsa repo")
  set(SEALLVM_REPO "https://github.com/seahorn/llvm-seahorn" CACHE STRING "llvm-seahorn repo")

  add_custom_target(sea-dsa-git
    ${GIT_EXECUTABLE} clone -b dev11 ${SEADSA_REPO} ${SEADSA_SOURCE_DIR})
  add_custom_target(seahorn-llvm-git
    ${GIT_EXECUTABLE} clone -b dev11 ${SEALLVM_REPO} ${SEALLVM_SOURCE_DIR})
  add_custom_target(crab-ntt-git
    ${GIT_EXECUTABLE} clone -b ntt ${CRAB_REPO} ${CRAB__SOURCE_DIR})  
  add_custom_target(clam-git 
    ${GIT_EXECUTABLE} clone -b dev11 ${CLAM_REPO} ${CLAM_SOURCE_DIR})
  
  add_custom_target(clam-seallvm DEPENDS seahorn-llvm-git)
  add_custom_target(clam-seadsa  DEPENDS sea-dsa-git )  
  add_custom_target(ntt-clam    DEPENDS clam-git crab-ntt-git)
else()
  message (FATAL_ERROR "Clam cannot be downloaded because git is not found")
endif()

## sea-dsa
if (IS_DIRECTORY ${SEADSA_SOURCE_DIR} AND EXISTS ${SEADSA_SOURCE_DIR}/CMakeLists.txt)
  include_directories(BEFORE ${SEADSA_SOURCE_DIR}/include)
  add_subdirectory(${SEADSA_SOURCE_DIR}/lib/seadsa)
  set(SEA_DSA_LIBS SeaDsaAnalysis)
else()
  message (WARNING "sea-dsa is required by clam but not found in ${SEADSA_SOURCE_DIR}. "
                   "Run\n\tcmake --build . --target clam-seadsa && cmake ${CMAKE_SOURCE_DIR}")
  return()
endif()

## llvm-seahorn
if (IS_DIRECTORY ${SEALLVM_SOURCE_DIR} AND EXISTS ${SEALLVM_SOURCE_DIR}/lib/CMakeLists.txt)
  include_directories(BEFORE ${SEALLVM_SOURCE_DIR}/include)
  add_subdirectory(${SEALLVM_SOURCE_DIR}/lib)
  add_subdirectory(${SEALLVM_SOURCE_DIR}/tools)
  set(LLVM_SEAHORN_LIBS SeaInstCombine SeaLoops)
else()
  message (WARNING "llvm-seahorn is required by clam but not found in ${SEALLVM_SOURCE_DIR}. "
                   "Run\n\tcmake --build . --target clam-seallvm && cmake ${CMAKE_SOURCE_DIR}")
  return()
endif()


## clam and crab
if(IS_DIRECTORY ${CLAM_SOURCE_DIR} AND EXISTS ${CLAM_SOURCE_DIR}/CMakeLists.txt)
  set(HAVE_LLVM_SEAHORN ON CACHE BOOL "Enable llvm-seahorn in clam")
  set(CLAM_INCLUDE_ALL_DOMAINS OFF CACHE BOOL "Exclude all clam abstract domains")
  add_subdirectory(${CLAM_SOURCE_DIR})
  include_directories(BEFORE ${CLAM_INCLUDE_DIRS})
  # these components are not included if clam is a subproject
  add_subdirectory(${CLAM_SOURCE_DIR}/lib/Transforms)
  set(CLAM_LIBS LlvmPasses ${CLAM_LIBS})
  add_subdirectory(${CLAM_SOURCE_DIR}/tools/clam-pp)
else()
  message (WARNING "Clam not found. "
                   "Run \n\tcmake --build . --target ntt-clam && cmake ${CMAKE_SOURCE_DIR}")
  return ()
endif()


include_directories(include)
add_subdirectory(src)
