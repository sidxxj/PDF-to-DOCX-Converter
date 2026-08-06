cmake_minimum_required(VERSION 3.16)
project(FormatConverter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

add_executable(FormatConverter
    main.cpp
    MainWindow.cpp
    MainWindow.h
    ConversionWorker.h
)

target_link_libraries(FormatConverter Qt6::Core Qt6::Gui Qt6::Widgets)