
include_directories( ../ )

add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_PLUGIN)

QT5_WRAP_UI ( UI_SRC
    decimaldegree_to_utm.ui
    )

add_library(ToolboxCoordinates SHARED
    toolbox_coordinates.cpp
    decimaldegree_to_utm.cpp
    DecDegUTM/latlong_utm_conversion.cpp
    ${UI_SRC}  )

target_link_libraries(ToolboxCoordinates
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Xml_LIBRARIES}
    plotjuggler_base
    plotjuggler_qwt)

install(TARGETS ToolboxCoordinates DESTINATION ${PJ_PLUGIN_INSTALL_DIRECTORY}  )
