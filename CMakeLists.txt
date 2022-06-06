cmake_minimum_required(VERSION 3.0)

project(SIK_Bomberman VERSION 1.0 LANGUAGES CXX)

set_property(GLOBAL PROPERTY RULE_MESSAGE OFF)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

## To get rid of -Wshadow and -pedantic once the project's done
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wshadow -Wconversion -Werror -pedantic -O2")

set(SOURCE_FILES
    src/server.cpp
    src/random.cpp
    src/network/socket.cpp
)

add_executable(robots-server ${SOURCE_FILES})

target_include_directories(robots-server PRIVATE include)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(robots-server PRIVATE Threads::Threads)
target_link_libraries(robots-server PRIVATE -latomic)

# find_package(Boost 1.40 REQUIRED)
# target_link_libraries(robots-server PRIVATE Boost)
