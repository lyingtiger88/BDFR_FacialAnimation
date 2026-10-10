find_package(Qt6 6.8 REQUIRED COMPONENTS Widgets Network OpenGL OpenGLWidgets)

qt_standard_project_setup()

qt_add_executable(BDFRStudio
    main.cpp
    StudioMainWindow.h
    StudioMainWindow.cpp
    FacePreviewWidget.h
    FacePreviewWidget.cpp
    MetaHumanViewportWidget.h
    MetaHumanViewportWidget.cpp
)

target_link_libraries(BDFRStudio PRIVATE
    bdfr_core
    bdfr_metahuman
    bdfr_facebuilder
    Qt6::Widgets
    Qt6::Network
    Qt6::OpenGL
    Qt6::OpenGLWidgets
)

target_include_directories(BDFRStudio PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

set_target_properties(BDFRStudio PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
    OUTPUT_NAME "BDFR Studio"
)


add_test(
    NAME bdfr_studio_self_test
    COMMAND BDFRStudio --self-test
)
