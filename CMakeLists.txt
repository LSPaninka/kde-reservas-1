cmake_minimum_required(VERSION 3.16)
project(kde-reservas LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 5.15 REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
    QuickControls2
    Sql
    Network
)

set(SOURCES
    src/main.cpp
    src/database.cpp
    src/reservationmodel.cpp
    src/restserver.cpp
)

set(HEADERS
    src/database.h
    src/reservationmodel.h
    src/restserver.h
)

set(RESOURCES
    qml/qml.qrc
)

add_executable(kde-reservas
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

target_link_libraries(kde-reservas PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Qml
    Qt5::Quick
    Qt5::QuickControls2
    Qt5::Sql
    Qt5::Network
)

install(TARGETS kde-reservas DESTINATION bin)
