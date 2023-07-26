# Add target for firmware
# Params:
#   TARGET_NAME     the name of target
#   LINKER_SCRIPT   script for linking the target
#   SOURCES         source files for the target
#   INCLUDE_DIRS    include directories
#   LINK_TARGETS    link libraries that were created with add_library
#   LINK_LIBRARIES  link libraries
function(add_firmware TARGET_NAME)
    set(options)
    set(one_val_options LINKER_SCRIPT)
    set(multi_val_options SOURCES INCLUDE_DIRS LINK_LIBRARIES LINK_TARGETS)
    cmake_parse_arguments(ADD_FIRMWARE "${options}" "${one_val_options}" "${multi_val_options}" ${ARGN})

    message(STATUS "Configure firmware ${TARGET_NAME}${CMAKE_EXECUTABLE_SUFFIX}\n     linker script ${ADD_FIRMWARE_LINKER_SCRIPT}\n     link targets ${ADD_FIRMWARE_LINK_TARGETS}")

    add_executable(${TARGET_NAME} ${ADD_FIRMWARE_SOURCES})
    add_custom_command(
        TARGET ${TARGET_NAME}
        POST_BUILD
        COMMAND ${ARM_OBJ_SIZE} 
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}${CMAKE_EXECUTABLE_SUFFIX_CXX}
        COMMENT "ELF size:"
    )
    add_custom_command(
        TARGET ${TARGET_NAME}
        POST_BUILD
        COMMAND ${ARM_OBJCOPY} 
            -O binary 
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}${CMAKE_EXECUTABLE_SUFFIX_CXX} 
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET_NAME}.bin
        COMMENT "Generating binary file ${TARGET_NAME}.bin"
        BYPRODUCTS ${TARGET_NAME}.bin
    )
    
    set_target_properties(
        ${TARGET_NAME} PROPERTIES 
        CXX_STANDARD 20
    )

    if (ADD_FIRMWARE_INCLUDE_DIRS)
        target_include_directories(${TARGET_NAME} PRIVATE ${ADD_FIRMWARE_INCLUDE_DIRS})
    endif()

    target_link_options(
        ${TARGET_NAME}
        PRIVATE
        -mthumb
        -nodefaultlibs
        -Xlinker
        --gc-sections
        -Wl,-Map,${TARGET_NAME}.map
        --specs=nano.specs
    )

    if (ADD_FIRMWARE_LINKER_SCRIPT)
        set_target_properties(${TARGET_NAME} PROPERTIES LINK_DEPENDS ${ADD_FIRMWARE_LINKER_SCRIPT})
        target_link_options(
            ${TARGET_NAME}
            PRIVATE
            -T ${ADD_FIRMWARE_LINKER_SCRIPT}
        )
    endif()

    if (ADD_FIRMWARE_LINK_LIBRARIES)
        target_link_libraries(${TARGET_NAME} ${ADD_FIRMWARE_LINK_LIBRARIES})
    endif()

    if (ADD_FIRMWARE_LINK_TARGETS)
        target_link_libraries(${TARGET_NAME} ${ADD_FIRMWARE_LINK_TARGETS})
        foreach(LINK_TARGET ${ADD_FIRMWARE_LINK_TARGETS})
            message(STATUS "Get includes for ${LINK_TARGET}")
            get_target_property(includes ${LINK_TARGET} INCLUDE_DIRECTORIES)
            message(STATUS "Include dirs for ${LINK_TARGET} ${includes}")
            if (includes)
            endif()
        endforeach()
    endif()
endfunction()
