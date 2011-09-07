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

#include "TexturedRenderable.hpp"
#include "TCMaskRenderable.hpp"
#include "IGLRenderable.hpp"

QtGLView::QtGLView(QWidget *parent) :
		QGLViewer(parent),
		m_tcmaskShader(NULL),
		m_currentMode(None),
		m_tcmSupport(0)
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

	if (m_tcmaskShader != NULL)
	{
		delete m_tcmaskShader;
	}
}

void QtGLView::init()
{
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

	// Determine which opengl version is present.
	QGLFormat::OpenGLVersionFlags oglFFlags = context()->format().openGLVersionFlags();

	/*
	 * Fixed function OGL requirements
	 * GL_ARB_texture_env_crossbar (core in 1.4)
	 *	-> GL_ARB_texture_env_combine ( core in 1.3)
	 * 		-> GL_ARB_multitexture ( core in 1.3)
	 * GL_ARB_multitexture ( core in 1.3)
	 * Just going to check for 1.4
	 * Also, shader requirements are > 1.4,
	 */
	if (oglFFlags & QGLFormat::OpenGL_Version_1_4)
	{
		// Can use fixed function tcmask
		m_tcmSupport = FixedPipeline;

		/* What about shader tcmask?
		 * Using Qt's shader functions, so using
		 * QGLShaderProgram::hasOpenGLShaderPrograms
		 * and whether linking fails
		 */

		if (QGLShaderProgram::hasOpenGLShaderPrograms(context()))
		{
			if (m_tcmaskShader != NULL)
			{
				m_tcmaskShader->removeAllShaders();
			}
			else
			{
				m_tcmaskShader = new QGLShaderProgram(this);
			}

			if (!m_tcmaskShader->addShaderFromSourceFile(QGLShader::Vertex,"./tcmask.vert"))
			{
				qWarning() << QString("QtGLView::init - Error loading vertex shader:\n%1").arg(m_tcmaskShader->log());
				delete m_tcmaskShader;
				m_tcmaskShader = NULL;
			}
			else if (!m_tcmaskShader->addShaderFromSourceFile(QGLShader::Fragment,"./tcmask.frag"))
			{
				qWarning() << QString("QtGLView::init - Error loading fragment shader:\n%1").arg(m_tcmaskShader->log());
				delete m_tcmaskShader;
				m_tcmaskShader = NULL;
			}
			else if (!m_tcmaskShader->link())
			{
				qWarning() << QString("QtGLView::init - Error linking shaders:\n%1").arg(m_tcmaskShader->log());
				delete m_tcmaskShader;
				m_tcmaskShader = NULL;
			}
			else
			{
				m_tcmaskShader->bind();
				m_baseTexLoc = m_tcmaskShader->uniformLocation("Texture0");
				m_tcTexLoc = m_tcmaskShader->uniformLocation("Texture1");
				m_colorLoc = m_tcmaskShader->uniformLocation("teamcolour");
				m_tcmaskShader->setUniformValue(m_tcTexLoc, GLint(1));
				m_tcmaskShader->setUniformValue(m_baseTexLoc, GLint(0));
				m_tcmaskShader->release();
				m_tcmSupport |= Shaders;
			}
		}
	}

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
		glColor3f(.7f, 1.f, .7f);
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

	/* Grid end
	 * Axis begin  - Copied from QGLViewer source then modified */
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
		// The Z
		glVertex3f(-charWidth,  charHeight, charShift);
		glVertex3f( charWidth,  charHeight, charShift);
		glVertex3f( charWidth,  charHeight, charShift);
		glVertex3f(-charWidth, -charHeight, charShift);
		glVertex3f(-charWidth, -charHeight, charShift);
		glVertex3f( charWidth, -charHeight, charShift);
		glEnd();

		glEnable(GL_LIGHTING);

		float color[4];
		color[0] = 0.7f;  color[1] = 0.7f;  color[2] = 1.0f;  color[3] = 1.0f;
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
		QGLViewer::drawArrow(length, 0.01*length);

		color[0] = 1.0f;  color[1] = 0.7f;  color[2] = 0.7f;  color[3] = 1.0f;
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
		glPushMatrix();
		glRotatef(-90.0, 0.0, 1.0, 0.0);
		QGLViewer::drawArrow(length, 0.01*length);
		glPopMatrix();

		color[0] = 0.7f;  color[1] = 1.0f;  color[2] = 0.7f;  color[3] = 1.0f;
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
		glPushMatrix();
		glRotatef(-90.0, 1.0, 0.0, 0.0);
		QGLViewer::drawArrow(length, 0.01*length);
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
	renderList.append(object);
}

void QtGLView::addToRenderList(ITexturedRenderable* object)
{
	object->setTextureManager(this);
	renderList.append(object);
}

void QtGLView::addToRenderList(ITCMaskRenderable* object)
{
	object->setTextureManager(this);
	object->setTCMaskProvider(this);
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

GLTexture QtGLView::createTexture(const QString& fileName)
{
	if (!fileName.isEmpty())
	{
		t_texIt texIt = m_textures.find(fileName);
		if (texIt == m_textures.end())
		{
			QPixmap pixmap(fileName);
			ManagedGLTexture texture(QGLWidget::bindTexture(pixmap, GL_TEXTURE_2D, GL_RGBA, QGLContext::LinearFilteringBindOption),
									 pixmap.width(),
									 pixmap.height());

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
	return GLTexture(0, 0, 0);
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

unsigned QtGLView::tcmaskSupport() const
{
	return m_tcmSupport;
}

TCMaskMethod QtGLView::currentTCMaskMode() const
{
	return m_currentMode;
}

void QtGLView::setTCMaskMode(TCMaskMethod method)
{
	m_currentMode = static_cast<TCMaskMethod>(m_tcmSupport & method);
	updateGL();
}

void QtGLView::setTCMaskEnvironment(const GLfloat tcmaskColour[4])
{
	if (m_currentMode == Shaders)
	{
		m_tcmaskShader->bind();

		m_tcmaskShader->setUniformValue(m_colorLoc, tcmaskColour[0],
								tcmaskColour[1],
								tcmaskColour[2],
								tcmaskColour[3]);
	}
	else if (m_currentMode == FixedPipeline)
	{
		glActiveTexture(GL_TEXTURE0);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,	GL_COMBINE);
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, tcmaskColour);

		// TU0 RGB
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB,		GL_ADD_SIGNED);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB,		GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB,		GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB,		GL_CONSTANT);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB,		GL_SRC_COLOR);

		// TU0 Alpha
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA,		GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA,		GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA,	GL_SRC_ALPHA);

		// TU1
		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,	GL_COMBINE);

		// TU1 RGB
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB,		GL_INTERPOLATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB,		GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB,		GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB,		GL_TEXTURE0);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB,		GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB,		GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB,		GL_SRC_ALPHA);

		// TU1 Alpha
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA,		GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA,		GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA,	GL_SRC_ALPHA);
	}
}

void QtGLView::resetTCMaskEnvironment()
{
	if (m_currentMode == Shaders)
	{
		m_tcmaskShader->release();
	}
}

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
