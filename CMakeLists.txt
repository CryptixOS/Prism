cmake_minimum_required(VERSION 3.25)
project(Prism VERSION 0.1)

cmake_policy(SET CMP0092 NEW)

set(CMAKE_CXX_STANDARD 23)
set(CXX_STANDARD_REQUIRED ON)

add_subdirectory(fmt)
add_subdirectory(subprojects/magic_enum)

set (SOURCES
    Source/Prism/Algorithm/Hash.cpp

    Source/Prism/Containers/Bitmap.cpp
    Source/Prism/Containers/RingBuffer.cpp

    Source/Prism/Debug/Assertions.cpp
    Source/Prism/Debug/Log.cpp
    Source/Prism/Debug/Logger.cpp
    Source/Prism/Debug/LogSink.cpp
    Source/Prism/Debug/Ubsan.cpp
    Source/Prism/Debug/Stacktrace.cpp

    Source/Prism/Memory/Memory.cpp
    Source/Prism/String/Printf.cpp

    Source/Prism/Utility/Path.cpp
    Source/Prism/Utility/PathView.cpp
    Source/Prism/Utility/Path.cpp
    subprojects/demangler/source/demangler.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC Source subprojects/demangler/include)
target_compile_definitions(${PROJECT_NAME} PUBLIC -DFMT_STATIC_THOUSANDS_SEPARATOR=1
    -DFMT_USE_LONG_DOUBLE=0
    -DFMT_USE_DOUBLE=0
    -DFMT_USE_FLOAT=0
    -DFMT_HEADER_ONLY
)
target_link_libraries(${PROJECT_NAME} PUBLIC fmt magic_enum::magic_enum)
target_include_directories(${PROJECT_NAME} PUBLIC subprojects/magic_enum/include)
