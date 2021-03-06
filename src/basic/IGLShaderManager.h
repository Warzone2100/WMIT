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

#pragma once

#include <QString>
#include <QPointer>
#include <QHash>
#include <QHashIterator>
#include <QOpenGLShaderProgram>

struct ShaderInfo
{
	QPointer<QOpenGLShaderProgram> program;
	bool is_external;
};

class IGLShaderManager
{
protected:
	QHash<int, ShaderInfo> m_shaders;
public:
	virtual ~IGLShaderManager() {}

	virtual bool loadShader(int type, const QString& fileNameVert, const QString& fileNameFrag,
				QString* errString) = 0;
	virtual void unloadShader(int type) = 0;

	virtual bool hasShader(int type) const
	{
		return m_shaders.contains(type) && !m_shaders.value(type).program.isNull();
	}

	virtual bool isShaderExternal(int type) const
	{
		return m_shaders.contains(type) && m_shaders.value(type).is_external;
	}

	virtual QOpenGLShaderProgram* getShader(int type)
	{
		if (m_shaders.contains(type))
			return m_shaders[type].program.data();

		return nullptr;
	}
};
