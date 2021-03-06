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
#include "WZLight.h"

static const char vertexAtributeName[] = "vertex";
static const char vertexNormalAtributeName[] = "vertexNormal";
static const char vertexTexCoordAtributeName[] = "vertexTexCoord";
static const char vertexTangentAtributeName[] = "vertexTangent";

const GLint QWZM::winding = GL_CW;

QWZM::QWZM(QObject *parent):
	QObject(parent),
	m_timeAnimationStarted(std::chrono::steady_clock::now()),
	m_tcmaskColour(0, 0x60, 0, 0xFF),
	m_drawNormals(false),
	m_drawTangentAndBitangent(false),
	m_drawCenterPoint(false),
	m_animation_elapsed_msecs(-1.),
	m_shadertime(0.f),
	m_drawConnectors(false),
	m_ecmState(0),
	m_alphatest(1),
	m_enableTangentsInShaders(true)
{
	defaultConstructor();
}

QWZM::~QWZM()
{
}

static QMatrix4x4 render_mtxModelView, render_mtxModelView_preAnim, render_mtxProj;
static QMatrix4x4 render_mtxMVP, render_mtxNM;
static QVector4D render_posSun;

void QWZM::render(const float* mtxModelView, const float* mtxProj, const float* posSun)
{
	int activeShader = getActiveShader();

	QOpenGLShaderProgram* shader = nullptr;

	glPushAttrib(GL_TEXTURE_BIT);

	// prepare shader data
	if (!setupTextureUnits(activeShader))
	{
		glPopAttrib();
		return;
	}

	glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
	glPushMatrix();

	static const float WZ_SCALE = 1/128.f;

	if (!isFixedPipelineRenderer())
	{
		const static QVector4D wz_scale(-WZ_SCALE, WZ_SCALE, WZ_SCALE, 1.f);
		const static QVector4D pos_invert(-1.f, -1.f, -1.f, 1.f);

		render_mtxModelView = QMatrix4x4(mtxModelView).transposed();
		if (m_active_mesh < 0)
			render_mtxModelView.scale(scale_all * scale_xyz[0], scale_all * scale_xyz[1], scale_all * scale_xyz[2]);
		render_mtxModelView.scale(wz_scale.toVector3D());
		render_mtxModelView_preAnim = render_mtxModelView;

		render_mtxProj = QMatrix4x4(mtxProj).transposed();
		render_posSun = QVector4D(posSun[0], posSun[1], posSun[2], posSun[3]) * wz_scale;

		// Invert sun position for 4.0 shader, which is doing some invertion to workaround wz light weirdness
		if (activeShader == WZ_SHADER_WZ40)
			render_posSun *= pos_invert;
	}

	glScalef(-WZ_SCALE, WZ_SCALE, WZ_SCALE); // Scale from warzone to fit in our scene. possibly a FIXME

	// actual draw code starts here

	glClientActiveTexture(GL_TEXTURE0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);

	GLint frontFace;
	glGetIntegerv(GL_FRONT_FACE, &frontFace);
	// check for desired
	if (frontFace != winding)
	{
		glFrontFace(winding);
	}

	if (m_active_mesh < 0)
	{
		glScalef(scale_all * scale_xyz[0], scale_all * scale_xyz[1], scale_all * scale_xyz[2]);
	}

	QMatrix4x4 origMshMV = render_mtxModelView;

	for (size_t i = 0; i < m_meshes.size(); ++i)
	{
		const Mesh& msh = m_meshes.at(i);

		glColor3f(1.f, 1.f, 1.f);

		glPushMatrix();

		if (m_active_mesh == static_cast<int>(i))
		{
			glScalef(scale_all * scale_xyz[0], scale_all * scale_xyz[1], scale_all * scale_xyz[2]);
			if (!isFixedPipelineRenderer())
				render_mtxModelView.scale(scale_all * scale_xyz[0], scale_all * scale_xyz[1], scale_all * scale_xyz[2]);
		}

		if ((m_animation_elapsed_msecs >= 0.) && !msh.m_frameArray.empty())
		{
			const size_t animframe_fromtime = static_cast<size_t>(m_animation_elapsed_msecs /
								  static_cast<double>(msh.m_frame_time));
			const size_t animframe_to_draw = animframe_fromtime % msh.m_frameArray.size();
			const Frame& curAnimFrame = msh.m_frameArray[animframe_to_draw];

			// disabled frame if negative, for implementing key frame animation
			if (curAnimFrame.scale.x() >= 0)
			{
				render_mtxModelView_preAnim = render_mtxModelView;

				glTranslatef(curAnimFrame.trans.x(), curAnimFrame.trans.y(), curAnimFrame.trans.z());
				glRotatef(curAnimFrame.rot.x(), 1.f, 0.f, 0.f);
				glRotatef(curAnimFrame.rot.y(), 0.f, 1.f, 0.f);
				glRotatef(curAnimFrame.rot.z(), 0.f, 0.f, 1.f);
				glScalef(curAnimFrame.scale.x(), curAnimFrame.scale.y(), curAnimFrame.scale.z());

				if (!isFixedPipelineRenderer())
				{
					render_mtxModelView.translate(curAnimFrame.trans.x(), curAnimFrame.trans.y(), curAnimFrame.trans.z());
					render_mtxModelView.rotate(curAnimFrame.rot.x(), 1.f, 0.f, 0.f);
					render_mtxModelView.rotate(curAnimFrame.rot.y(), 0.f, 1.f, 0.f);
					render_mtxModelView.rotate(curAnimFrame.rot.z(), 0.f, 0.f, 1.f);
					render_mtxModelView.scale(curAnimFrame.scale.x(), curAnimFrame.scale.y(), curAnimFrame.scale.z());
				}
			}
		}

		// prepare shader data
		setupTextureUnits(activeShader);

		if (!isFixedPipelineRenderer())
		{
			if (bindShader(activeShader))
			{
				shader = m_shaderman->getShader(activeShader);
				if (shader)
				{
					shader->enableAttributeArray(vertexAtributeName);
					shader->enableAttributeArray(vertexNormalAtributeName);
					shader->enableAttributeArray(vertexTexCoordAtributeName);
					shader->enableAttributeArray(vertexTangentAtributeName);

					shader->setAttributeArray(vertexAtributeName, msh.m_vertexArray[0], 3);
					shader->setAttributeArray(vertexTexCoordAtributeName, msh.m_textureArray[0], 2);
					shader->setAttributeArray(vertexNormalAtributeName, msh.m_normalArray[0], 3);
					shader->setAttributeArray(vertexTangentAtributeName, msh.m_tangentArray[0], 4);
				}
			}
		}

		glMaterialfv(GL_FRONT, GL_EMISSION, m_material.vals[WZM_MAT_EMISSIVE]);
		glMaterialfv(GL_FRONT, GL_AMBIENT, m_material.vals[WZM_MAT_AMBIENT]);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, m_material.vals[WZM_MAT_DIFFUSE]);
		glMaterialfv(GL_FRONT, GL_SPECULAR, m_material.vals[WZM_MAT_SPECULAR]);
		glMaterialf(GL_FRONT, GL_SHININESS, m_material.shininess);

		static_assert(sizeof(WZMUV) == sizeof(GLfloat)*2, "WZMUV has become fat.");
		glTexCoordPointer(2, GL_FLOAT, 0, &msh.m_textureArray[0]);

		glNormalPointer(GL_FLOAT, 0, &msh.m_normalArray[0]);

		static_assert(sizeof(WZMVertex) == sizeof(GLfloat)*3, "WZMVertex has become fat.");
		glVertexPointer(3, GL_FLOAT, 0, &msh.m_vertexArray[0]);

		static_assert(sizeof(IndexedTri) == sizeof(GLushort)*3, "IndexedTri has become fat.");
		glDrawElements(GL_TRIANGLES, static_cast<int>(msh.m_indexArray.size()) * 3, GL_UNSIGNED_SHORT, &msh.m_indexArray[0]);

		if (!isFixedPipelineRenderer())
		{
			render_mtxModelView = origMshMV;
			render_mtxModelView_preAnim = render_mtxModelView;

			// release shader data
			if (shader)
			{
				shader->disableAttributeArray(vertexAtributeName);
				shader->disableAttributeArray(vertexNormalAtributeName);
				shader->disableAttributeArray(vertexTexCoordAtributeName);
				shader->disableAttributeArray(vertexTangentAtributeName);
			}
			releaseShader(activeShader);
		}

		clearTextureUnits(activeShader);

		if (m_drawNormals)
			drawNormals(i, m_drawTangentAndBitangent);
		if (m_drawConnectors)
			drawConnectors(i);

		glPopMatrix();
	}

	// set it back
	if (frontFace != winding)
	{
		glFrontFace(frontFace);
	}

	// after shaders
	if (!hasAnimObject())
	{
		if (m_drawCenterPoint)
			drawCenterPoint();
	}

	glPopMatrix();
	glPopClientAttrib();
	glPopAttrib();
}

