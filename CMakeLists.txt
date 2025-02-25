cmake_minimum_required(VERSION 3.15)

# Configure vcpkg toolchain
set(VCPKG_DIR $ENV{VCPKG_DIR})
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake")

if(MSVC)
    set(VCPKG_CRT_LINKAGE static)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

project("ScoreGen")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Locate dependencies
find_package(Aubio CONFIG REQUIRED)
find_package(portaudio CONFIG REQUIRED)
find_package(SndFile REQUIRED)
find_package(FFTW3 REQUIRED)
find_package(GTest CONFIG REQUIRED)

# Setup non-vcpkg dependency for libmusicxml.
set(LIBMUSICXML_ROOT "${CMAKE_SOURCE_DIR}/libs/libmusicxml")
if(APPLE)
    set(LIBMUSICXML_STATIC_PATH "${LIBMUSICXML_ROOT}/lib/macos/libmusicxml2.a")
else()
    set(LIBMUSICXML_STATIC_PATH "${LIBMUSICXML_ROOT}/lib/win64/libmusicxml.lib")
    set(LIBMUSICXML_SHARED_PATH "${LIBMUSICXML_ROOT}/lib/win64/libmusicxml.dll")
    set(LIBMUSICXML_DEST "$<TARGET_FILE_DIR:ScoreGen>/libmusicxml.dll")

# Declare an imported target for libmusicxml.
add_library(libmusicxml SHARED IMPORTED)
set_target_properties(libmusicxml PROPERTIES
    IMPORTED_IMPLIB ${LIBMUSICXML_STATIC_PATH}
    IMPORTED_LOCATION ${LIBMUSICXML_SHARED_PATH}
)
endif()

# --------------------------------------------------
# Create Core Library Target
# --------------------------------------------------
# Get backend source files.
file(GLOB CORE_SOURCES "src/backend/*.cpp")
list(REMOVE_ITEM CORE_SOURCES "${CMAKE_SOURCE_DIR}/src/backend/ScoreGen.cpp")

# Create a static library containing the core functionality.
add_library(ScoreGenLib STATIC ${CORE_SOURCES})

# Include directories for the core library.
target_include_directories(ScoreGenLib PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${LIBMUSICXML_ROOT}/include
)

# Link dependencies into the core library.
target_link_libraries(ScoreGenLib PUBLIC 
    Aubio::aubio
    SndFile::sndfile
    portaudio_static
    ${LIBMUSICXML_STATIC_PATH}
    FFTW3::fftw3
)

# --------------------------------------------------
# Create Main Application Executable
# --------------------------------------------------
add_executable(ScoreGen src/backend/ScoreGen.cpp)
target_link_libraries(ScoreGen PRIVATE ScoreGenLib)

# Platform-specific linking
if(APPLE)
    target_link_libraries(ScoreGen PRIVATE "-framework CoreAudio" "-framework CoreFoundation")
endif()

# --------------------------------------------------
# Create Test Executable for GTest
# --------------------------------------------------
# Get test source files.
file(GLOB GTEST_SOURCES "test/gtest/*.cpp")

# Create the test executable.
add_executable(GTests ${GTEST_SOURCES})
target_link_libraries(GTests PRIVATE 
    ScoreGenLib
    GTest::gtest_main
)

# Use CTest.
# include(GoogleTest)
# gtest_discover_tests(GTests)