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

#include "QtGLView.hpp"

#include <QPixmap>
#include <QImage>
#include <QApplication>

#include <QGLShaderProgram>
#include <QtDebug>

#include <QGLViewer/vec.h>

#include "IGLTexturedRenderable.hpp"
#include "IGLShaderRenderable.h"

enum LIGHTING_TYPE {
	LIGHT_EMISSIVE, LIGHT_AMBIENT, LIGHT_DIFFUSE, LIGHT_SPECULAR, LIGHT_TYPE_MAX
};

static GLfloat lighting[LIGHT_TYPE_MAX][4] = {
	{0.0f, 0.0f, 0.0f, 1.0f},  {0.5f, 0.5f, 0.5f, 1.0f},  {0.8f, 0.8f, 0.8f, 1.0f},  {1.0f, 1.0f, 1.0f, 1.0f}
};


QtGLView::QtGLView(QWidget *parent) :
		QGLViewer(parent)
{
	connect(&textureUpdater, SIGNAL(fileChanged(QString)), this, SLOT(textureChanged(QString)));

	setShortcut(DISPLAY_FPS, 0); // Disable stuff that won't work.
	setGridIsDrawn(true);
	setAxisIsDrawn(true);
}

QtGLView::~QtGLView()
{
	foreach (ManagedGLTexture texture, m_textures)
	{
		const GLuint id = texture.id();
		QGLWidget::deleteTexture(id);
	}
}

void QtGLView::init()
{
	const float pos[4] = {225.0f, -600.0f, 450.0f, 0.0f};

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lighting[LIGHT_EMISSIVE]);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
	glLightfv(GL_LIGHT0, GL_POSITION, pos);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lighting[LIGHT_AMBIENT]);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lighting[LIGHT_DIFFUSE]);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lighting[LIGHT_SPECULAR]);
	glEnable(GL_LIGHT0);

	glDisable(GL_LIGHTING); // QGLViewer likes enabling this stuff
	glDisable(GL_COLOR_MATERIAL);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GEQUAL, 0.05f);

	glEnable(GL_CULL_FACE);

	setSceneRadius(2);

	camera()->setPosition(qglviewer::Vec(0.5 * 2, 2.12 * 2, -2.12 * 2));
	camera()->setViewDirection(qglviewer::Vec(-0.5, -2.12, 2.12));

	emit viewerInitialized();
}

void QtGLView::draw()
{
	foreach(IGLRenderable* obj, renderList)
	{
		obj->render();
	}
}

