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

#include "QWZM.h"
#include "Pie.h"

#include "QtGLView.h"

#ifdef CPP0X_AVAILABLE
#  define CPP0X_FEATURED(x) x
#else
#  define CPP0X_FEATURED(x) do {} while (0)
#endif

static const char tangentAtributeName[] = "tangent";

const GLint QWZM::winding = GL_CCW;

QWZM::QWZM(QObject *parent):
	QObject(parent), m_tcmaskColour(0, 0x60, 0, 0xFF), m_drawNormals(false), m_drawCenterPoint(false)
{
	defaultConstructor();
}

QWZM::~QWZM()
{
	clear();
}

void QWZM::render()
{
	GLint frontFace;
	glGetIntegerv(GL_FRONT_FACE, &frontFace);

	glPushMatrix();
	glPushAttrib(GL_TEXTURE_BIT);
	glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

	glScalef(1/128.f, 1/128.f, 1/128.f); // Scale from warzone to fit in our scene. possibly a FIXME

	// before shaders
	if (m_drawCenterPoint)
		drawCenterPoint();
	if (m_drawNormals)
		drawNormals();

	// actual draw code starts here

	glColor3f(1.f, 1.f, 1.f);

	QGLShaderProgram* shader = 0;

	// prepare shader data
	if (setupTextureUnits(getActiveShader()))
	{
		if (!isFixedPipelineRenderer())
		{
			if (bindShader(getActiveShader()))
			{
				shader = m_shaderman->getShader(getActiveShader());
				if (shader)
				{
					shader->enableAttributeArray(tangentAtributeName);
				}
			}
		}
	}

	glClientActiveTexture(GL_TEXTURE0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);

	// check for desired
	if (frontFace != winding)
	{
		glFrontFace(winding);
	}

	if (m_active_mesh < 0)
	{
		glScalef(scale_all * scale_xyz[0], scale_all * scale_xyz[1], scale_all * scale_xyz[2]);
	}

	for (int i = 0; i < (int)m_meshes.size(); ++i)
	{
		const Mesh& msh = m_meshes.at(i);

		if (m_active_mesh == i)
		{
			glPushMatrix();
			glScalef(scale_all * scale_xyz[0], scale_all * scale_xyz[1], scale_all * scale_xyz[2]);
		}

		glMaterialfv(GL_FRONT, GL_EMISSION, m_material.vals[WZM_MAT_EMISSIVE]);
		glMaterialfv(GL_FRONT, GL_AMBIENT, m_material.vals[WZM_MAT_AMBIENT]);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, m_material.vals[WZM_MAT_DIFFUSE]);
		glMaterialfv(GL_FRONT, GL_SPECULAR, m_material.vals[WZM_MAT_SPECULAR]);
		glMaterialf(GL_FRONT, GL_SHININESS, m_material.shininess);

		CPP0X_FEATURED(static_assert(sizeof(WZMUV) == sizeof(GLfloat)*2, "WZMUV has become fat."));
		glTexCoordPointer(2, GL_FLOAT, 0, &msh.m_textureArray[0]);

		if (shader)
		{
			shader->setAttributeArray(tangentAtributeName, (GLfloat*)&msh.m_tangentArray[0], 4);
		}

		glNormalPointer(GL_FLOAT, 0, &msh.m_normalArray[0]);

		CPP0X_FEATURED(static_assert(sizeof(WZMVertex) == sizeof(GLfloat)*3, "WZMVertex has become fat."));
		glVertexPointer(3, GL_FLOAT, 0, &msh.m_vertexArray[0]);

		CPP0X_FEATURED(static_assert(sizeof(IndexedTri) == sizeof(GLushort)*3, "IndexedTri has become fat."));
		glDrawElements(GL_TRIANGLES, msh.m_indexArray.size() * 3, GL_UNSIGNED_SHORT, &msh.m_indexArray[0]);

		if (m_active_mesh == i)
		{
			glPopMatrix();
		}
	}

	// release shader data
	if (shader)
	{
		shader->disableAttributeArray(tangentAtributeName);
	}
	releaseShader(getActiveShader());
	clearTextureUnits(getActiveShader());

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
	WZMVertex center;

	if (m_active_mesh < 0 || !m_meshes.size())
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

	GLboolean lighting;
	glGetBooleanv(GL_LIGHTING, &lighting);
	if (lighting)
		glDisable(GL_LIGHTING);
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
	if (lighting)
	{
		glEnable(GL_LIGHTING);
	}
}

