set (LIB3DS_SOURCES
  io.c
  vector.c
  matrix.c
  quat.c
  tcb.c
  ease.c
  chunktable.h
  chunk.c
  file.c
  background.c
  atmosphere.c
  shadow.c
  viewport.c
  material.c
  mesh.c
  camera.c
  light.c
  tracks.c
  node.c
)

set (LIB3DS_HEADERS
  types.h
  io.h
  vector.h
  matrix.h
  quat.h
  tcb.h
  ease.h
  chunk.h
  file.h
  background.h
  atmosphere.h
  shadow.h
  viewport.h
  material.h
  mesh.h
  camera.h
  light.h
  tracks.h
  node.h
)

foreach (header ${LIB3DS_HEADERS})
  list (APPEND TMP_HEADERS lib3ds/${header})
endforeach ()
set (LIB3DS_HEADERS TMP_HEADERS)
foreach (source ${LIB3DS_SOURCES})
  list (APPEND TMP_SRCS lib3ds/${source})
endforeach ()
set (LIB3DS_SOURCES ${TMP_SRCS})


include_directories (${CMAKE_CURRENT_SOURCE_DIR})
set (LIB3DS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")

add_library(lib3ds STATIC ${LIB3DS_SOURCES})

set_target_properties (lib3ds
  PROPERTIES
  VERSION 1.3
  SOVERSION 1
)

target_link_libraries (lib3ds m)
set (LIB3DS_LIB lib3ds CACHE INTERNAL "")
