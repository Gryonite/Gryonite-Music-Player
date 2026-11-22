cmake_minimum_required(VERSION 3.21)

project(Gryonite VERSION 0.6.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "C:/Qt/6.12.0/mingw_64")

find_package(Qt6 REQUIRED COMPONENTS
    Widgets
    Multimedia
    Sql
)

qt_add_executable(Gryonite WIN32
    main.cpp
    gryonite.rc
)

target_link_libraries(Gryonite PRIVATE
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::Sql
)