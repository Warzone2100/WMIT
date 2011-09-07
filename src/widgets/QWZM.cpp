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

#include "QWZM.hpp"

#include <QString>

#include "IGLTextureManager.hpp"

QWZM::QWZM()
{
	defaultConstructor();
}

QWZM::QWZM(const Pie3Model& p3)
	: WZM(p3)
{
	defaultConstructor();
}

void QWZM::operator =(const WZM& wzm)
{
	if (m_texture != 0)
	{
		deleteTexture(m_texture);
	}
	if (m_tcm != 0)
	{
		deleteTexture(m_texture);
	}
	WZM::operator=(wzm);
}

void QWZM::render()
{
	const GLfloat tcColour[4] = {160/255.f,32/255.f,240/255.f,255/255.f}; // temporary...
	const bool tcmask = currentTCMaskMode() != None && m_tcm != 0;

	std::vector<Mesh>::iterator it;

	GLint frontFace;
	glGetIntegerv(GL_FRONT_FACE, &frontFace);


	glPushAttrib(GL_TEXTURE_BIT);
	glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_texture);

	glClientActiveTexture(GL_TEXTURE0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glEnableClientState(GL_VERTEX_ARRAY);

	glPushMatrix();

	if (frontFace != winding)
	{
		glFrontFace(winding);
	}
	glScalef(-1/128.f, 1/128.f, 1/128.f); // Scale from warzone to fit in our scene. possibly a FIXME

	glScalef(scale_all * scale_xyz[0], scale_all * scale_xyz[1], scale_all * scale_xyz[2]);

	if (tcmask)
	{
		setTCMaskEnvironment(tcColour);

		if (currentTCMaskMode() == FixedPipeline)
		{
			glClientActiveTexture(GL_TEXTURE1);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		else
		{
			glActiveTexture(GL_TEXTURE1);
			glEnable(GL_TEXTURE_2D);
		}
		glBindTexture(GL_TEXTURE_2D, m_tcm);
	}

	for (it = m_meshes.begin(); it != m_meshes.end(); ++it)
	{
		static_assert(sizeof(WZMUV) == sizeof(GLfloat)*2, "WZMUV has become fat.");
		glTexCoordPointer(2, GL_FLOAT, 0, &it->m_textureArrays[0][0]);

		if (currentTCMaskMode() == FixedPipeline)
		{
			glClientActiveTexture(GL_TEXTURE0);
			glTexCoordPointer(2, GL_FLOAT, 0, &it->m_textureArrays[0][0]);
			glClientActiveTexture(GL_TEXTURE1);
		}

		static_assert(sizeof(WZMVertex) == sizeof(GLfloat)*3, "WZMVertex has become fat.");
		glVertexPointer(3, GL_FLOAT, 0, &it->m_vertexArray[0]);

		static_assert(sizeof(IndexedTri) == sizeof(GLushort)*3, "IndexedTri has become fat.");
		glDrawElements(GL_TRIANGLES, it->m_indexArray.size() * 3, GL_UNSIGNED_SHORT, &it->m_indexArray[0]);
	}

	resetTCMaskEnvironment();

	if (frontFace != winding)
	{
		glFrontFace(GL_CCW);
	}

	glPopMatrix();
	glPopClientAttrib();
	glPopAttrib();
}

void QWZM::animate()
{

}

void QWZM::setRenderTexture(QString fileName)
{
	if (m_texture != 0)
	{
		deleteTexture(m_texture);
	}
	m_texture = createTexture(fileName).id();
}

void QWZM::setTextureManager(IGLTextureManager * manager)
{
	QString tex_fileName, tcm_fileName;

	if (m_texture != 0)
	{
		tex_fileName = idToFilePath(m_texture);
		deleteTexture(m_texture);
	}
	if (m_tcm != 0)
	{
		tcm_fileName = idToFilePath(m_tcm);
		deleteTexture(m_tcm);
	}

	ATexturedRenderable::setTextureManager(manager);

	if (m_texture != 0)
	{
		m_texture = createTexture(tex_fileName).id();
	}
	if (m_tcm != 0)
	{
		m_texture = createTexture(tcm_fileName).id();
	}
}

void QWZM::setTCMaskTexture(QString fileName)
{
	if (m_tcm != 0)
	{
		deleteTexture(m_tcm);
	}
	m_tcm = createTexture(fileName).id();
}

inline void QWZM::defaultConstructor()
{
	m_texture = 0;
	m_tcm = 0;
	scale_all = 1.f;
	scale_xyz[0] = 1.f;
	scale_xyz[1] = 1.f;
	scale_xyz[2] = 1.f;
	winding = GL_CW;
}

QWZM::~QWZM()
{
	if (m_texture != 0)
	{
		deleteTexture(m_texture);
	}
	if (m_tcm != 0)
	{
		deleteTexture(m_texture);
	}
}

void QWZM::setScaleXYZ(GLfloat xyz)
{
	scale_all = xyz;
}

void QWZM::setScaleX(GLfloat x)
{
	scale_xyz[0] = x;
}

void QWZM::setScaleY(GLfloat y)
{
	scale_xyz[1] = y;
}

void QWZM::setScaleZ(GLfloat z)
{
	scale_xyz[2] = z;
}

void QWZM::reverseWindings()
{
	if (winding == GL_CCW)
	{
		winding = GL_CW;
	}
	else
	{
		winding = GL_CCW;
	}
}
