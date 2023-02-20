cmake_minimum_required(VERSION 3.5)

project(TDCAcquisition VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(TDCAcquisition
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET TDCAcquisition APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(TDCAcquisition SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(TDCAcquisition
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(TDCAcquisition PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(TDCAcquisition PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

set(XTDC4_DIR "C:/Users/BaLiToGo/Desktop/TDC Acquisition software/TDCAcquisition/")

#ExternalProject_Add(extProject
#    BUILD_BYPRODUCTS ${XTDC4_DIR}/xtdc4_driver_64.lib
#)
add_library(xtdc4_driver_64::xtdc4_driver_64 SHARED IMPORTED)
#add_dependencies(anewlib::anewlib extProject)

set_target_properties(
  xtdc4_driver_64::xtdc4_driver_64
  PROPERTIES
  IMPORTED_IMPLIB "${XTDC4_DIR}/xtdc4_driver_64.lib"
  IMPORTED_LOCATION "${XTDC4_DIR}/xtdc4_driver_64.dll"
  IMPORTED_NO_SONAME "TRUE"
)

add_executable(simple_read simple_read_TDC4HM.cpp)
target_include_directories(simple_read PUBLIC ${XTDC4_DIR})
target_link_libraries(simple_read PRIVATE xtdc4_driver_64::xtdc4_driver_64)


install(TARGETS TDCAcquisition
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(TDCAcquisition)
endif()
