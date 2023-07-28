set(HAVE_FLAG_SEARCH_PATHS_FIRST 0)

set(TARGET_TRIPLET arm-none-eabi)

find_program(
    ARM_C_COMPILER 
    NAMES ${TARGET_TRIPLET}-gcc
    HINTS /Applications/ARM/bin
    REQUIRED
)
find_program(
    ARM_CXX_COMPILER 
    NAMES ${TARGET_TRIPLET}-g++ 
    HINTS /Applications/ARM/bin
    REQUIRED
)
find_program(
    ARM_OBJCOPY
    NAMES ${TARGET_TRIPLET}-objcopy
    HINTS /Applications/ARM/bin
    REQUIRED
)

find_program(
    ARM_OBJ_SIZE
    NAMES ${TARGET_TRIPLET}-size
    HINTS /Applications/ARM/bin
    REQUIRED
)

# TODO if mac os
execute_process(COMMAND ${ARM_CXX_COMPILER} -print-sysroot OUTPUT_VARIABLE ARM_SYSROOT OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "Sys root ${ARM_SYSROOT}")
set(CMAKE_OSX_SYSROOT ${ARM_SYSROOT})


set(CMAKE_C_COMPILER ${ARM_C_COMPILER})
set(CMAKE_CXX_COMPILER ${ARM_CXX_COMPILER})
set(CMAKE_ASM_COMPILER ${ARM_C_COMPILER})

add_compile_options(
    #-std=c++20 
    -Os 
    -mcpu=${TARGET_PROCESSOR} 
    -mthumb 
    -mfloat-abi=soft 
    -fmessage-length=0 
    -fsigned-char 
    -ffunction-sections 
    -fdata-sections 
    -g3
    -fno-rtti
    -fno-exceptions
    -Wno-volatile # C++20 deprecated volatile stuff
    -Wall
    )


# Don't run the linker on compiler check
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_EXECUTABLE_SUFFIX .elf)
set(CMAKE_EXECUTABLE_SUFFIX_C .elf)
set(CMAKE_EXECUTABLE_SUFFIX_CXX .elf)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Gowin magic 
add_compile_definitions(__STARTUP_CLEAR_BSS)

enable_language(C CXX ASM)

set(ARMPP_SYSTEM_FREQUENCY 54_MHz)
