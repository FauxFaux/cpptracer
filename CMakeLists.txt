cmake_minimum_required(VERSION 2.8)
project(cpptracer)

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
