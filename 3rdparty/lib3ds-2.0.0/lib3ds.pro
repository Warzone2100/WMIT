TEMPLATE = lib
TARGET = 3ds
VERSION = 2.0.0
CONFIG *= warn_on thread create_prl rtti no_keywords shared

INCLUDEPATH += src

lib3ds_SOURCES = \
	src/lib3ds_impl.h \
	src/lib3ds_atmosphere.c \
	src/lib3ds_background.c \
	src/lib3ds_camera.c \
	src/lib3ds_chunk.c \
	src/lib3ds_chunktable.c \
	src/lib3ds_file.c \
	src/lib3ds_io.c \
	src/lib3ds_light.c \
	src/lib3ds_material.c \
	src/lib3ds_math.c \
	src/lib3ds_matrix.c \
	src/lib3ds_mesh.c \
	src/lib3ds_node.c \
	src/lib3ds_quat.c \
	src/lib3ds_shadow.c \
	src/lib3ds_track.c \
	src/lib3ds_util.c \
	src/lib3ds_vector.c \
	src/lib3ds_viewport.c

lib3ds_HEADERS = \
	src/lib3ds.h

SOURCES = $${lib3ds_SOURCES}
HEADERS = $${lib3ds_HEADERS}

win32 {
	CONFIG *= debug_and_release build_all
}
