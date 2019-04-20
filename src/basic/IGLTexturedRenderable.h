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

#include "GLTexture.h"
#include "IGLTextureManager.h"
#include "IGLRenderable.h"

class IGLTexturedRenderable : virtual public IGLRenderable
{
protected:
	IGLTextureManager *m_texMan;

public:
	IGLTexturedRenderable(): m_texMan(nullptr) {}
	IGLTexturedRenderable(IGLTextureManager *manager): m_texMan(manager) {}
	virtual ~IGLTexturedRenderable() {}

	virtual void setTextureManager(IGLTextureManager *manager) {m_texMan = manager;}

	virtual QString idToFilePath(GLuint id)
	{
		if (m_texMan != nullptr)
		{
			return m_texMan->idToFilePath(id);
		}
		else
		{
			return QString();
		}
	}

	virtual IGLTextureManager* getTextureManager() {return m_texMan;}

	virtual bool hasTextureManager() const {return m_texMan != nullptr;}

protected:
	virtual GLTexture createTexture(const QString& fileName) const
	{
		if (m_texMan != nullptr)
		{
			return m_texMan->createTexture(fileName);
		}
		else
		{
			return GLTexture();
		}
	}

	virtual void deleteTexture(GLuint id) const
	{
		if (m_texMan != nullptr)
		{
			m_texMan->deleteTexture(id);
		}
	}
};
