/****************************************************************************
** Meta object code from reading C++ file 'RoiManager.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/RoiManager.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'RoiManager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN10RoiManagerE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN10RoiManagerE = QtMocHelpers::stringData(
    "RoiManager",
    "RoiListChanged",
    "",
    "AddRoi",
    "RoiData*",
    "roiType",
    "centerX",
    "centerY",
    "roiWidth",
    "roiHeight",
    "angle",
    "color",
    "RemoveRoi",
    "roiId",
    "ClearAllRois",
    "GetRoiById",
    "SelectOnly",
    "UnselectAll",
    "SaveToJson",
    "filePath",
    "LoadFromJson",
    "roiList",
    "QQmlListProperty<RoiData>"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN10RoiManagerE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       1,  101, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   68,    2, 0x06,    2 /* Public */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
       3,    7,   69,    2, 0x02,    3 /* Public */,
      12,    1,   84,    2, 0x02,   11 /* Public */,
      14,    0,   87,    2, 0x02,   13 /* Public */,
      15,    1,   88,    2, 0x02,   14 /* Public */,
      16,    1,   91,    2, 0x02,   16 /* Public */,
      17,    0,   94,    2, 0x02,   18 /* Public */,
      18,    1,   95,    2, 0x02,   19 /* Public */,
      20,    1,   98,    2, 0x02,   21 /* Public */,

 // signals: parameters
    QMetaType::Void,

 // methods: parameters
    0x80000000 | 4, QMetaType::QString, QMetaType::Double, QMetaType::Double, QMetaType::Double, QMetaType::Double, QMetaType::Double, QMetaType::QString,    5,    6,    7,    8,    9,   10,   11,
    QMetaType::Void, QMetaType::QString,   13,
    QMetaType::Void,
    0x80000000 | 4, QMetaType::QString,   13,
    QMetaType::Void, QMetaType::QString,   13,
    QMetaType::Void,
    QMetaType::Bool, QMetaType::QString,   19,
    QMetaType::Bool, QMetaType::QString,   19,

 // properties: name, type, flags, notifyId, revision
      21, 0x80000000 | 22, 0x00015009, uint(0), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject RoiManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ZN10RoiManagerE.offsetsAndSizes,
    qt_meta_data_ZN10RoiManagerE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN10RoiManagerE_t,
        // property 'roiList'
        QtPrivate::TypeAndForceComplete<QQmlListProperty<RoiData>, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<RoiManager, std::true_type>,
        // method 'RoiListChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'AddRoi'
        QtPrivate::TypeAndForceComplete<RoiData *, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'RemoveRoi'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'ClearAllRois'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'GetRoiById'
        QtPrivate::TypeAndForceComplete<RoiData *, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'SelectOnly'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'UnselectAll'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'SaveToJson'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'LoadFromJson'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void RoiManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<RoiManager *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->RoiListChanged(); break;
        case 1: { RoiData* _r = _t->AddRoi((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[4])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[5])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[6])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[7])));
            if (_a[0]) *reinterpret_cast< RoiData**>(_a[0]) = std::move(_r); }  break;
        case 2: _t->RemoveRoi((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->ClearAllRois(); break;
        case 4: { RoiData* _r = _t->GetRoiById((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast< RoiData**>(_a[0]) = std::move(_r); }  break;
        case 5: _t->SelectOnly((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->UnselectAll(); break;
        case 7: { bool _r = _t->SaveToJson((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 8: { bool _r = _t->LoadFromJson((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (RoiManager::*)();
            if (_q_method_type _q_method = &RoiManager::RoiListChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QQmlListProperty<RoiData>*>(_v) = _t->GetRoiList(); break;
        default: break;
        }
    }
}

const QMetaObject *RoiManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *RoiManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN10RoiManagerE.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int RoiManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 9;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void RoiManager::RoiListChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