void QWZM::drawNormals()
{
	GLboolean lighting;
	glGetBooleanv(GL_LIGHTING, &lighting);

	if (lighting)
		glDisable(GL_LIGHTING);

	glDisable(GL_TEXTURE_2D);
	glColor3f(0.7f, 1.0f, 0.7f);

	for (int i = 0; i < (int)m_meshes.size(); ++i)
	{
		const Mesh& msh = m_meshes.at(i);
		WZMVertex nrm;

		for (int j = 0; j < (int)msh.m_vertexArray.size(); ++j)
		{
			nrm = msh.m_normalArray[j];// / 0.5; // FIXME: multiplier
			qglviewer::Vec from(msh.m_vertexArray[j].x(), msh.m_vertexArray[j].y(), msh.m_vertexArray[j].z());
			qglviewer::Vec to(msh.m_vertexArray[j].x() + nrm.x(),
					     msh.m_vertexArray[j].y() + nrm.y(),
					     msh.m_vertexArray[j].z() + nrm.z());
			QGLViewer::drawArrow(from, to);
		}
	}

	glEnable(GL_TEXTURE_2D);
	if (lighting)
		glEnable(GL_LIGHTING);
}

void QWZM::animate()
{

}

void QWZM::clear()
{
	WZM::clear();

	clearGLRenderTextures();

	defaultConstructor();

	meshCountChanged(meshes(), getMeshNames());
}

void QWZM::loadGLRenderTexture(wzm_texture_type_t type, QString fileName)
{
	unloadGLRenderTexture(type);
	m_gl_textures[type] = createTexture(fileName).id();
}

void QWZM::unloadGLRenderTexture(wzm_texture_type_t type)
{
	std::map<wzm_texture_type_t, GLuint>::iterator it;
	it = m_gl_textures.find(type);
	if (it != m_gl_textures.end())
	{
		deleteTexture(it->second);
		it->second = 0;
	}
}

bool QWZM::hasGLRenderTexture(wzm_texture_type_t type) const
{
	std::map<wzm_texture_type_t, GLuint>::const_iterator it;
	it = m_gl_textures.find(type);
	if (it != m_gl_textures.end() && it->second)
		return true;

	return false;
}

void QWZM::clearGLRenderTextures()
{
	std::map<wzm_texture_type_t, GLuint>::iterator it;
	for (it = m_gl_textures.begin(); it != m_gl_textures.end(); it++)
	{
		if (it->second)
		{
			deleteTexture(it->second);
			it->second = 0;
		}
	}
}

void QWZM::setTextureManager(IGLTextureManager * manager)
{
	std::map<wzm_texture_type_t, QString> texture_names;
	std::map<wzm_texture_type_t, QString>::iterator it_names;
	std::map<wzm_texture_type_t, GLuint>::iterator it;

	for (it = m_gl_textures.begin(); it != m_gl_textures.end(); it++)
	{
		if (it->second)
		{
			texture_names[it->first] = idToFilePath(it->second);
			deleteTexture(it->second);
			it->second = 0;
		}
	}

	IGLTexturedRenderable::setTextureManager(manager);

	for (it_names = texture_names.begin(); it_names != texture_names.end(); it_names++)
	{
		m_gl_textures[it_names->first] = createTexture(it_names->second).id();
	}
}

inline void QWZM::defaultConstructor()
{
	m_active_mesh = -1;

	resetAllPendingChanges();
}

QStringList QWZM::getMeshNames() const
{
	QStringList names;
	std::vector<Mesh>::const_iterator it;

	for (it = m_meshes.begin(); it != m_meshes.end(); ++it)
	{
		names.append(QString::fromStdString(it->getName()));
	}

	return names;
}

