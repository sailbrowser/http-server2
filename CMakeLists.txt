cmake_minimum_required(VERSION 2.8)

project(final)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_POSIX_C_SOURCE=200809L")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -o2")
# set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

set (LIBS "-pthread")

aux_source_directory(src SOURCES)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fpermissive")

add_executable(final ${SOURCES})

target_link_libraries(${PROJECT_NAME} ${LIBS})