void QtGLView::postDraw()
{
	glDisable(GL_TEXTURE_2D);

	/* Grid begin - Copied from QGLViewer source then modified */
	if (gridIsDrawn())
	{
		glColor3f(.4f, .4f, .4f);
		glPushMatrix();

		glRotatef(90.0f, 1.f, 0.f, 0.f);
		const int subdivisions = 3;
		const float halfSize = subdivisions/2.f;
		glBegin(GL_LINES);
		for (int i=0; i <= subdivisions; ++i)
		{
			const float pos = i - halfSize;
			glVertex2f(pos, -halfSize); // vertical
			glVertex2f(pos, +halfSize);

			// horizontal
			glVertex2f(-halfSize, pos); // |   |    | |  |_|_
			glVertex2f( halfSize, pos); // |   |___ |_|_ |_|_
		}
		glEnd();
		glColor3f(1.f, 1.f, 1.f);
		glPopMatrix();
	}

	/* Axis begin  - Copied from QGLViewer source then modified
	 * WZ models use negative Z axis as "front", hence X, Y and -Z
	 */
	if (axisIsDrawn())
	{
		const float length =camera()->sceneRadius();
		const float charWidth = length / 40.0;
		const float charHeight = length / 30.0;
		const float charShift = 1.04 * length;

		GLboolean lighting;
		glGetBooleanv(GL_LIGHTING, &lighting);

		glDisable(GL_LIGHTING);
		glEnable(GL_LINE_SMOOTH);
		glLineWidth(2);

		glBegin(GL_LINES);
		// The X
		glVertex3f(-charShift,  charWidth, -charHeight);
		glVertex3f(-charShift, -charWidth,  charHeight);
		glVertex3f(-charShift, -charWidth, -charHeight);
		glVertex3f(-charShift,  charWidth,  charHeight);
		// The Y
		glVertex3f( charWidth, charShift, charHeight);
		glVertex3f(0.f,        charShift, 0.f);
		glVertex3f(-charWidth, charShift, charHeight);
		glVertex3f(0.f,        charShift, 0.f);
		glVertex3f(0.f,        charShift, 0.f);
		glVertex3f(0.f,        charShift, -charHeight);
		// The Z (part of -Z)
		glVertex3f(-charWidth,  charHeight, -charShift);
		glVertex3f( charWidth,  charHeight, -charShift);
		glVertex3f( charWidth,  charHeight, -charShift);
		glVertex3f(-charWidth, -charHeight, -charShift);
		glVertex3f(-charWidth, -charHeight, -charShift);
		glVertex3f( charWidth, -charHeight, -charShift);
		// The - (part of -Z)
		glVertex3f(-charWidth*2, 0.f, -charShift);
		glVertex3f(-charWidth,   0.f, -charShift);
		glEnd();

		glEnable(GL_LIGHTING);

		float color[4];
		color[0] = 0.f;  color[1] = 0.f;  color[2] = 0.f;  color[3] = 1.0f;
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, color);
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, color);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.f);

		color[0] = 0.7f;  color[1] = 0.7f;  color[2] = 1.0f;  color[3] = 1.0f;
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
		glRotatef(180.0, 0.0, 1.0, 0.0);
		QGLViewer::drawArrow(length, 0.003*length);

		color[0] = 1.0f;  color[1] = 0.7f;  color[2] = 0.7f;  color[3] = 1.0f;
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
		glPushMatrix();
		glRotatef(90.0, 0.0, 1.0, 0.0);
		QGLViewer::drawArrow(length, 0.003*length);
		glPopMatrix();

		color[0] = 0.7f;  color[1] = 1.0f;  color[2] = 0.7f;  color[3] = 1.0f;
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
		glPushMatrix();
		glRotatef(-90.0, 1.0, 0.0, 0.0);
		QGLViewer::drawArrow(length, 0.003*length);
		glPopMatrix();

		if (!lighting)
		{
			glDisable(GL_LIGHTING);
		}
	}

	glEnable(GL_TEXTURE_2D);
}

void QtGLView::addToRenderList(IGLRenderable* object)
{
	// We have a TextureMan
	IGLTexturedRenderable* obj_tr = dynamic_cast<IGLTexturedRenderable*>(object);
	if (obj_tr)
		obj_tr->setTextureManager(this);

	// and a ShaderMan
	IGLShaderRenderable* obj_sr = dynamic_cast<IGLShaderRenderable*>(object);
	if (obj_sr)
		obj_sr->setShaderManager(this);

	renderList.append(object);
}

void QtGLView::removeFromRenderList(IGLRenderable* object)
{
	int index = renderList.indexOf(object, 0);
	if (index != -1)
	{
		renderList.removeAt(index);
	}
}

void QtGLView::clearRenderList()
{
	renderList.clear();
}

/// check textures for change

void QtGLView::textureChanged(const QString& fileName)
{
	t_texIt texIt = m_textures.find(fileName);
	if (texIt != m_textures.constEnd())
	{
		texIt.value().update = true;
		updateTimer.start(800, this);;
	}
}

void QtGLView::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == updateTimer.timerId())
	{
		updateTimer.stop();
		updateTextures();
	}
	else
	{
		QGLViewer::timerEvent(event);
	}
}

void QtGLView::updateTextures()
{
	t_texIt texIt;
	for (texIt = m_textures.begin(); texIt != m_textures.end(); ++texIt)
	{
		if (texIt.value().update)
		{
			QImage image(texIt.key());
			texIt.value().update = false;
			if (!image.isNull())
			{
				image = convertToGLFormat(image.mirrored(false, true));
				glBindTexture(GL_TEXTURE_2D, texIt.value().id());
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());
			}
		}
	}
	updateGL();
}

void QtGLView::_deleteTexture(t_texIt& texIt)
{
	textureUpdater.removePath(texIt.key());
	QGLWidget::deleteTexture(texIt.value().id());
	texIt = m_textures.erase(texIt);
}

