#pragma once

#define WMIT_ORG "WMIT"
#define WMIT_APPNAME "WMIT"
#define WMIT_VER_STR "0.7.1"

#define WMIT_SETTINGS_IMPORTVAL "importFolder"
#define WMIT_SETTINGS_EXPORTVAL "exportFolder"
#define WMIT_SETTINGS_TEXSEARCHDIRS "textureSearchDirs"

#define WMIT_SETTINGS_IMPORT_WELDER "Import/EnableWelder"

#define WMIT_WZ_TEXPAGE_REMASK "page\\-(\\d+)"

#define WMIT_SHADER_WZ31_DEFPATH_VERT ":/data/shaders/wz31.vert"
#define WMIT_SHADER_WZ31_DEFPATH_FRAG ":/data/shaders/wz31.frag"

#define WMIT_SHADER_WZ32TC_DEFPATH_VERT ":/data/shaders/wz32_tcmask.vert"
#define WMIT_SHADER_WZ32TC_DEFPATH_FRAG ":/data/shaders/wz32_tcmask.frag"

#define WMIT_SHADER_WZ33TC_DEFPATH_VERT ":/data/shaders/wz33_tcmask.vert"
#define WMIT_SHADER_WZ33TC_DEFPATH_FRAG ":/data/shaders/wz33_tcmask.frag"

#define WMIT_SHADER_WZ40TC_DEFPATH_VERT ":/data/shaders/wz40_tcmask.vert"
#define WMIT_SHADER_WZ40TC_DEFPATH_FRAG ":/data/shaders/wz40_tcmask.frag"

#define WMIT_IMAGES_NOTEXTURE ":/data/images/notex.png"
#define WMIT_IMAGES_BANNER ":/data/images/wmit_banner.png"
#define WMIT_IMAGES_LOGO_64 ":/data/images/wmit_logo_64.png"

#define WMIT_WARN_DEPRECATED_WZM "WARNING: support for .WZM format is deprecated and will be removed in a future release. Please migrate your files into .PIE format!"

enum wmit_filetype_t { WMIT_FT_PIE = 0, WMIT_FT_PIE2, WMIT_FT_WZM, WMIT_FT_OBJ };
