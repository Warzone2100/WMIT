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

#ifndef ITCMASKPROVIDER_HPP
#define ITCMASKPROVIDER_HPP

#include <GL/gl.h>

enum TCMaskMethod
{
	None = 0,
	FixedPipeline = 1,
	Shaders = 2
};

class ITCMaskProvider
{
public:
	virtual ~ITCMaskProvider(){}

	/// Return a bitfield containing supported tcmask methods
	virtual unsigned tcmaskSupport() const = 0;
	virtual TCMaskMethod currentTCMaskMode() const = 0;
	virtual void setTCMaskEnvironment(const GLfloat tcmaskColour[4]) = 0;
	virtual void resetTCMaskEnvironment() = 0;
};

#endif // ITCMASKPROVIDER_HPP
