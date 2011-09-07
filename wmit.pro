TARGET = wmit
TEMPLATE = app

QT += opengl xml

INCLUDEPATH += src src/basic src/formats src/ui src/widgets 3rdparty/GLee

HEADERS += \
    src/formats/WZM.hpp \
    src/formats/Pie.hpp \
    src/formats/OBJ.hpp \
    src/formats/Mesh.hpp \
    src/ui/UVEditor.hpp \
    src/ui/TransformDock.hpp \
    src/ui/TeamColoursDock.hpp \
    src/ui/MainWindow.hpp \
    src/ui/ImportDialog.hpp \
    src/ui/ExportDialog.hpp \
    src/ui/ConfigDialog.hpp \
    src/Util.hpp \
    src/Generic.hpp \
    src/basic/VectorTypes.hpp \
    src/basic/Vector.hpp \
    src/basic/TexturedRenderable.hpp \
    src/basic/TCMaskRenderable.hpp \
    src/basic/Polygon.hpp \
    src/basic/ITCMaskProvider.hpp \
    src/basic/IGLTextureManager.hpp \
    src/basic/IGLRenderable.hpp \
    src/basic/IAnimatable.hpp \
    src/basic/GLTexture.hpp \
    3rdparty/GLee/GLee.h \
    src/widgets/QWZM.hpp \
    src/widgets/QtGLView.hpp
    
SOURCES += \
    src/formats/WZM.cpp \
    src/formats/Pie_t.cpp \
    src/formats/Pie.cpp \
    src/formats/Mesh.cpp \
    src/ui/UVEditor.cpp \
    src/ui/TransformDock.cpp \
    src/ui/TeamColoursDock.cpp \
    src/ui/MainWindow.cpp \
    src/ui/ImportDialog.cpp \
    src/ui/ExportDialog.cpp \
    src/ui/ConfigDialog.cpp \
    src/Util.cpp \
    src/main.cpp \
    src/Generic.cpp \
    src/basic/Polygon_t.cpp \
    src/basic/GLTexture.cpp \
    3rdparty/GLee/GLee.c \
    src/widgets/QWZM.cpp \
    src/widgets/QtGLView.cpp
    
FORMS += \
    src/ui/UVEditor.ui \
    src/ui/TransformDock.ui \
    src/ui/TeamColoursDock.ui \
    src/ui/MainWindow.ui \
    src/ui/ImportDialog.ui \
    src/ui/ExportDialog.ui \
    src/ui/ConfigDialog.ui
    
OTHER_FILES += \
    TODO.txt \
    COPYING.txt \
    HACKING.txt \
    COPYING.nongpl

QMAKE_CXXFLAGS += \
    -std=c++0x

# If your system uses different paths for QGLViewer, create a file named
# "config.pri" and override the necessary variables below (with "=").
QGLVIEWER_INCL = /usr/include/QGLViewer
QGLVIEWER_LIBS = -lQGLViewer

include("config.pri")

UI_DIR = ui
MOC_DIR = moc
OBJECTS_DIR = bin

INCLUDEPATH += $$QGLVIEWER_INCL

LIBS += -l3ds \
    -lm \
    $$QGLVIEWER_LIBS
