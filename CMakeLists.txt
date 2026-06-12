cmake_minimum_required(VERSION 3.20)
project(MySanitizedCProject LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

file(GLOB SOURCES "HITCPU/HITCPUCustLang/*.c")

add_executable(lang ${SOURCES})
    
# Apply settings only when the build type is Debug
target_compile_options(lang PRIVATE 
    $<$<CONFIG:Debug>:-Wall -Wextra -g -fsanitize=address -fno-omit-frame-pointer>
    $<$<CONFIG:VSDebug>:-march=native -g> 
    $<$<CONFIG:Release>:-O3 -march=native -g> 
)

target_link_options(lang PRIVATE 
    $<$<CONFIG:Debug>:-fsanitize=address>
)