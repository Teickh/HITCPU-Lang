# ==============================================================================
# HOW TO RUN THIS PROJECT (Using Ninja)
# ==============================================================================
#
# 1. Configure the project with Ninja (choose your config):
#    cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug      # With AddressSanitizer
#    cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release    # Optimized build
#
# 2. Build the executable using Ninja:
#    cmake --build build
#
# Note: To target only the custom language or assembly add
# --target assembly
# or
# --target lang
#
# 3. Run the compiled executable:
#    ./build/lang
#    or
#    ./build/assembly
#
# Note: If you get a "Ninja not found" error, make sure it's installed:
#       - macOS: brew install ninja
#       - Ubuntu/Debian: sudo apt install ninja-build
# ==============================================================================

cmake_minimum_required(VERSION 3.20)
project(MySanitizedCProject LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ------------------------------------------------------------------------------
# 1. Custom Language Target
# ------------------------------------------------------------------------------
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

# ------------------------------------------------------------------------------
# 2. Assembler Target
# ------------------------------------------------------------------------------
file(GLOB ASSEM_SOURCES "HITCPU/HITCPUAssem/*.c")
add_executable(assembly ${ASSEM_SOURCES}) # Creates a separate binary named 'assem'

# Reuse the exact same compiler and linker flag logic for the assembler
target_compile_options(assembly PRIVATE 
    $<$<CONFIG:Debug>:-Wall -Wextra -g -fsanitize=address -fno-omit-frame-pointer>
    $<$<CONFIG:VSDebug>:-march=native -g> 
    $<$<CONFIG:Release>:-O3 -march=native -g> 
)

target_link_options(assembly PRIVATE 
    $<$<CONFIG:Debug>:-fsanitize=address>
)