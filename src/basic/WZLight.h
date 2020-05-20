/*
	Copyright 2010 Warzone 2100 Project

	This file is part of WMIT.

	WMIT is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	WMIT is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with WMIT.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef WZLIGHT_H
#define WZLIGHT_H

#include <array>

#include <GL/glew.h>

enum LIGHTING_TYPE {
	LIGHT_EMISSIVE, LIGHT_AMBIENT, LIGHT_DIFFUSE, LIGHT_SPECULAR, LIGHT_TYPE_MAX
};

enum LIGHTING_WZVER {
	LIGHT_WZ32, LIGHT_WZ33, LIGHT_WZVER_MAX
};

typedef std::array<GLfloat, 4> light_col_t;
typedef std::array<light_col_t, LIGHT_TYPE_MAX> light_cols_t;

extern light_cols_t lightCol0_external;
extern light_cols_t lightCol0;

void switchLightToWzVer(LIGHTING_WZVER ver, bool allow_external);
void switchLightToExternal();
bool updateLightToExternalIfNeeded();

void loadLightColorSetting();
void saveLightColorSettings();

#endif // WZLIGHT_H
