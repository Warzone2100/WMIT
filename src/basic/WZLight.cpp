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

#include "WZLight.h"

#include <QString>
#include <QSettings>

const static QString base_lightcol_name = "3DView/LightColor";

const static std::array<light_cols_t,LIGHT_WZVER_MAX> lightCol0_default = {{
	{{{0.0f, 0.0f, 0.0f, 1.0f},  {0.5f, 0.5f, 0.5f, 1.0f}, {0.8f, 0.8f, 0.8f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}},
	{{{0.f, 0.f, 0.f, 1.f},  {1.f, 1.f, 1.f, 1.f},  {0.f, 0.f, 0.f, 1.f},  {1.f, 1.f, 1.f, 1.f}}}
}};

static light_cols_t lightCol0_external = lightCol0_default[LIGHT_WZ33];
light_cols_t lightCol0 = lightCol0_default[LIGHT_WZ33];
static bool lightCol_use_external = false;

void getExtLightColFromSettings(const LIGHTING_TYPE type, const char* lightcol_suffix)
{
	QStringList lightCol = QSettings().value(base_lightcol_name + lightcol_suffix).toStringList();
	if (lightCol.count() == 4)
		for (size_t idx = 0; idx < 4; ++idx) {
			lightCol0_external[type][idx] = lightCol[static_cast<int>(idx)].toFloat();
		}
}

void setExtLightColToSettings(const LIGHTING_TYPE type, const char* lightcol_suffix)
{
	QStringList lightCol;
	for (size_t idx = 0; idx < 4; ++idx) {
		lightCol.append(QString("%1").arg(static_cast<double>(lightCol0_external[type][idx])));
	}
	QSettings().setValue(base_lightcol_name + lightcol_suffix, lightCol);
}


void loadLightColorSetting()
{
	getExtLightColFromSettings(LIGHT_EMISSIVE, "_E");
	getExtLightColFromSettings(LIGHT_AMBIENT, "_A");
	getExtLightColFromSettings(LIGHT_DIFFUSE, "_D");
	getExtLightColFromSettings(LIGHT_SPECULAR, "_S");

	lightCol_use_external = QSettings().value(base_lightcol_name + "_UseExternal",
						  lightCol_use_external).toBool();
}

void saveLightColorSettings()
{
	setExtLightColToSettings(LIGHT_EMISSIVE, "_E");
	setExtLightColToSettings(LIGHT_AMBIENT, "_A");
	setExtLightColToSettings(LIGHT_DIFFUSE, "_D");
	setExtLightColToSettings(LIGHT_SPECULAR, "_S");

	QSettings().setValue(base_lightcol_name + "_UseExternal", lightCol_use_external);
}

void switchLightToWzVer(LIGHTING_WZVER ver, bool allow_external)
{
	if (allow_external && lightCol_use_external)
		switchLightToExternal();
	else
		lightCol0 = lightCol0_default[ver];
}

void switchLightToExternal()
{
	lightCol0 = lightCol0_external;
}
