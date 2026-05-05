cmake_minimum_required(VERSION 3.16)

project(QT456TDirect VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appQT456TDirect
    main.cpp
)

qt_add_qml_module(appQT456TDirect
    URI QT456TDirect
    QML_FILES
        Main.qml
SOURCES third_party/HikCamera/includes/CameraParams.h third_party/HikCamera/includes/MvCameraControl.h third_party/HikCamera/includes/MvErrorDefine.h third_party/HikCamera/includes/MvISPErrorDefine.h third_party/HikCamera/includes/MvObsoleteInterfaces.h third_party/HikCamera/includes/MvSdkExport.h third_party/HikCamera/includes/ObsoleteCamParams.h third_party/HikCamera/includes/PixelType.h third_party/SPSC/atomicops.h third_party/SPSC/readerwritercircularbuffer.h third_party/SPSC/readerwriterqueue.h
RESOURCES third_party/HikCamera/libs/MvCameraControl.lib
SOURCES src/CameraParamsWrapper.h src/HikCamera.cpp src/HikCamera.h src/HikSdkRuntime.h src/HikSdkWrapper.h src/RoiData.cpp src/RoiData.h src/RoiManager.cpp src/RoiManager.h
SOURCES src/CameraViewModel.h
SOURCES src/CameraViewModel.cpp
SOURCES src/CameraFrameItem.h
SOURCES src/CameraFrameItem.cpp
SOURCES src/CaptureWorker.h
SOURCES src/ProcessWorker.h
SOURCES src/CaptureWorker.cpp
SOURCES src/ProcessWorker.cpp
SOURCES src/MainViewWindow.h
SOURCES src/MainViewWindow.cpp
SOURCES src/Algorithm.h

SOURCES src/Algorithm.cpp
#QML_FILES qml/components/CameraTile.qml
#QML_FILES qml/pages/DashboardPage.qml
QML_FILES qml/theme/theme.qml
QML_FILES qml/components/CameraTile.qml qml/pages/DashboardPage.qml
QML_FILES qml/components/RoiItem.qml
QML_FILES qml/components/ImageCanvas.qml
QML_FILES qml/components/RoiEditorWindow.qml
SOURCES
SOURCES
SOURCES src/ConfigReader.h
SOURCES src/CameraConfig.h
SOURCES src/ConfigReader.cpp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appQT456TDirect PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appQT456TDirect
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)
target_include_directories(appQT456TDirect PRIVATE
    ${CMAKE_SOURCE_DIR}/third_party/HikCamera/includes
    ${CMAKE_SOURCE_DIR}/third_party/halcon/includes
    ${CMAKE_SOURCE_DIR}/third_party/halcon/includes/hlib
    ${CMAKE_SOURCE_DIR}/third_party/halcon/includes/halconc
    ${CMAKE_SOURCE_DIR}/third_party/halcon/includes/halconcpp
    ${CMAKE_SOURCE_DIR}/third_party/halcon/includes/hclib
     ${CMAKE_SOURCE_DIR}/third_party/halcon/includes/hdevengine
    ${CMAKE_SOURCE_DIR}/third_party/SPSC
    ${CMAKE_SOURCE_DIR}/third_party/RM
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(appQT456TDirect
    PRIVATE Qt6::Quick
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/HikCamera/libs/MvCameraControl.lib
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/halcon/libs/halconc.lib
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/halcon/libs/halconcpp.lib
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/halcon/libs/halcon.lib

)

include(GNUInstallDirs)
install(TARGETS appQT456TDirect
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
