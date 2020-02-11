cmake_minimum_required(VERSION 2.8)

project(final)

add_definitions(-DASIO_STANDALONE)

set(IncludeDirectories
    asio-1.12.2/include
)

set(Includes
    httpparser.hpp
)

set(Sources
    main.cpp
)

set(CMAKE_CXX_FLAGS -pthread)
add_executable(${PROJECT_NAME} ${Includes} ${Sources})
target_include_directories(${PROJECT_NAME} PRIVATE  ${IncludeDirectories})

