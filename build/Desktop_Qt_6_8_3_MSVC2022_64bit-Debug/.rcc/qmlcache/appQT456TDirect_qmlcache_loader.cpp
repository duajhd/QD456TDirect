#include <QtQml/qqmlprivate.h>
#include <QtCore/qdir.h>
#include <QtCore/qurl.h>
#include <QtCore/qhash.h>
#include <QtCore/qstring.h>

namespace QmlCacheGeneratedCode {
namespace _qt_qml_QT456TDirect_Main_qml { 
    extern const unsigned char qmlData[];
    extern const QQmlPrivate::AOTCompiledFunction aotBuiltFunctions[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), &aotBuiltFunctions[0], nullptr
    };
}
namespace _qt_qml_QT456TDirect_qml_theme_theme_qml { 
    extern const unsigned char qmlData[];
    extern const QQmlPrivate::AOTCompiledFunction aotBuiltFunctions[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), &aotBuiltFunctions[0], nullptr
    };
}
namespace _qt_qml_QT456TDirect_qml_components_CameraTile_qml { 
    extern const unsigned char qmlData[];
    extern const QQmlPrivate::AOTCompiledFunction aotBuiltFunctions[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), &aotBuiltFunctions[0], nullptr
    };
}
namespace _qt_qml_QT456TDirect_qml_pages_DashboardPage_qml { 
    extern const unsigned char qmlData[];
    extern const QQmlPrivate::AOTCompiledFunction aotBuiltFunctions[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), &aotBuiltFunctions[0], nullptr
    };
}
namespace _qt_qml_QT456TDirect_qml_pages_RoiEditorPage_qml { 
    extern const unsigned char qmlData[];
    extern const QQmlPrivate::AOTCompiledFunction aotBuiltFunctions[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), &aotBuiltFunctions[0], nullptr
    };
}
namespace _qt_qml_QT456TDirect_qml_components_RoiItem_qml { 
    extern const unsigned char qmlData[];
    extern const QQmlPrivate::AOTCompiledFunction aotBuiltFunctions[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), &aotBuiltFunctions[0], nullptr
    };
}
namespace _qt_qml_QT456TDirect_qml_components_ImageCanvas_qml { 
    extern const unsigned char qmlData[];
    extern const QQmlPrivate::AOTCompiledFunction aotBuiltFunctions[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), &aotBuiltFunctions[0], nullptr
    };
}
namespace _qt_qml_QT456TDirect_qml_components_RoiEditorWindow_qml { 
    extern const unsigned char qmlData[];
    extern const QQmlPrivate::AOTCompiledFunction aotBuiltFunctions[];
    const QQmlPrivate::CachedQmlUnit unit = {
        reinterpret_cast<const QV4::CompiledData::Unit*>(&qmlData), &aotBuiltFunctions[0], nullptr
    };
}

}
namespace {
struct Registry {
    Registry();
    ~Registry();
    QHash<QString, const QQmlPrivate::CachedQmlUnit*> resourcePathToCachedUnit;
    static const QQmlPrivate::CachedQmlUnit *lookupCachedUnit(const QUrl &url);
};

Q_GLOBAL_STATIC(Registry, unitRegistry)


Registry::Registry() {
    resourcePathToCachedUnit.insert(QStringLiteral("/qt/qml/QT456TDirect/Main.qml"), &QmlCacheGeneratedCode::_qt_qml_QT456TDirect_Main_qml::unit);
    resourcePathToCachedUnit.insert(QStringLiteral("/qt/qml/QT456TDirect/qml/theme/theme.qml"), &QmlCacheGeneratedCode::_qt_qml_QT456TDirect_qml_theme_theme_qml::unit);
    resourcePathToCachedUnit.insert(QStringLiteral("/qt/qml/QT456TDirect/qml/components/CameraTile.qml"), &QmlCacheGeneratedCode::_qt_qml_QT456TDirect_qml_components_CameraTile_qml::unit);
    resourcePathToCachedUnit.insert(QStringLiteral("/qt/qml/QT456TDirect/qml/pages/DashboardPage.qml"), &QmlCacheGeneratedCode::_qt_qml_QT456TDirect_qml_pages_DashboardPage_qml::unit);
    resourcePathToCachedUnit.insert(QStringLiteral("/qt/qml/QT456TDirect/qml/pages/RoiEditorPage.qml"), &QmlCacheGeneratedCode::_qt_qml_QT456TDirect_qml_pages_RoiEditorPage_qml::unit);
    resourcePathToCachedUnit.insert(QStringLiteral("/qt/qml/QT456TDirect/qml/components/RoiItem.qml"), &QmlCacheGeneratedCode::_qt_qml_QT456TDirect_qml_components_RoiItem_qml::unit);
    resourcePathToCachedUnit.insert(QStringLiteral("/qt/qml/QT456TDirect/qml/components/ImageCanvas.qml"), &QmlCacheGeneratedCode::_qt_qml_QT456TDirect_qml_components_ImageCanvas_qml::unit);
    resourcePathToCachedUnit.insert(QStringLiteral("/qt/qml/QT456TDirect/qml/components/RoiEditorWindow.qml"), &QmlCacheGeneratedCode::_qt_qml_QT456TDirect_qml_components_RoiEditorWindow_qml::unit);
    QQmlPrivate::RegisterQmlUnitCacheHook registration;
    registration.structVersion = 0;
    registration.lookupCachedQmlUnit = &lookupCachedUnit;
    QQmlPrivate::qmlregister(QQmlPrivate::QmlUnitCacheHookRegistration, &registration);
}

Registry::~Registry() {
    QQmlPrivate::qmlunregister(QQmlPrivate::QmlUnitCacheHookRegistration, quintptr(&lookupCachedUnit));
}

const QQmlPrivate::CachedQmlUnit *Registry::lookupCachedUnit(const QUrl &url) {
    if (url.scheme() != QLatin1String("qrc"))
        return nullptr;
    QString resourcePath = QDir::cleanPath(url.path());
    if (resourcePath.isEmpty())
        return nullptr;
    if (!resourcePath.startsWith(QLatin1Char('/')))
        resourcePath.prepend(QLatin1Char('/'));
    return unitRegistry()->resourcePathToCachedUnit.value(resourcePath, nullptr);
}
}
int QT_MANGLE_NAMESPACE(qInitResources_qmlcache_appQT456TDirect)() {
    ::unitRegistry();
    return 1;
}
Q_CONSTRUCTOR_FUNCTION(QT_MANGLE_NAMESPACE(qInitResources_qmlcache_appQT456TDirect))
int QT_MANGLE_NAMESPACE(qCleanupResources_qmlcache_appQT456TDirect)() {
    return 1;
}
