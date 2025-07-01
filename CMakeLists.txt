cmake_minimum_required(VERSION 3.29)
project(Final2)

set(CMAKE_CXX_STANDARD 20)

add_executable(Final2
        main.cpp
        TransitData.cpp
        Router.cpp
)

include_directories(/opt/homebrew/opt/curl/include)
link_directories(/opt/homebrew/opt/curl/lib)
target_link_libraries(Final2 curl)
