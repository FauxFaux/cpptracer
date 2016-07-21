cmake_minimum_required(VERSION 3.0)
project(cpptracer)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} \
 -march=native \
 -fexpensive-optimizations \
 -ftree-loop-im \
 -fivopts \
 -ftree-loop-linear \
 -fipa-matrix-reorg \
 -ftracer \
 -fweb \
")

find_package(Threads)

add_executable(cpptracer
        main.cpp
        tracer.cpp
        bitmap.cpp
        )

target_link_libraries(cpptracer
        ${CMAKE_THREAD_LIBS_INIT}
        )

target_compile_features(cpptracer PUBLIC
        cxx_range_for
        )
