import Qt 4.7
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1

Item {
    id: printerItem
    width: printerItem.ListView.view.width
    height: 50

    PlasmaCore.FrameSvgItem {
        id: padding
        imagePath: "widgets/viewitem"
        prefix: "hover"
        opacity: 0
        anchors.fill: parent
    }
    MouseArea {
        id: container
        anchors {
            fill: parent
            topMargin: padding.margins.top
            leftMargin: padding.margins.left
            rightMargin: padding.margins.right
            bottomMargin: padding.margins.bottom
        }
        hoverEnabled: true
        onEntered: {
            printerItem.ListView.view.currentIndex = index;
            printerItem.ListView.view.highlightItem.opacity = 0.7;
        }
        onExited: {
            printerItem.ListView.view.highlightItem.opacity = 0;
        }
    
        QIconItem {
            id: printerIcon
            width: 32
            height: 32
            icon: QIcon(iconName)
            anchors {
                left: parent.left
                top: parent.top
            }
        }
        
        Column {
            id: labelsColumn
            spacing: padding.margins.top/2
            anchors {
                top: parent.top
                left: printerIcon.right
                right: rightAction.left
                leftMargin: padding.margins.left
            }
            PlasmaComponents.Label {
                height: paintedHeight
                elide: Text.ElideRight
                text: info
            }
            
            PlasmaComponents.Label {
                height: paintedHeight
                text: stateMessage
                elide: Text.ElideRight
                font.italic: true
                font.pointSize: theme.smallestFont.pointSize
                color: "#99"+(theme.textColor.toString().substr(1))
            }
        }
        
        QIconItem {
            id: rightAction
            width: 22
            height: 22
            opacity: 0.6
            anchors {
                right: parent.right
                verticalCenter: printerIcon.verticalCenter
            }
            icon: stateEnum == "stopped" ? QIcon("media-playback-start") : QIcon("media-playback-pause")
            
            MouseArea {
                id: mouseArea
                hoverEnabled: true
                anchors {
                    fill: parent
                }
                onClicked: {
                    service = printersSource.serviceForSource(DataEngineSource);
                    operation = service.operationDescription(stateEnum == "stopped" ? "resumePrinter" : "pausePrinter");
                    service.startOperationCall(operation);
                }
                onEntered: {
                    parent.state = "mouseOver";
                }
                onExited: {
                    parent.state = "";
                }
            }
            
            states: State {
                name: "mouseOver"; when: mouseArea.containsMouse
                PropertyChanges { target: rightAction; opacity: 1 }
            }
            transitions: Transition {
                NumberAnimation { properties: "opacity"; easing.type: Easing.InOutQuad }
            }
        }
    }
}