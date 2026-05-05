# Auto-generated deploy QML imports script for target "appQT456TDirect".
# Do not edit, all changes will be lost.
# This file should only be included by qt6_deploy_qml_imports().

set(__qt_opts )
if(arg_NO_QT_IMPORTS)
    list(APPEND __qt_opts NO_QT_IMPORTS)
endif()

_qt_internal_deploy_qml_imports_for_target(
    ${__qt_opts}
    IMPORTS_FILE "D:/zhijain/QT456TDirect/build/Desktop_Qt_6_8_3_MSVC2022_64bit-Debug/.qt/qml_imports/appQT456TDirect_build.cmake"
    PLUGINS_FOUND __qt_internal_plugins_found
    QML_DIR     "${arg_QML_DIR}"
    PLUGINS_DIR "${arg_PLUGINS_DIR}"
)

if(arg_PLUGINS_FOUND)
    set(${arg_PLUGINS_FOUND} "${__qt_internal_plugins_found}" PARENT_SCOPE)
endif()
