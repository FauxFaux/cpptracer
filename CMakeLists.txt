cmake_minimum_required(VERSION 2.8)
project(cpptracer)

add_executable(cpptracer
        color.cpp
        cppraytracer.cpp
        intersectioninfo.cpp
        pixel.cpp
        ray.cpp
        rtlight.cpp
        rtobject.cpp
        rtsphere.cpp
        stdafx.cpp
        v3.cpp
        )

target_link_libraries(cpptracer
        boost_thread
        boost_system
        )