void QWZM::drawAPoint(const WZMVertex& center, const WZMVertex& scale, const WZMVertex& color, const float lineLength)
{
	GLfloat x, y, z;
	x = center.x() * scale[0];
	y = center.y() * scale[1];
	z = center.z() * scale[2];

	GLboolean lighting, texture;

	glGetBooleanv(GL_LIGHTING, &lighting);
	if (lighting)
		glDisable(GL_LIGHTING);

	texture = glIsEnabled(GL_TEXTURE_2D);
	if (texture)
		glDisable(GL_TEXTURE_2D);

	glEnable(GL_LINE_SMOOTH);
	glLineWidth(2);

	glColor3f(color.x(), color.y(), color.z());

	glBegin(GL_LINES);
	glVertex3f(-lineLength + x, y, z);
	glVertex3f(lineLength + x, y, z);
	glVertex3f(x, -lineLength + y, z);
	glVertex3f(x, lineLength + y, z);
	glVertex3f(x, y, -lineLength + z);
	glVertex3f(x, y, lineLength + z);
	glEnd();

	if (texture)
		glEnable(GL_TEXTURE_2D);

	if (lighting)
		glEnable(GL_LIGHTING);
}

void QWZM::drawCenterPoint()
{
	WZMVertex center, scale;

	if (m_active_mesh < 0 || !m_meshes.size())
	{
		center = calculateCenterPoint();
		scale = WZMVertex(1.f, 1.f, 1.f);
	}
	else
	{
		center = m_meshes.at(m_active_mesh).getCenterPoint();
		scale = WZMVertex(scale_xyz[0], scale_xyz[1], scale_xyz[2]) * scale_all;
	}

	const static WZMVertex whiteCol = WZMVertex(1.f, 1.f, 1.f);
	drawAPoint(center, scale, whiteCol, 40.f);
}

