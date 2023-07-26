cmake_minimum_required(VERSION 3.25)

set(HAVE_FLAG_SEARCH_PATHS_FIRST 0)

set(
    ARMPP_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/system.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uart.cpp
)
set(
    ARMPP_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(armpp STATIC ${ARMPP_SRC})
set_target_properties(
    armpp PROPERTIES 
    CXX_STANDARD 20
)
target_include_directories(
    armpp PUBLIC
    ${ARMPP_INCLUDE_DIR}
)
target_link_libraries(armpp stdc++)
