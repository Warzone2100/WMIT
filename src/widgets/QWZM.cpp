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
#include "Pie.hpp"

#include <QString>

#include "IGLTextureManager.hpp"

#ifdef CPP0X_AVAILABLE
#  define CPP0X_FEATURED(x) x
#else
#  define CPP0X_FEATURED(x) do {} while (0)
#endif

const GLint QWZM::winding = GL_CW;

QWZM::QWZM(QObject *parent): QObject(parent)
{
	defaultConstructor();
}

QWZM::~QWZM()
{
	clear();
}

void QWZM::render()
{
	const GLfloat tcColour[4] = {160/255.f,32/255.f,240/255.f,255/255.f}; // temporary...
	const bool tcmask = currentTCMaskMode() != None && m_tcm != 0;

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

	// check for desired
	if (frontFace != winding)
	{
		glFrontFace(winding);
	}

	glScalef(-1/128.f, 1/128.f, 1/128.f); // Scale from warzone to fit in our scene. possibly a FIXME

	drawCenterPoint();

	if (m_active_mesh < 0)
	{
		glScalef(scale_all * scale_xyz[0], scale_all * scale_xyz[1], scale_all * scale_xyz[2]);

	}

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

	for (int i = 0; i < (int)m_meshes.size(); ++i)
	{
		const Mesh& msh = m_meshes.at(i);
		if (m_active_mesh == i)
		{
			glPushMatrix();
			glScalef(scale_all * scale_xyz[0], scale_all * scale_xyz[1], scale_all * scale_xyz[2]);
		}

		CPP0X_FEATURED(static_assert(sizeof(WZMUV) == sizeof(GLfloat)*2, "WZMUV has become fat."));
		glTexCoordPointer(2, GL_FLOAT, 0, &msh.m_textureArrays[0][0]);

		if (currentTCMaskMode() == FixedPipeline)
		{
			glClientActiveTexture(GL_TEXTURE0);
			glTexCoordPointer(2, GL_FLOAT, 0, &msh.m_textureArrays[0][0]);
			glClientActiveTexture(GL_TEXTURE1);
		}

		CPP0X_FEATURED(static_assert(sizeof(WZMVertex) == sizeof(GLfloat)*3, "WZMVertex has become fat."));
		glVertexPointer(3, GL_FLOAT, 0, &msh.m_vertexArray[0]);

		CPP0X_FEATURED(static_assert(sizeof(IndexedTri) == sizeof(GLushort)*3, "IndexedTri has become fat."));
		glDrawElements(GL_TRIANGLES, msh.m_indexArray.size() * 3, GL_UNSIGNED_SHORT, &msh.m_indexArray[0]);

		if (m_active_mesh == i)
		{
			glPopMatrix();
		}
	}

	resetTCMaskEnvironment();

	// set it back
	if (frontFace != winding)
	{
		glFrontFace(frontFace);
	}

	glPopMatrix();
	glPopClientAttrib();
	glPopAttrib();
}

void QWZM::drawCenterPoint()
{
#ifdef _DEBUG
	WZMVertex center;

	if (m_active_mesh < 0)
	{
		center = calculateCenterPoint();
	}
	else
	{
		center = m_meshes.at(m_active_mesh).getCenterPoint();
	}

	const float lineLength = 40.0;
	GLfloat x, y, z;
	x = center.x() * scale_all * scale_xyz[0];
	y = center.y() * scale_all * scale_xyz[1];
	z = center.z() * scale_all * scale_xyz[2];

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(2);
	glColor3f(1.f, 1.f, 1.f);

	glBegin(GL_LINES);
	glVertex3f(-lineLength + x, y, z);
	glVertex3f(lineLength + x, y, z);
	glVertex3f(x, -lineLength + y, z);
	glVertex3f(x, lineLength + y, z);
	glVertex3f(x, y, -lineLength + z);
	glVertex3f(x, y, lineLength + z);
	glEnd();

	glEnable(GL_TEXTURE_2D);
#endif
}

void QWZM::animate()
{

}

void QWZM::clear()
{
	WZM::clear();

	clearRenderTexture();
	clearTCMaskTexture();

	defaultConstructor();

	meshCountChanged(meshes(), getMeshNames());
}

void QWZM::setRenderTexture(QString fileName)
{
	clearRenderTexture();
	m_texture = createTexture(fileName).id();
}

void QWZM::clearRenderTexture()
{
	if (m_texture)
	{
		deleteTexture(m_texture);
		m_texture = 0;
	}
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
		m_tcm = createTexture(tcm_fileName).id();
	}
}

void QWZM::setTCMaskTexture(QString fileName)
{
	clearTCMaskTexture();
	m_tcm = createTexture(fileName).id();
}

void QWZM::clearTCMaskTexture()
{
	if (m_tcm)
	{
		deleteTexture(m_tcm);
		m_tcm = 0;
	}
}

inline void QWZM::defaultConstructor()
{
	m_texture = 0;
	m_tcm = 0;

	scale_all = 1.f;
	scale_xyz[0] = 1.f;
	scale_xyz[1] = 1.f;
	scale_xyz[2] = 1.f;

	m_active_mesh = -1;
}

QStringList QWZM::getMeshNames()
{
	QStringList names;
	std::vector<Mesh>::const_iterator it;

	for (it = m_meshes.begin(); it != m_meshes.end(); ++it)
	{
		names.append(QString::fromStdString(it->getName()));
	}

	return names;
}

/*********** SLOTS ************/

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

void QWZM::slotMirrorAxis(int axis)
{
	mirror(axis, m_active_mesh);
}

void QWZM::applyTransformations()
{
	scale(scale_all * scale_xyz[0], scale_all * scale_xyz[1], scale_all * scale_xyz[2], m_active_mesh);
}

void QWZM::setActiveMesh(int mesh)
{
	m_active_mesh = mesh;
}


/************** Mesh control wrappers *****************/

void QWZM::operator=(const WZM& wzm)
{
	clear();
	WZM::operator=(wzm);
	meshCountChanged(meshes(), getMeshNames());
}

QWZM::operator Pie3Model() const
{
	return WZM::operator Pie3Model();
}

void QWZM::addMesh(const Mesh& mesh)
{
	WZM::addMesh(mesh);
	meshCountChanged(meshes(), getMeshNames());
}

void QWZM::rmMesh(int index)
{
	WZM::rmMesh(index);
	meshCountChanged(meshes(), getMeshNames());
}

bool QWZM::importFromOBJ(std::istream& in)
{
	if (WZM::importFromOBJ(in))
	{
		meshCountChanged(meshes(), getMeshNames());
		return true;
	}

	clear();
	return false;
}

bool QWZM::importFrom3DS(std::string fileName)
{
	if (WZM::importFrom3DS(fileName))
	{
		meshCountChanged(meshes(), getMeshNames());
		return true;
	}

	clear();
	return false;
}
