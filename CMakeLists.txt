# project specific logic here.
#
cmake_minimum_required(VERSION 3.28)
set(CMAKE_CXX_STANDARD 20)

project("industry")

option(ENABLE_PROFILER "Enable Tracy Profiling" ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(lc_client)
add_library(stb INTERFACE)

add_subdirectory("res")
add_subdirectory("lc_client")
add_subdirectory("libs")

add_subdirectory("game")

set(CMAKE_CXX_SCAN_FOR_MODULES true)

target_include_directories(game PRIVATE lc_client)
target_link_libraries(game PUBLIC lc_client)

if(ENABLE_PROFILER)
  add_subdirectory(external/tracy)
  target_link_libraries(game PRIVATE Tracy::TracyClient)
  target_compile_definitions(game PRIVATE TRACY_ENABLE)
endif()

target_compile_options(lc_client PRIVATE $<$<CONFIG:Debug>:-g>
                                         $<$<CONFIG:Debug>:-O0>)
target_compile_options(game PRIVATE $<$<CONFIG:Debug>:-g>
                                    $<$<CONFIG:Debug>:-O0>)
target_compile_options(game_exe PRIVATE $<$<CONFIG:Debug>:-g>
                                        $<$<CONFIG:Debug>:-O0>)

target_compile_definitions(lc_client PRIVATE $<$<CONFIG:Debug>:LE_DEBUG>)
