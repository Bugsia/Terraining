# CMakeList.txt : CMake project for Terraining, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(DATA_SOURCE_DIR ${CMAKE_SOURCE_DIR}/data)
set(DATA_DEST_DIR ${CMAKE_BINARY_DIR}/)
file(COPY ${DATA_SOURCE_DIR} DESTINATION ${DATA_DEST_DIR})

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(CMAKE_CXX_STANDARD 20)   # Set C++ standard to C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # Ensure compiler enforces C++17

project ("Terraining")

add_subdirectory(raylib-master)

# Add include folder
include_directories(${CMAKE_SOURCE_DIR}/inc)
include_directories(${CMAKE_SOURCE_DIR}/raylib-master/build/raylib/include)
include_directories(${CMAKE_SOURCE_DIR}/rlImGui/inc)
include_directories(${CMAKE_SOURCE_DIR}/imgui/inc)
include_directories(${CMAKE_SOURCE_DIR}/raylibBackend/inc)

# raylibBackend
file(GLOB_RECURSE BACKEND_SOURCE_HEADER_FILES
    ${CMAKE_SOURCE_DIR}/raylibBackend/src/*.cpp
    ${CMAKE_SOURCE_DIR}/raylibBackend/inc/*.h
)

message(STATUS "SOURCE_HEADER_FILES: ${BACKEND_SOURCE_HEADER_FILES}")

# Add the shared library target
add_library(raylibBackend STATIC ${BACKEND_SOURCE_HEADER_FILES})

# Terraining.exe
file(GLOB_RECURSE TERRAINING_SOURCE_HEADER_FILES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/inc/*.h
)

message(STATUS "SOURCE_HEADER_FILES: ${TERRAINING_SOURCE_HEADER_FILES}")

# Add source to this project's executable.
add_executable (Terraining ${TERRAINING_SOURCE_HEADER_FILES})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Terraining PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
# Collect all precompiled libraries in the lib directory
file(GLOB_RECURSE PRECOMPILED_LIBS
    ${CMAKE_SOURCE_DIR}/*.lib
)

# Link with all precompiled libraries
target_link_libraries(Terraining ${PRECOMPILED_LIBS})
target_link_libraries(Terraining raylibBackend)
target_link_libraries(Terraining WinMM) # For raylib