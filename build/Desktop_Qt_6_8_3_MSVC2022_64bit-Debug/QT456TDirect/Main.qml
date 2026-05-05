import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "qml/pages"
ApplicationWindow {
    id: window
    width: 1600
    height: 960
    visible: true
    title: "Four Camera Inspection"

    color: "#f5f7fa"
//1.初始界面默认DashboardPage2.将自己传给stackViewRef3.在
StackView { id: stackView
    anchors.fill: parent
    initialItem: DashboardPage
    {
        stackViewRef: stackView
    }
    }
}
