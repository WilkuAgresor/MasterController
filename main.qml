import QtQuick 2.9
import QtQuick.Window 2.3
import QtQuick.Controls 2.2

Window {
    visible: true
    width: 640
    height: 480
    title: qsTr("Hello World")

    Connections {
        target: mainApp
        onInProgress: {
            status.text = controllerName + " in progress"
        }
        onFinished: {
            status.text = controllerName + ": " + opStatus
        }
    }

    Column {
        id: tools

        TextField {
            id: ipAddr
        }
        TextField {
            id: port
        }
        TextField {
            id: message
        }
        Text {
            id: status
        }

        Button {
            id: execute
            text: "Execute"
            onClicked: {
                mainApp.sendRequest("Elektrozaczep", message.text)
            }
        }
        Button {
            id: execute2
            text: "Execute2"
            onClicked: {
                mainApp.sendRequest("Czujnik_okno_1", message.text)
            }
        }

        ListView{
            focus: true
            highlight: Rectangle {color: "lightblue"}

            model: ListModel {
                ListElement{device: "czujnik 1"; room: "salon"}
                ListElement{device: "czujnik 2"; room: "sypialnia"}
                ListElement{device: "czujnik 3"; room: "strych"}
            }
            delegate: Item {
                height: 40
                width: 300
                Column {
                    Text {text: "<b>czujnik: </b>" + device}
                    Text {text: "<b>pokój: </b>" + room}
                }
            }
        }
    }



}
