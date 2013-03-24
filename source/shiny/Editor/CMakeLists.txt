set(SHINY_EDITOR_LIBRARY "shiny.Editor")

find_package(Qt4 REQUIRED)
set(QT_USE_QTGUI 1)

# Headers that must be preprocessed
set(SHINY_EDITOR_HEADER_MOC
    MainWindow.hpp
)

set(SHINY_EDITOR_UI
    mainwindow.ui
    newmaterialdialog.ui
)

QT4_WRAP_CPP(MOC_SRCS ${SHINY_EDITOR_HEADER_MOC})
QT4_WRAP_UI(UI_HDRS ${SHINY_EDITOR_UI})

set(SOURCE_FILES
    MainWindow.cpp
    Editor.cpp
    Actions.cpp
    Query.cpp
    NewMaterialDialog.cpp
    mainwindow.ui
    newmaterialdialog.ui
)

include(${QT_USE_FILE})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(${SHINY_EDITOR_LIBRARY} STATIC ${SOURCE_FILES} ${MOC_SRCS} ${UI_HDRS})

set(SHINY_LIBRARIES ${SHINY_LIBRARIES}
    ${SHINY_EDITOR_LIBRARY}
    ${QT_LIBRARIES}
)
set(SHINY_LIBRARIES ${SHINY_LIBRARIES} PARENT_SCOPE)
