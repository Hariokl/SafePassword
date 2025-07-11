cmake_minimum_required(VERSION 3.10)
project(NoteBook)
set(CMAKE_CXX_STANDARD 17)

# SDL2
include_directories(${CMAKE_SOURCE_DIR}/libs/SDL2/include)
link_directories(${CMAKE_SOURCE_DIR}/libs/SDL2/lib/x64)

# SDL2_ttf
include_directories(${CMAKE_SOURCE_DIR}/libs/SDL2_ttf/include)
link_directories(${CMAKE_SOURCE_DIR}/libs/SDL2_ttf/lib/x64)

# Add your executable
add_executable(NoteBook src/main.cpp)


# Link libraries
target_link_libraries(NoteBook SDL2 SDL2main SDL2_ttf)

# Removes console
set_target_properties(NoteBook PROPERTIES WIN32_EXECUTABLE TRUE)
