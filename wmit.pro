TARGET = wmit
TEMPLATE = app

QT = core gui opengl xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += src src/basic src/formats src/ui src/widgets 3rdparty/GLee

HEADERS += \
    3rdparty/GLee/GLee.h \
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
    src/formats/WZM.h \
    src/basic/GLTexture.h \
    src/basic/IAnimatable.h \
    src/basic/IGLRenderable.h \
    src/basic/IGLTexturedRenderable.h \
    src/basic/IGLTextureManager.h \
    src/basic/Polygon.h \
    src/basic/Vector.h \
    src/basic/VectorTypes.h \
    src/widgets/QWZM.h \
    src/ui/MaterialDock.h \
    src/ui/meshdock.h
    
SOURCES += \
    src/formats/WZM.cpp \
    src/formats/Pie_t.cpp \
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
    src/basic/Polygon_t.cpp \
    src/basic/GLTexture.cpp \
    3rdparty/GLee/GLee.c \
    src/widgets/QWZM.cpp \
    src/widgets/QtGLView.cpp \
    src/ui/TextureDialog.cpp \
    src/ui/TexConfigDialog.cpp \
    src/ui/MaterialDock.cpp \
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
    src/ui/meshdock.ui
    
OTHER_FILES += \
    TODO.txt \
    COPYING.txt \
    HACKING.txt \
    COPYING.nongpl


CONFIG(debug, debug|release) {
    DEFINES *= DEBUG _DEBUG
} else {
    DEFINES *= NDEBUG
}

# turn off c++0x support on win32 till decent compiler is available out-of-the-box
#!win32 {
{
    QMAKE_CXXFLAGS += -std=c++0x
    DEFINES += CPP0X_AVAILABLE
}
win32 {
    DEFINES += WIN32
}
    
LIBS += -lm
!win32 {
    LIBS += -lGLU
} else {
    LIBS += -lglu32 -lopengl32
}

RESOURCES += \
    resources.qrc

# If your system uses different paths for QGLViewer, create a file named
# "config.pri" and override the necessary variables below (with "=").
QGLVIEWER_INCL = /usr/include/QGLViewer
QGLVIEWER_LIBS = -lQGLViewer

UI_DIR = ui
MOC_DIR = moc
OBJECTS_DIR = bin

include("config.pri")

INCLUDEPATH += $$QGLVIEWER_INCL
LIBS += $$QGLVIEWER_LIBS
