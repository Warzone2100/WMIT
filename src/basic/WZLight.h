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

typedef std::array<std::array<GLfloat, 4>, LIGHT_TYPE_MAX> light_cols_t;

const static light_cols_t lightCol0_default = {{
	{0.0f, 0.0f, 0.0f, 1.0f},  {0.5f, 0.5f, 0.5f, 1.0f}, {0.8f, 0.8f, 0.8f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}
};

static light_cols_t lightCol0_external = lightCol0_default;
static light_cols_t lightCol0 = lightCol0_default;
static bool lightCol_use_external = false;

void loadLightColorSetting();
void saveLightColorSettings();

#endif // WZLIGHT_H