void QWZM::drawNormals(size_t mesh_idx, bool draw_tb)
{
	GLboolean lighting, texture;

	glGetBooleanv(GL_LIGHTING, &lighting);
	if (lighting)
		glDisable(GL_LIGHTING);

	texture = glIsEnabled(GL_TEXTURE_2D);
	if (texture)
		glDisable(GL_TEXTURE_2D);

	glColor3f(0.7f, 1.0f, 0.7f);

	WZMVertex nrm, tb;
	qglviewer::Vec from, to;

	const Mesh& msh = m_meshes.at(mesh_idx);
	for (size_t j = 0; j < msh.m_vertexArray.size(); ++j)
	{
		nrm = msh.m_normalArray[j].normalize() * 2. / scale_all;
		from = qglviewer::Vec(msh.m_vertexArray[j].x(), msh.m_vertexArray[j].y(), msh.m_vertexArray[j].z());

		if (draw_tb)
			glColor3f(0.7f, 1.0f, 0.7f);
		to = qglviewer::Vec(from + qglviewer::Vec(nrm.x(), nrm.y(), nrm.z()));
		QGLViewer::drawArrow(from, to);

		if (draw_tb)
		{
			const WZMVertex4& tngt = msh.m_tangentArray[j];
			tb = WZMVertex(tngt.x(), tngt.y(), tngt.z()).normalize() * 2. / scale_all;

			glColor3f(1.0f, 0.7f, 0.7f);
			to = qglviewer::Vec(from + qglviewer::Vec(tb.x(), tb.y(), tb.z()));
			QGLViewer::drawArrow(from, to);

			tb = msh.m_bitangentArray[j].normalize() * 2. / scale_all;

			glColor3f(0.7f, 0.7f, 1.0f);
			to = qglviewer::Vec(from + qglviewer::Vec(tb.x(), tb.y(), tb.z()));
			QGLViewer::drawArrow(from, to);
		}
	}

	if (texture)
		glEnable(GL_TEXTURE_2D);

	if (lighting)
		glEnable(GL_LIGHTING);
}

