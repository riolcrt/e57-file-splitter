cmake_minimum_required(VERSION 3.14)
project(E57Splitter VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find and include libE57Format
add_subdirectory(libE57Format)

# Add the executable
add_executable(e57splitter src/main.cpp)

# Link against libE57Format
target_link_libraries(e57splitter PRIVATE E57Format)

# Include directories
target_include_directories(e57splitter PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/libE57Format/include
)

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Install
install(TARGETS e57splitter
    RUNTIME DESTINATION bin
)
