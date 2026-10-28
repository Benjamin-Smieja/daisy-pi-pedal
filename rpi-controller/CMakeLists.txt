cmake_minimum_required(VERSION 3.16)
project(QtClangHello LANGUAGES CXX)

# Force standard C++ version
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Crucial for Qt: Auto-run moc, uic, and rcc tools
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find the Qt6 package components
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Define your executable target
add_executable(rpi-ds-gui 
rpi-ds-gui.cpp
daisyComsManager.cpp
)

# Link your app against Qt Widgets
target_link_libraries(rpi-ds-gui PRIVATE Qt6::Widgets)
