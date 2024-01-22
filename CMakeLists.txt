cmake_minimum_required(VERSION 3.2)

# Definitions
set(PROJECT_NAME tbi)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

#project(cmake_c_template)
project(${PROJECT_NAME})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

# Include paths
set(INCLUDE_DIRS ${INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/lib ${PROJECT_SOURCE_DIR}/generated)

# Assign the include directories
include_directories(${INCLUDE_DIRS})

# Find library sources
file(GLOB_RECURSE LIB_SRC_FILES lib/*.c)

# Build static/shared library
add_library(${PROJECT_NAME} STATIC ${LIB_SRC_FILES})

# Install library
install(TARGETS ${PROJECT_NAME} DESTINATION lib)

# Install library headers
file(GLOB_RECURSE LIB_INC_FILES lib/include/*.h)
install(FILES ${LIB_INC_FILES} DESTINATION include)

# Build executables
set(CMAKE_BUILD_TYPE Debug)
add_executable(tbi_client client.c)
target_link_libraries(tbi_client ${PROJECT_NAME})

add_executable(tbi_server server.c)
target_link_libraries(tbi_server ${PROJECT_NAME})

add_executable(test_bitmagic test/test_bitmagic.c)
target_link_libraries(test_bitmagic ${PROJECT_NAME})
