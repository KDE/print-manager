import Qt 4.7
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1

Item {
    property alias delegate: list.delegate
    property alias model: list.model

    ListView {
        id: list
        clip: true
        anchors {
            left:   parent.left
            right:  scrollBar.visible ? scrollBar.left : parent.right
            top :   parent.top
            bottom: parent.bottom
        }
        highlight: highlighter
        highlightMoveDuration: 250
        highlightMoveSpeed: 1
    }
    Component {
        id: highlighter
        PlasmaCore.FrameSvgItem {
            imagePath: "widgets/viewitem"
            prefix: "hover"
            opacity: 0
            Behavior on opacity {
                NumberAnimation {
                    duration: 250
                    easing.type: Easing.OutQuad
                }
            }
        }
    }
    PlasmaComponents.ScrollBar {
        id: scrollBar
        flickableItem: list
        anchors {
            right: parent.right
            top: list.top
            bottom: list.bottom
        }
    }
}