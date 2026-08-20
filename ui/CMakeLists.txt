find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 QuickDialogs2 QuickLayouts)

qt_policy(SET QTP0001 NEW)  # QML module resource prefix derived from URI
qt_policy(SET QTP0004 NEW)  # require qmldir for extra QML source directories

qt_standard_project_setup()

set(CMAKE_AUTOMOC ON)

qt_add_executable(rts_ui
    src/main.cpp
    src/mapmodel.hpp
    src/mapmodel.cpp
    src/solvercontroller.hpp
    src/solvercontroller.cpp
)

qt_add_qml_module(rts_ui
    URI RtsPathfinder
    VERSION 1.0
    QML_FILES
        qml/Main.qml
)

# Qt's generated QML type-registration file includes headers with QML_ELEMENT
# via #include <HeaderName.hpp> (angle brackets, filename only), so their
# directory must be on the include path directly, not just ui/.
target_include_directories(rts_ui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(rts_ui PRIVATE
    rts_core
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::QuickLayouts
)
