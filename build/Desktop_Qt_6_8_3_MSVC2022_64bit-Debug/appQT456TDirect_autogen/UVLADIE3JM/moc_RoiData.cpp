/****************************************************************************
** Meta object code from reading C++ file 'RoiData.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../src/RoiData.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'RoiData.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN7RoiDataE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN7RoiDataE = QtMocHelpers::stringData(
    "RoiData",
    "RoiChanged",
    "",
    "roiId",
    "roiName",
    "roiType",
    "centerX",
    "centerY",
    "roiWidth",
    "roiHeight",
    "angle",
    "color",
    "selected"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN7RoiDataE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
      10,   21, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   20,    2, 0x06,   11 /* Public */,

 // signals: parameters
    QMetaType::Void,

 // properties: name, type, flags, notifyId, revision
       3, QMetaType::QString, 0x00015003, uint(0), 0,
       4, QMetaType::QString, 0x00015003, uint(0), 0,
       5, QMetaType::QString, 0x00015003, uint(0), 0,
       6, QMetaType::Double, 0x00015003, uint(0), 0,
       7, QMetaType::Double, 0x00015003, uint(0), 0,
       8, QMetaType::Double, 0x00015003, uint(0), 0,
       9, QMetaType::Double, 0x00015003, uint(0), 0,
      10, QMetaType::Double, 0x00015003, uint(0), 0,
      11, QMetaType::QString, 0x00015003, uint(0), 0,
      12, QMetaType::Bool, 0x00015003, uint(0), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject RoiData::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ZN7RoiDataE.offsetsAndSizes,
    qt_meta_data_ZN7RoiDataE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN7RoiDataE_t,
        // property 'roiId'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'roiName'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'roiType'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'centerX'
        QtPrivate::TypeAndForceComplete<double, std::true_type>,
        // property 'centerY'
        QtPrivate::TypeAndForceComplete<double, std::true_type>,
        // property 'roiWidth'
        QtPrivate::TypeAndForceComplete<double, std::true_type>,
        // property 'roiHeight'
        QtPrivate::TypeAndForceComplete<double, std::true_type>,
        // property 'angle'
        QtPrivate::TypeAndForceComplete<double, std::true_type>,
        // property 'color'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'selected'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<RoiData, std::true_type>,
        // method 'RoiChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void RoiData::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<RoiData *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->RoiChanged(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (RoiData::*)();
            if (_q_method_type _q_method = &RoiData::RoiChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = _t->GetRoiId(); break;
        case 1: *reinterpret_cast< QString*>(_v) = _t->GetRoiName(); break;
        case 2: *reinterpret_cast< QString*>(_v) = _t->GetRoiType(); break;
        case 3: *reinterpret_cast< double*>(_v) = _t->GetCenterX(); break;
        case 4: *reinterpret_cast< double*>(_v) = _t->GetCenterY(); break;
        case 5: *reinterpret_cast< double*>(_v) = _t->GetRoiWidth(); break;
        case 6: *reinterpret_cast< double*>(_v) = _t->GetRoiHeight(); break;
        case 7: *reinterpret_cast< double*>(_v) = _t->GetAngle(); break;
        case 8: *reinterpret_cast< QString*>(_v) = _t->GetColor(); break;
        case 9: *reinterpret_cast< bool*>(_v) = _t->GetSelected(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->SetRoiId(*reinterpret_cast< QString*>(_v)); break;
        case 1: _t->SetRoiName(*reinterpret_cast< QString*>(_v)); break;
        case 2: _t->SetRoiType(*reinterpret_cast< QString*>(_v)); break;
        case 3: _t->SetCenterX(*reinterpret_cast< double*>(_v)); break;
        case 4: _t->SetCenterY(*reinterpret_cast< double*>(_v)); break;
        case 5: _t->SetRoiWidth(*reinterpret_cast< double*>(_v)); break;
        case 6: _t->SetRoiHeight(*reinterpret_cast< double*>(_v)); break;
        case 7: _t->SetAngle(*reinterpret_cast< double*>(_v)); break;
        case 8: _t->SetColor(*reinterpret_cast< QString*>(_v)); break;
        case 9: _t->SetSelected(*reinterpret_cast< bool*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *RoiData::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *RoiData::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN7RoiDataE.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int RoiData::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 1;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void RoiData::RoiChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