void QWZM::getTexturesMap(QMap<wzm_texture_type_t, QString>& map) const
{
	std::map<wzm_texture_type_t, std::string>::const_iterator it_names;

	map.clear();
	for (it_names = m_textures.begin(); it_names != m_textures.end(); ++it_names)
	{
		if (!it_names->second.empty())
		{
			map[it_names->first] = QString::fromStdString(it_names->second);
		}
	}
}

void QWZM::disableShaders()
{
	releaseShader(m_active_shader);
	m_active_shader = WZ_SHADER_NONE;
}

QString QWZM::shaderTypeToString(wz_shader_type_t type)
{
	QString str;

	switch (type)
	{
	case WZ_SHADER_NONE:
		str = "Fixed pipeline";
		break;
	case WZ_SHADER_PIE3:
		str = "PIE3 shader";
		break;
	case WZ_SHADER_PIE3_USER:
		str = "PIE3 shader (external)";
		break;
	default:
		str = "Unknown!";
	}

	return str;
}

inline bool QWZM::isFixedPipelineRenderer() const
{
	return getActiveShader() == WZ_SHADER_NONE;
}

static inline void activateAndBindTexture(int unit, GLuint texture)
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
}

static inline void deactivateTexture(int unit)
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glDisable(GL_TEXTURE_2D);
}

bool QWZM::setupTextureUnits(int type)
{
	switch (type)
	{
	case WZ_SHADER_PIE3:
	case WZ_SHADER_PIE3_USER:
		if (hasGLRenderTexture(WZM_TEX_DIFFUSE))
			activateAndBindTexture(0, m_gl_textures[WZM_TEX_DIFFUSE]);
		else
			return false;

		if (hasGLRenderTexture(WZM_TEX_TCMASK))
			activateAndBindTexture(1, m_gl_textures[WZM_TEX_TCMASK]);

		if (hasGLRenderTexture(WZM_TEX_NORMALMAP))
			activateAndBindTexture(2, m_gl_textures[WZM_TEX_NORMALMAP]);

		if (hasGLRenderTexture(WZM_TEX_SPECULAR))
			activateAndBindTexture(3, m_gl_textures[WZM_TEX_SPECULAR]);

		break;
	default:
		if (hasGLRenderTexture(WZM_TEX_DIFFUSE))
			activateAndBindTexture(0, m_gl_textures[WZM_TEX_DIFFUSE]);
		else
			return false;
	}

	return true;
}

void QWZM::clearTextureUnits(int type)
{
	switch (type)
	{
	case WZ_SHADER_PIE3:
	case WZ_SHADER_PIE3_USER:
		deactivateTexture(3);
		deactivateTexture(2);
		deactivateTexture(1);
		deactivateTexture(0);
		break;
	default:
		deactivateTexture(0);
	}
}

bool QWZM::initShader(int type)
{
	if (!m_shaderman)
		return false;

	QGLShaderProgram* shader = m_shaderman->getShader(type);

	if (!shader)
		return false;

	shader->bind();

	switch (type)
	{
	case WZ_SHADER_PIE3:
	case WZ_SHADER_PIE3_USER:
		int uniLoc, baseTexLoc, tcTexLoc, nmTexLoc, smTexLoc;

		baseTexLoc = shader->uniformLocation("Texture0");
		tcTexLoc = shader->uniformLocation("Texture1");
		nmTexLoc = shader->uniformLocation("Texture2");
		smTexLoc = shader->uniformLocation("Texture3");

		shader->setUniformValue(baseTexLoc, GLint(0));
		shader->setUniformValue(tcTexLoc, GLint(1));
		shader->setUniformValue(nmTexLoc, GLint(2));
		shader->setUniformValue(smTexLoc, GLint(3));

		uniLoc = shader->uniformLocation("fogEnabled");
		shader->setUniformValue(uniLoc, GLint(0));

		uniLoc = shader->uniformLocation("ecmEffect");
		shader->setUniformValue(uniLoc, GLint(0));

		break;
	}

	shader->release();
	return true;
}

