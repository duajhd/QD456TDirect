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

    property bool closeConfirmed: false

    onClosing: function(close) {
        if (closeConfirmed)
            return

        close.accepted = false
        closeConfirmPopup.open()
    }

    Popup {
        id: closeConfirmPopup
        modal: true
        focus: true
        closePolicy: Popup.NoAutoClose
        width: 420
        height: 188
        x: Math.round((window.width - width) / 2)
        y: Math.round((window.height - height) / 2)
        padding: 0

        Overlay.modal: Rectangle {
            color: "#73000000"
        }

        background: Rectangle {
            radius: 8
            color: "#ffffff"
            border.color: "#f0f0f0"
            border.width: 1

            Rectangle {
                anchors.fill: parent
                anchors.topMargin: 2
                radius: parent.radius
                color: "#14000000"
                z: -1
            }
        }

        contentItem: ColumnLayout {
            anchors.fill: parent
            anchors.margins: 24
            spacing: 18

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Rectangle {
                    Layout.preferredWidth: 22
                    Layout.preferredHeight: 22
                    radius: 11
                    color: "#faad14"
                    Layout.alignment: Qt.AlignTop

                    Text {
                        anchors.centerIn: parent
                        text: "!"
                        color: "#ffffff"
                        font.pixelSize: 15
                        font.bold: true
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        text: "关闭确认"
                        color: "#262626"
                        font.pixelSize: 16
                        font.bold: true
                    }

                    Text {
                        text: "确定关闭程序吗?"
                        color: "#595959"
                        font.pixelSize: 14
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                }
            }

            Item {
                Layout.fillHeight: true
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    id: cancelCloseButton
                    Layout.preferredWidth: 72
                    Layout.preferredHeight: 32
                    text: "取消"
                    padding: 0

                    background: Rectangle {
                        radius: 6
                        color: cancelCloseButton.down ? "#f5f5f5" : "#ffffff"
                        border.width: 1
                        border.color: cancelCloseButton.hovered ? "#4096ff" : "#d9d9d9"
                    }

                    contentItem: Text {
                        text: cancelCloseButton.text
                        color: cancelCloseButton.hovered ? "#1677ff" : "#262626"
                        font.pixelSize: 14
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: closeConfirmPopup.close()
                }

                Button {
                    id: confirmCloseButton
                    Layout.preferredWidth: 72
                    Layout.preferredHeight: 32
                    text: "确定"
                    padding: 0

                    background: Rectangle {
                        radius: 6
                        color: confirmCloseButton.down ? "#0958d9"
                             : confirmCloseButton.hovered ? "#4096ff"
                             : "#1677ff"
                    }

                    contentItem: Text {
                        text: confirmCloseButton.text
                        color: "#ffffff"
                        font.pixelSize: 14
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    onClicked: {
                        window.closeConfirmed = true
                        window.close()
                    }
                }
            }
        }
    }

    StackView {
        id: stackView
        anchors.fill: parent

        initialItem: DashboardPage {
            stackViewRef: stackView
        }
    }
}
