cmake_minimum_required(VERSION 3.20)
project(MySanitizedCProject LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(lang
    HITCPU/HITCPUCustLang/lang.c
    HITCPU/HITCPUCustLang/lexer.c
    HITCPU/HITCPUCustLang/parser.c
    HITCPU/HITCPUCustLang/semantic_analysis.c
    HITCPU/HITCPUCustLang/code_gen.c
)
    
# Apply settings only when the build type is Debug
target_compile_options(lang PRIVATE 
    $<$<CONFIG:Debug>:-Wall -Wextra -g -fsanitize=address -fno-omit-frame-pointer>
    # Added -g to Release so you can still debug it!
    $<$<CONFIG:Release>:-O3 -march=native -g> 
)

target_link_options(lang PRIVATE 
    $<$<CONFIG:Debug>:-fsanitize=address>
)