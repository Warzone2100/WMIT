#pragma once

#define WMIT_ORG "WMIT"
#define WMIT_APPNAME "WMIT"
#define WMIT_VER_STR "0.6"

#define WMIT_SETTINGS_IMPORTVAL "importFolder"
#define WMIT_SETTINGS_EXPORTVAL "exportFolder"
#define WMIT_SETTINGS_TEXSEARCHDIRS "textureSearchDirs"

#define WMIT_SETTINGS_IMPORT_WELDER "Import/EnableWelder"

#define WMIT_WZ_TEXPAGE_REMASK "page\\-(\\d+)"

#define WMIT_SHADER_WZ31_DEFPATH_VERT ":/data/shaders/wz31.vert"
#define WMIT_SHADER_WZ31_DEFPATH_FRAG ":/data/shaders/wz31.frag"

#define WMIT_SHADER_WZ32TC_DEFPATH_VERT ":/data/shaders/wz32_tcmask.vert"
#define WMIT_SHADER_WZ32TC_DEFPATH_FRAG ":/data/shaders/wz32_tcmask.frag"

#define WMIT_IMAGES_NOTEXTURE ":/data/images/notex.png"

enum wmit_filetype_t { WMIT_FT_PIE = 0, WMIT_FT_PIE2, WMIT_FT_WZM, WMIT_FT_OBJ };
