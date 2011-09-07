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

#ifndef GLTEXTURE_HPP
#define GLTEXTURE_HPP

#include <GL/gl.h>

class GLTexture
{
	GLuint m_id; // opengl id
	GLsizei m_w, m_h; // dimensions
public:
	GLTexture();
	GLTexture(GLuint id, GLsizei w, GLsizei h);
	GLuint id() const;
	GLsizei width() const;
	GLsizei height() const;
};
#endif // GLTEXTURE_HPP