/// GLTextureManager components

GLTexture QtGLView::createTexture(const QString& fileName)
{
	if (!fileName.isEmpty())
	{
		t_texIt texIt = m_textures.find(fileName);
		if (texIt == m_textures.end())
		{

			QImage image(fileName);
			ManagedGLTexture texture(QGLWidget::bindTexture(image, GL_TEXTURE_2D, GL_RGBA, QGLContext::LinearFilteringBindOption),
									 image.width(),
									 image.height());

			m_textures.insert(fileName, texture);

			textureUpdater.addPath(fileName);

			if (m_textures.size() > 2)
			{
				// Remove some old textures now
				texIt = m_textures.begin();
				while (texIt != m_textures.end())
				{
					if (texIt->users <= 0)
					{
						_deleteTexture(texIt);

					}
					else
					{
						++texIt;
					}
				}
			}

			return texture;
		}
		else
		{
			texIt.value().users++;
			return texIt.value();
		}
	}
	return GLTexture();
}

GLTexture QtGLView::bindTexture(const QString &fileName)
{
	return createTexture(fileName);
}

QString QtGLView::idToFilePath(GLuint id)
{
	t_texIt texIt;
	for (texIt = m_textures.begin(); texIt != m_textures.end(); ++texIt)
	{
		if (texIt->id() == id)
		{
			return texIt.key();
		}
	}
	return QString();
}

void QtGLView::deleteTexture(GLuint id)
{
	t_texIt texIt;
	for (texIt = m_textures.begin(); texIt != m_textures.end(); ++texIt)
	{
		if (texIt->id() == id)
		{
			texIt->users = std::max(texIt->users - 1, 0);
			if ( m_textures.size() > 2 && texIt->users == 0)
			{
				_deleteTexture(texIt);
			}
			break;
		}
	}
}

void QtGLView::deleteTexture(const QString& fileName)
{
	t_texIt texIt = m_textures.find(fileName);
	if (texIt != m_textures.end())
	{
		texIt->users = std::min(texIt->users - 1, 0);
		if (m_textures.size() > 2 && texIt->users == 0)
		{
			_deleteTexture(texIt);
		}
	}
}

void QtGLView::deleteAllTextures()
{
	t_texIt texIt;
	for (texIt = m_textures.begin(); texIt != m_textures.end(); ++texIt)
	{
		_deleteTexture(texIt);
	}
	m_textures.clear();
}

/// IGLShaderManager component

bool QtGLView::loadShader(int type, const QString& fileNameVert, const QString& fileNameFrag)
{
	if (QGLShaderProgram::hasOpenGLShaderPrograms(context()))
	{
		QGLShaderProgram* shader = getShader(type);
		bool ok_flag = true;

		if (shader != NULL)
		{
			shader->release();
			shader->removeAllShaders();
		}
		else
		{
			shader = new QGLShaderProgram(this);
		}

		if (!shader->addShaderFromSourceFile(QGLShader::Vertex, fileNameVert))
		{
			qWarning() << QString("QtGLView::loadShader - Error loading vertex shader:\n%1").arg(shader->log());
			ok_flag = false;
		}
		else if (!shader->addShaderFromSourceFile(QGLShader::Fragment, fileNameFrag))
		{
			qWarning() << QString("QtGLView::loadShader - Error loading fragment shader:\n%1").arg(shader->log());
			ok_flag = false;
		}
		else if (!shader->link())
		{
			qWarning() << QString("QtGLView::loadShader - Error linking shaders:\n%1").arg(shader->log());
			ok_flag = false;
		}

		if (ok_flag)
		{
			m_shaders[type] = shader;
		}
		else
		{
			delete shader;
		}

		return ok_flag;
	}

	return false;
}

void QtGLView::unloadShader(int type)
{
	if (QGLShaderProgram::hasOpenGLShaderPrograms(context()))
	{
		QGLShaderProgram* shader = getShader(type);

		if (shader != NULL)
		{
			shader->release();
			shader->removeAllShaders();

			delete shader;
			shader = NULL;
		}
	}
}
