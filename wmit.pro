TARGET = wmit
TEMPLATE = app

QT = core gui opengl xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

INCLUDEPATH += src src/basic src/formats src/ui src/widgets 3rdparty/GLEW/include

HEADERS += \
    3rdparty/GLEW/include/GL/glew.h \
    src/ui/aboutdialog.h \
    src/wmit.h \
    src/basic/IGLShaderManager.h \
    src/basic/IGLShaderRenderable.h \
    src/ui/TextureDialog.h \
    src/Generic.h \
    src/Util.h \
    src/widgets/QtGLView.h \
    src/ui/ExportDialog.h \
    src/ui/ImportDialog.h \
    src/ui/MainWindow.h \
    src/ui/TexConfigDialog.h \
    src/ui/TransformDock.h \
    src/ui/UVEditor.h \
    src/formats/Mesh.h \
    src/formats/OBJ.h \
    src/formats/Pie.h \
    src/formats/Pie_t.hpp \
    src/formats/WZM.h \
    src/basic/GLTexture.h \
    src/basic/IAnimatable.h \
    src/basic/IGLRenderable.h \
    src/basic/IGLTexturedRenderable.h \
    src/basic/IGLTextureManager.h \
    src/basic/Polygon.h \
    src/basic/Polygon_t.hpp \
    src/basic/Vector.h \
    src/basic/VectorTypes.h \
    src/basic/WZLight.h \
    src/widgets/QWZM.h \
    src/ui/MaterialDock.h \
    src/ui/LightColorWidget.h \
    src/ui/LightColorDock.h \
    src/ui/meshdock.h
    
SOURCES += \
    3rdparty/GLEW/src/glew.c \
    src/formats/WZM.cpp \
    src/formats/Pie.cpp \
    src/formats/Mesh.cpp \
    src/ui/UVEditor.cpp \
    src/ui/TransformDock.cpp \
    src/ui/MainWindow.cpp \
    src/ui/ImportDialog.cpp \
    src/ui/ExportDialog.cpp \
    src/Util.cpp \
    src/main.cpp \
    src/Generic.cpp \
    src/basic/GLTexture.cpp \
    src/basic/WZLight.cpp \
    src/ui/aboutdialog.cpp \
    src/widgets/QWZM.cpp \
    src/widgets/QtGLView.cpp \
    src/ui/TextureDialog.cpp \
    src/ui/TexConfigDialog.cpp \
    src/ui/MaterialDock.cpp \
    src/ui/LightColorWidget.cpp \
    src/ui/LightColorDock.cpp \
    src/ui/meshdock.cpp
    
FORMS += \
    src/ui/UVEditor.ui \
    src/ui/TransformDock.ui \
    src/ui/MainWindow.ui \
    src/ui/ImportDialog.ui \
    src/ui/ExportDialog.ui \
    src/ui/TextureDialog.ui \
    src/ui/TexConfigDialog.ui \
    src/ui/MaterialDock.ui \
    src/ui/LightColorWidget.ui \
    src/ui/LightColorDock.ui \
    src/ui/aboutdialog.ui \
    src/ui/meshdock.ui
    
OTHER_FILES += \
    TODO.txt \
    COPYING.txt \
    HACKING.txt \
    COPYING.nongpl.txt


CONFIG(debug, debug|release) {
    DEFINES *= DEBUG _DEBUG
} else {
    DEFINES *= NDEBUG
}

win32 {
    DEFINES += WIN32
}

DEFINES += GLEW_STATIC
    
LIBS += -lm
!win32 {
    LIBS += -lGLU
} else {
    LIBS += -lglu32 -lopengl32
}

# Disable gazillion warnings from GCC
#CONFIG += warn_off
QMAKE_CFLAGS_WARN_ON += -Wno-cast-function-type
QMAKE_CXXFLAGS_WARN_ON += -Wno-deprecated-copy

RESOURCES += \
    resources.qrc

# If your system uses different paths for QGLViewer, create a file named
# "config.pri" and override the necessary variables below (with "=").
QGLVIEWER_INCL = 3rdparty/libQGLViewer
QGLVIEWER_LIBS = -L"$$_PRO_FILE_PWD_"/3rdparty/libQGLViewer/QGLViewer
lessThan(QT_MAJOR_VERSION, 5): QGLVIEWER_LIBS += -lQGLViewer
greaterThan(QT_MAJOR_VERSION, 4): QGLVIEWER_LIBS += -lQGLViewer-qt5

UI_DIR = ui
MOC_DIR = moc
OBJECTS_DIR = bin

include("config.pri")

INCLUDEPATH += $$QGLVIEWER_INCL
LIBS += $$QGLVIEWER_LIBS
