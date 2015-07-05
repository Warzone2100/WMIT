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

#include "IGLShaderManager.h"
#include "IGLRenderable.h"

class IGLShaderRenderable: virtual public IGLRenderable
{
protected:
	IGLShaderManager* m_shaderman;
	int m_active_shader;
public:
	IGLShaderRenderable(): m_shaderman(NULL), m_active_shader(0) {}
	IGLShaderRenderable(IGLShaderManager *shaderman): m_shaderman(shaderman), m_active_shader(0) {}
	virtual ~IGLShaderRenderable() {}

	virtual void setShaderManager(IGLShaderManager* manager) {m_shaderman = manager;}
	virtual bool setActiveShader(const int type)
	{
		releaseShader(m_active_shader);
		if (m_shaderman != NULL && m_shaderman->hasShader(type) && initShader(type))
		{
			m_active_shader = type;
			return true;
        }

        return false;
	}
	virtual int getActiveShader() const {return m_active_shader;}

protected:
	virtual bool initShader(const int type) = 0;
	virtual bool bindShader(const int type) = 0;
	virtual void releaseShader(const int type) = 0;
};
