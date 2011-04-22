set (LIB3DS_SOURCES
  lib3ds_impl.h
  lib3ds_atmosphere.c
  lib3ds_background.c
  lib3ds_camera.c
  lib3ds_chunk.c
  lib3ds_chunktable.c
  lib3ds_file.c
  lib3ds_io.c
  lib3ds_light.c
  lib3ds_material.c
  lib3ds_math.c
  lib3ds_matrix.c
  lib3ds_mesh.c
  lib3ds_node.c
  lib3ds_quat.c
  lib3ds_shadow.c
  lib3ds_track.c
  lib3ds_util.c
  lib3ds_vector.c
  lib3ds_viewport.c
)

set (LIB3DS_HEADERS
  lib3ds.h
)

foreach (header ${LIB3DS_HEADERS})
  list (APPEND TMP_HEADERS src/${header})
endforeach ()
set (LIB3DS_HEADERS TMP_HEADERS)
foreach (source ${LIB3DS_SOURCES})
  list (APPEND TMP_SRCS src/${source})
endforeach ()
set (LIB3DS_SOURCES ${TMP_SRCS})

set (LIB3DS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src CACHE INTERNAL "")

add_library(lib3ds STATIC ${LIB3DS_SOURCES})

set_target_properties (lib3ds
  PROPERTIES
  VERSION 2.0
  SOVERSION 2
)

target_link_libraries (lib3ds m)
set (LIB3DS_LIB lib3ds CACHE INTERNAL "")
