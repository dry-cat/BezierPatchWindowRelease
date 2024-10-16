cmake_minimum_required(VERSION 3.10)

project(testLibrary)

add_executable(testLibrary
    main.cpp
    Homogeneous4.cpp
    Matrix4.cpp
    Point3.cpp
    Quaternion.cpp
    Vector3.cpp
)

target_compile_options(testLibrary PUBLIC -Wall -O2 -Wall -Wextra -D_REENTRANT)

include(CMakePrintHelpers)
cmake_print_properties(TARGETS testLibrary PROPERTIES COMPILE_OPTIONS)