void QWZM::drawConnectors(size_t mesh_idx)
{
	static const WZMVertex scale(1.f, 1.f, 1.f);

	const Mesh& msh = m_meshes.at(mesh_idx);

	for (size_t j = 0; j < msh.connectors(); ++j)
	{
		size_t con_idx = 0;
		for (auto itC = msh.m_connectors.begin(); itC != msh.m_connectors.end(); ++itC)
		{
			drawAPoint(itC->getPos(), scale, CONNECTOR_COLORS[con_idx % MAX_CONNECTOR_COLORS], 20.f);
			++con_idx;
		}
	}
}

void QWZM::animate()
{
	using namespace std::chrono;
	duration<double> time_span = steady_clock::now() - m_timeAnimationStarted;
	m_animation_elapsed_msecs = time_span.count() * 1000.;

	// Do it like they do it in WZ
	uint32_t base = static_cast<uint32_t>(m_animation_elapsed_msecs) % 1000;
	if (base > 500)
		base = 1000 - base;	// cycle
	m_shadertime = base / 1000.0f;
}

void QWZM::clear()
{
	meshCountChanged();

	WZM::clear();

	clearGLRenderTextures();

	defaultConstructor();
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

void QWZM::setTCMaskColor(const QColor &tcmaskColour)
{
    m_tcmaskColour = tcmaskColour;
}

QColor QWZM::getTCMaskColor()
{
    return m_tcmaskColour;
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
	case WZ_SHADER_WZ31:
		str = "WZ 3.1";
		break;
	case WZ_SHADER_WZ32:
		str = "WZ 3.2";
		break;
	case WZ_SHADER_WZ33:
		str = "WZ 3.3/3.4";
		break;
	case WZ_SHADER_WZ40:
		str = "WZ 4.0";
		break;
	default:
		str = "<Unknown>";
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
	case WZ_SHADER_WZ31:
	case WZ_SHADER_WZ32:
	case WZ_SHADER_WZ33:
	case WZ_SHADER_WZ40:
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
	case WZ_SHADER_WZ31:
	case WZ_SHADER_WZ32:
	case WZ_SHADER_WZ33:
	case WZ_SHADER_WZ40:
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

	QOpenGLShaderProgram* shader = m_shaderman->getShader(type);

	if (!shader || !shader->bind())
		return false;

	int uniLoc;

	{
		int baseTexLoc, tcTexLoc, nmTexLoc, smTexLoc;

		switch (type)
		{
		case WZ_SHADER_WZ31:
			baseTexLoc = shader->uniformLocation("Texture0");
			tcTexLoc = shader->uniformLocation("Texture1");
			nmTexLoc = shader->uniformLocation("Texture2");
			smTexLoc = shader->uniformLocation("Texture3");
			break;
		case WZ_SHADER_WZ32:
		case WZ_SHADER_WZ33:
		case WZ_SHADER_WZ40:
			baseTexLoc = shader->uniformLocation("Texture");
			tcTexLoc = shader->uniformLocation("TextureTcmask");
			nmTexLoc = shader->uniformLocation("TextureNormal");
			smTexLoc = shader->uniformLocation("TextureSpecular");
			break;
		default:
			return false;
		}

		shader->setUniformValue(baseTexLoc, GLint(0));
		shader->setUniformValue(tcTexLoc, GLint(1));
		shader->setUniformValue(nmTexLoc, GLint(2));
		shader->setUniformValue(smTexLoc, GLint(3));
	}


	uniLoc = shader->uniformLocation("fogEnabled");
	shader->setUniformValue(uniLoc, GLint(0));

	switch (type)
	{
	case WZ_SHADER_WZ32:
	case WZ_SHADER_WZ33:
	case WZ_SHADER_WZ40:
		uniLoc = shader->uniformLocation("alphaTest");
		shader->setUniformValue(uniLoc, GLint(0));

		uniLoc = shader->uniformLocation("colour");
		shader->setUniformValue(uniLoc,	1.f, 1.f, 1.f, 1.f);

		break;
	}

	shader->release();
	return true;
}

bool QWZM::bindShader(int type)
{
	if (!m_shaderman)
		return false;

	QOpenGLShaderProgram* shader = m_shaderman->getShader(type);

	if (!shader || !shader->bind())
		return false;

	int uniloc;

	uniloc = shader->uniformLocation("hasTangents");
	if (uniloc >= 0)
		shader->setUniformValue(uniloc, GLint(m_enableTangentsInShaders));

	uniloc = shader->uniformLocation("tcmask");
	if (hasGLRenderTexture(WZM_TEX_TCMASK))
	{
		shader->setUniformValue(uniloc, GLint(1));
		uniloc = shader->uniformLocation("teamcolour");
		shader->setUniformValue(uniloc,
					m_tcmaskColour.redF(), m_tcmaskColour.greenF(),
					m_tcmaskColour.blueF(), m_tcmaskColour.alphaF());
	}
	else
		shader->setUniformValue(uniloc, GLint(0));

	uniloc = shader->uniformLocation("normalmap");
	if (hasGLRenderTexture(WZM_TEX_NORMALMAP))
		shader->setUniformValue(uniloc, GLint(1));
	else
		shader->setUniformValue(uniloc, GLint(0));

	uniloc = shader->uniformLocation("specularmap");
	if (hasGLRenderTexture(WZM_TEX_SPECULAR))
		shader->setUniformValue(uniloc, GLint(1));
	else
		shader->setUniformValue(uniloc, GLint(0));

	uniloc = shader->uniformLocation("graphicsCycle");
	shader->setUniformValue(uniloc, GLfloat(m_shadertime));

	uniloc = shader->uniformLocation("ecmEffect");
	shader->setUniformValue(uniloc, GLint(m_ecmState));

	switch (type)
	{
	case WZ_SHADER_WZ32:
	case WZ_SHADER_WZ33:
	case WZ_SHADER_WZ40:
		uniloc = shader->uniformLocation("ModelViewMatrix");
		shader->setUniformValue(uniloc,	render_mtxModelView);

		render_mtxMVP = render_mtxProj * render_mtxModelView;

		uniloc = shader->uniformLocation("ModelViewProjectionMatrix");
		shader->setUniformValue(uniloc,	render_mtxMVP);

		render_mtxNM = render_mtxModelView.inverted().transposed();

		uniloc = shader->uniformLocation("NormalMatrix");
		shader->setUniformValue(uniloc,	render_mtxNM);

		uniloc = shader->uniformLocation("lightPosition");
		shader->setUniformValue(uniloc,	render_posSun * render_mtxModelView_preAnim.inverted());

		uniloc = shader->uniformLocation("sceneColor");
		shader->setUniformValue(uniloc,	lightCol0[LIGHT_EMISSIVE][0], lightCol0[LIGHT_EMISSIVE][1],
				lightCol0[LIGHT_EMISSIVE][2], lightCol0[LIGHT_EMISSIVE][3]);

		uniloc = shader->uniformLocation("ambient");
		shader->setUniformValue(uniloc,	lightCol0[LIGHT_AMBIENT][0], lightCol0[LIGHT_AMBIENT][1],
				lightCol0[LIGHT_AMBIENT][2], lightCol0[LIGHT_AMBIENT][3]);

		uniloc = shader->uniformLocation("diffuse");
		shader->setUniformValue(uniloc,	lightCol0[LIGHT_DIFFUSE][0], lightCol0[LIGHT_DIFFUSE][1],
				lightCol0[LIGHT_DIFFUSE][2], lightCol0[LIGHT_DIFFUSE][3]);

		uniloc = shader->uniformLocation("specular");
		shader->setUniformValue(uniloc,	lightCol0[LIGHT_SPECULAR][0], lightCol0[LIGHT_SPECULAR][1],
				lightCol0[LIGHT_SPECULAR][2], lightCol0[LIGHT_SPECULAR][3]);

/*
		uniloc = shader->uniformLocation("sceneColor");
		shader->setUniformValue(uniloc,
				m_material.vals[WZM_MAT_EMISSIVE][0],
				m_material.vals[WZM_MAT_EMISSIVE][1],
				m_material.vals[WZM_MAT_EMISSIVE][2],
				m_material.vals[WZM_MAT_EMISSIVE][3]);

		uniloc = shader->uniformLocation("ambient");
		shader->setUniformValue(uniloc,
				m_material.vals[WZM_MAT_AMBIENT][0],
				m_material.vals[WZM_MAT_AMBIENT][1],
				m_material.vals[WZM_MAT_AMBIENT][2],
				m_material.vals[WZM_MAT_AMBIENT][3]);

		uniloc = shader->uniformLocation("diffuse");
		shader->setUniformValue(uniloc,
				m_material.vals[WZM_MAT_DIFFUSE][0],
				m_material.vals[WZM_MAT_DIFFUSE][1],
				m_material.vals[WZM_MAT_DIFFUSE][2],
				m_material.vals[WZM_MAT_DIFFUSE][3]);

		uniloc = shader->uniformLocation("specular");
		shader->setUniformValue(uniloc,
				m_material.vals[WZM_MAT_SPECULAR][0],
				m_material.vals[WZM_MAT_SPECULAR][1],
				m_material.vals[WZM_MAT_SPECULAR][2],
				m_material.vals[WZM_MAT_SPECULAR][3]);
*/
		uniloc = shader->uniformLocation("shininess");
		if (uniloc >= 0)
			shader->setUniformValue(uniloc,	m_material.shininess);

		uniloc = shader->uniformLocation("alphaTest");
		if (uniloc >= 0)
			shader->setUniformValue(uniloc,	m_alphatest);

		break;
	}

	return true;
}

void QWZM::releaseShader(int type)
{
	if (m_shaderman)
	{
		QOpenGLShaderProgram* shader = m_shaderman->getShader(type);
		if (shader)
		{
			shader->release();
			return;
		}
	}

	// glUseProgram(0); // should not be needed, as shader->release() does this
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

void QWZM::slotRecalculateTB()
{
	applyTransformations();
	recalculateTB(m_active_mesh);
}

void QWZM::applyTransformations()
{
	if (!m_pending_changes)
		return;

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

void QWZM::setDrawTangentAndBitangentFlag(bool draw)
{
	m_drawTangentAndBitangent = draw;
}

void QWZM::setDrawCenterPointFlag(bool draw)
{
	m_drawCenterPoint = draw;
}

void QWZM::setDrawConnectors(bool draw)
{
	m_drawConnectors = draw;
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

void QWZM::setEcmState(bool enable)
{
	m_ecmState = enable ? 1 : 0;
}

void QWZM::slotRemoveActiveMesh()
{
	auto meshIdx = m_active_mesh;
	if (meshIdx < 0 || meshIdx >= meshes())
		return;

	meshCountChanged();
	WZM::rmMesh(meshIdx);
	meshCountChanged(meshes(), getMeshNames());
}

bool QWZM::importFromOBJ(std::istream& in, bool welder)
{
	if (WZM::importFromOBJ(in, welder))
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
