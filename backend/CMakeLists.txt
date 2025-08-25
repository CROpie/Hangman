cmake_minimum_required(VERSION 3.10)
project(HangmanBackend)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

add_executable(hangmanbackend ./src/main.cpp)

target_include_directories(hangmanbackend PRIVATE
    ${CMAKE_SOURCE_DIR}/libs/json
    ${CMAKE_SOURCE_DIR}/include
)

find_package(Threads REQUIRED)
target_link_libraries(hangmanbackend PRIVATE Threads::Threads)

# Find OpenSSL
find_package(OpenSSL REQUIRED)

# Link OpenSSL
target_link_libraries(hangmanbackend PRIVATE OpenSSL::Crypto)