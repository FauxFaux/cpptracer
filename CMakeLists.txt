cmake_minimum_required(VERSION 3.0)
project(cpptracer)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -mavx")
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

set(GTEST_ROOT gtest)
find_package(GTest REQUIRED)

add_executable(cpptracer
        main.cpp
        tracer.cpp
        bitmap.cpp
        )

set(TRACER_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})

target_link_libraries(cpptracer
        ${TRACER_LIBRARIES}
        )

target_compile_features(cpptracer PUBLIC
        cxx_range_for
        )


if (GTEST_FOUND)
    enable_testing()
    add_executable(test_tracer
            test_tracer.cpp
            tracer.cpp)
    target_link_libraries(test_tracer
            ${GTEST_BOTH_LIBRARIES}
            ${CMAKE_THREAD_LIBS_INIT}
            ${TRACER_LIBRARIES}
            )
    target_compile_features(test_tracer PUBLIC
            cxx_range_for
            )
    gtest_add_tests(
            test_tracer "" test_tracer.cpp
    )
    configure_file(ref-320x240.ppm ref-320x240.ppm COPYONLY)
else ()
    message("Couldn't find Google Test library, so not doing testing stuff")
endif ()
