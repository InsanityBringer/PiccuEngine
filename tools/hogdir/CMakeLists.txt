cmake_minimum_required(VERSION 3.11)

add_executable(hogdir hogdir.cpp)
#Need C++17 for <filesystem>
set_target_properties(hogdir PROPERTIES CXX_STANDARD 17)
