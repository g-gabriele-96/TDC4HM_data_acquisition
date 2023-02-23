cmake_minimum_required(VERSION 3.5)

project(Acquisition VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
#        mainwindow.cpp
#        mainwindow.h
#        mainwindow.ui
)

set(XTDC4_DIR "C:/Users/BaLiToGo/Desktop/TDC Acquisition software/TDCAcquisition/")

add_library(xtdc4_driver_64::xtdc4_driver_64 SHARED IMPORTED)

add_executable(TDCAcquisition ${PROJECT_SOURCES})
target_link_libraries(TDCAcquisition PRIVATE Qt${QT_VERSION_MAJOR}::Core xtdc4_driver_64::xtdc4_driver_64)
target_include_directories(TDCAcquisition PUBLIC ${XTDC4_DIR})

set_target_properties(
  xtdc4_driver_64::xtdc4_driver_64
  PROPERTIES
  IMPORTED_IMPLIB "${XTDC4_DIR}/xtdc4_driver_64.lib"
  IMPORTED_LOCATION "${XTDC4_DIR}/xtdc4_driver_64.dll"
  IMPORTED_NO_SONAME "TRUE"
)

add_executable(simple_read simple_read_TDC4HM.cpp)
target_include_directories(simple_read PUBLIC ${XTDC4_DIR})
target_link_libraries(simple_read PRIVATE xtdc4_driver_64::xtdc4_driver_64 Qt${QT_VERSION_MAJOR}::Core)

install(TARGETS TDCAcquisition
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(TDCAcquisition)
endif()
