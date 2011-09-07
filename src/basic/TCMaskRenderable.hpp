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

#ifndef TCMASKRENDERABLE_HPP
#define TCMASKRENDERABLE_HPP

#include <GL/gl.h>

#include "TexturedRenderable.hpp"
#include "ITCMaskProvider.hpp"

class ITCMaskRenderable : virtual public ITexturedRenderable
{
public:
	virtual ~ITCMaskRenderable(){}
	virtual void setTCMaskProvider(ITCMaskProvider* provider) = 0;
};

class ATCMaskRenderable : public ITCMaskRenderable, public ATexturedRenderable
{
	ITCMaskProvider * m_tcProv;
public:
	ATCMaskRenderable():m_tcProv(NULL){}
	ATCMaskRenderable(ITCMaskProvider* provider):m_tcProv(provider){}
	virtual ~ATCMaskRenderable(){}

	void setTCMaskProvider(ITCMaskProvider* provider){m_tcProv=provider;}
protected:
	inline void setTCMaskEnvironment(const GLfloat tcmColour[4]){if (m_tcProv!=NULL){m_tcProv->setTCMaskEnvironment(tcmColour);}}
	inline void resetTCMaskEnvironment(){if (m_tcProv!=NULL){m_tcProv->resetTCMaskEnvironment();}}
	inline TCMaskMethod currentTCMaskMode()const
	{if (m_tcProv!=NULL){return m_tcProv->currentTCMaskMode();} else{return None;}}
};

#endif // TCMASKRENDERABLE_HPP