bool QWZM::bindShader(int type)
{
	if (!m_shaderman)
		return false;

	QGLShaderProgram* shader = m_shaderman->getShader(type);

	if (!shader)
		return false;

	shader->bind();

	switch (type)
	{
	case WZ_SHADER_PIE3:
	case WZ_SHADER_PIE3_USER:
		int uniloc = shader->uniformLocation("tcmask");
		if (hasGLRenderTexture(WZM_TEX_TCMASK))
		{
			shader->setUniformValue(uniloc, GLint(1));
			uniloc = shader->uniformLocation("teamcolour");
			shader->setUniformValue(uniloc,
						m_tcmaskColour.redF(), m_tcmaskColour.greenF(), m_tcmaskColour.blueF(), m_tcmaskColour.alphaF());
		}
		else
		{
			shader->setUniformValue(uniloc, GLint(0));
		}

		uniloc = shader->uniformLocation("normalmap");
		if (hasGLRenderTexture(WZM_TEX_NORMALMAP))
		{
			shader->setUniformValue(uniloc, GLint(1));
		}
		else
		{
			shader->setUniformValue(uniloc, GLint(0));
		}

		uniloc = shader->uniformLocation("specularmap");
		if (hasGLRenderTexture(WZM_TEX_SPECULAR))
		{
			shader->setUniformValue(uniloc, GLint(1));
		}
		else
		{
			shader->setUniformValue(uniloc, GLint(0));
		}
		break;
	}

	return true;
}

void QWZM::releaseShader(int type)
{
	if (!m_shaderman)
		return;

	QGLShaderProgram* shader = m_shaderman->getShader(type);

	if (shader)
		shader->release();
	else
		glUseProgram(0);
}

/*********** SLOTS ************/

void QWZM::setScaleXYZ(GLfloat xyz)
{
	scale_all = xyz;
	m_pending_changes = true;
}

void QWZM::setScaleX(GLfloat x)
{
	scale_xyz[0] = x;
	m_pending_changes = true;
}

void QWZM::setScaleY(GLfloat y)
{
	scale_xyz[1] = y;
	m_pending_changes = true;
}

void QWZM::setScaleZ(GLfloat z)
{
	scale_xyz[2] = z;
	m_pending_changes = true;
}

void QWZM::slotMirrorAxis(int axis)
{
	mirror(axis, m_active_mesh);
}

void QWZM::applyTransformations()
{
	scale(scale_all * scale_xyz[0], scale_all * scale_xyz[1], scale_all * scale_xyz[2], m_active_mesh);

	// reset values
	resetAllPendingChanges();
}

void QWZM::setActiveMesh(int mesh)
{
	m_active_mesh = mesh;
}

void QWZM::applyPendingChangesToModel(WZM &model) const
{
	if (m_pending_changes)
	{
		model.scale(scale_all * scale_xyz[0], scale_all * scale_xyz[1], scale_all * scale_xyz[2], m_active_mesh);
	}
}

void QWZM::resetAllPendingChanges()
{
	scale_all = scale_xyz[0] = scale_xyz[1] = scale_xyz[2] = 1.;
	m_pending_changes = false;
}

void QWZM::setDrawNormalsFlag(bool draw)
{
	m_drawNormals = draw;
}

void QWZM::setDrawCenterPointFlag(bool draw)
{
	m_drawCenterPoint = draw;
}

/************** Mesh control wrappers *****************/

void QWZM::operator=(const WZM& wzm)
{
	clear();
	WZM::operator=(wzm);
	meshCountChanged(meshes(), getMeshNames());
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

// apply any pending transformations to temp object on export

QWZM::operator Pie3Model() const
{
	if (m_pending_changes)
	{
		WZM res = *this;
		applyPendingChangesToModel(res);
		return Pie3Model(res);
	}

	return WZM::operator Pie3Model();
}

void QWZM::write(std::ostream& out) const
{
	if (m_pending_changes)
	{
		WZM res = *this;
		applyPendingChangesToModel(res);
		res.write(out);
	}
	else
	{
		WZM::write(out);
	}
}

void QWZM::exportToOBJ(std::ostream& out) const
{
	if (m_pending_changes)
	{
		WZM res = *this;
		applyPendingChangesToModel(res);
		res.exportToOBJ(out);
	}
	else
	{
		WZM::exportToOBJ(out);
	}
}
