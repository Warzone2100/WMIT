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

#include "QtGLView.h"

#ifdef Q_OS_MAC
# include <CoreFoundation/CoreFoundation.h>
# include <CoreFoundation/CFURL.h>
#endif

#include <QPixmap>
#include <QImage>
#include <QApplication>

#include <QGLShaderProgram>
#include <QtDebug>

#include <QGLViewer/vec.h>

#include "IGLTexturedRenderable.h"
#include "IGLShaderRenderable.h"
#include "IAnimatable.h"

using namespace qglviewer;

enum LIGHTING_TYPE {
	LIGHT_EMISSIVE, LIGHT_AMBIENT, LIGHT_DIFFUSE, LIGHT_SPECULAR, LIGHT_TYPE_MAX
};

const Vec lightPos(2.25, 6., 4.5);
static GLfloat lightCol0[LIGHT_TYPE_MAX][4] = {
	{0.0f, 0.0f, 0.0f, 1.0f},  {0.5f, 0.5f, 0.5f, 1.0f}, {0.8f, 0.8f, 0.8f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}
};


QtGLView::QtGLView(QWidget *parent) :
		QGLViewer(parent),
		drawLightSource(true),
		linkLightToCamera(true)
{
	setStateFileName(QString::null);
	connect(&textureUpdater, SIGNAL(fileChanged(QString)), this, SLOT(textureChanged(QString)));

	setShortcut(DISPLAY_FPS, 0); // Disable stuff that won't work.
	setGridIsDrawn(true);
	setAxisIsDrawn(true);
}

QtGLView::~QtGLView()
{
	foreach(IGLRenderable* obj, renderList)
	{
		dynamicManagedSetup(obj, true);
	}

/* This is crashing on F29 and unclear if we need this on destroy
	foreach (ManagedGLTexture texture, m_textures)
	{
		const GLuint id = texture.id();
		QGLWidget::deleteTexture(id);
	}
*/
}

void QtGLView::animate()
{
	foreach(IAnimatable* obj, animateList)
	{
		obj->animate();
	}
}

void QtGLView::init()
{
	// initialize GLEW
	glewInit();

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lightCol0[LIGHT_EMISSIVE]);
	glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, 1.0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightCol0[LIGHT_AMBIENT]);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightCol0[LIGHT_DIFFUSE]);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightCol0[LIGHT_SPECULAR]);
	glEnable(GL_LIGHT0);

	glEnable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL); // required for glMaterial to work
	glEnable(GL_MULTISAMPLE);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GEQUAL, 0.05f);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	setSceneRadius(3);

	camera()->setPosition(qglviewer::Vec(0.5 * 2, 2.12 * 2, -2.12 * 2));
	camera()->setViewDirection(qglviewer::Vec(-0.5, -2.12, 2.12));

	setManipulatedFrame(&light);

	light.setPosition(lightPos);

	setAnimationPeriod(1000 / 60);

	emit viewerInitialized();
}

#define FP12_MULTIPLIER (1 << 12)
void QtGLView::draw()
{
	static float mtxPrj[16], mtxMV[16], larr[4] = {0.f};

	camera()->getProjectionMatrix(mtxPrj);
	camera()->getModelViewMatrix(mtxMV);

	if (linkLightToCamera)
		light.setPosition(camera()->position());

	qglviewer::Vec lvec = light.position();
	lvec.normalize();
	lvec *= FP12_MULTIPLIER;
	larr[0] = lvec.x;
	larr[1] = lvec.y;
	larr[2] = lvec.z;

	glLightfv(GL_LIGHT0, GL_POSITION, larr);

	foreach(IGLRenderable* obj, renderList)
	{
		obj->render(mtxMV, mtxPrj, larr);
	}
}

void QtGLView::postDraw()
{
	// replacing default implementation

	GLboolean lighting, texture;

	glGetBooleanv(GL_LIGHTING, &lighting);
	glDisable(GL_LIGHTING);

	texture = glIsEnabled(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_2D);

	glColor3f(lightCol0[LIGHT_DIFFUSE][0], lightCol0[LIGHT_DIFFUSE][1], lightCol0[LIGHT_DIFFUSE][2]);

	if (drawLightSource && !linkLightToCamera)
	{
		drawLight(GL_LIGHT0, 2.);
	}

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

		glEnable(GL_LINE_SMOOTH);
		glLineWidth(2);
		glColor3f(1.f, 1.f, 1.f);

		glBegin(GL_LINES);
		// The X
		glVertex3f(charShift,  charWidth, -charHeight);
		glVertex3f(charShift, -charWidth,  charHeight);
		glVertex3f(charShift, -charWidth, -charHeight);
		glVertex3f(charShift,  charWidth,  charHeight);
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
		glPushMatrix();
		glRotatef(180.0, 0.0, 1.0, 0.0);
		QGLViewer::drawArrow(length, 0.003*length);
		glPopMatrix();

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
	}

	if (lighting)
	{
		glEnable(GL_LIGHTING);
	}
	else
	{
		glDisable(GL_LIGHTING);
	}

	if (texture)
		glEnable(GL_TEXTURE_2D);
}

void QtGLView::dynamicManagedSetup(IGLRenderable *object, bool remove)
{
	// We have a TextureMan
	IGLTexturedRenderable* obj_tr = dynamic_cast<IGLTexturedRenderable*>(object);
	if (obj_tr)
		obj_tr->setTextureManager(remove ? nullptr : this);

	// and a ShaderMan
	IGLShaderRenderable* obj_sr = dynamic_cast<IGLShaderRenderable*>(object);
	if (obj_sr)
		obj_sr->setShaderManager(remove ? nullptr : this);
}

void QtGLView::addToRenderList(IGLRenderable* object)
{
	dynamicManagedSetup(object);
	renderList.append(object);
}

void QtGLView::removeFromRenderList(IGLRenderable* object)
{
	int index = renderList.indexOf(object, 0);
	if (index != -1)
		renderList.removeAt(index);
}

void QtGLView::clearRenderList()
{
	renderList.clear();
}

void QtGLView::addToAnimateList(IAnimatable *object)
{
	animateList.append(object);
}

void QtGLView::removeFromAnimateList(IAnimatable *object)
{
	int index = animateList.indexOf(object, 0);
	if (index != -1)
		animateList.removeAt(index);
}

/// check textures for change

void QtGLView::textureChanged(const QString& fileName)
{
	t_texIt texIt = m_textures.find(fileName);
	if (texIt != m_textures.constEnd())
	{
		texIt.value().update = true;
        updateTimer.start(800, this);
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
						texIt = m_textures.begin();
					}
					else
					{
						++texIt;
					}
				}
			}

			return std::move(texture);
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
		texIt->users = std::max(texIt->users - 1, 0);
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

bool QtGLView::loadShader(int type, const QString& fileNameVert, const QString& fileNameFrag,
                          QString* errString)
{
	if (QGLShaderProgram::hasOpenGLShaderPrograms(context()))
	{
		QGLShaderProgram* shader = getShader(type);
		bool ok_flag = true;

		if (shader != nullptr)
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
			if (errString)
				*errString = QString("QtGLView::loadShader - Error loading vertex shader:\n%1").arg(shader->log());
			ok_flag = false;
		}
		else if (!shader->addShaderFromSourceFile(QGLShader::Fragment, fileNameFrag))
		{
			if (errString)
				*errString = QString("QtGLView::loadShader - Error loading fragment shader:\n%1").arg(shader->log());
			ok_flag = false;
		}
		else if (!shader->link())
		{
			if (errString)
				*errString = QString("QtGLView::loadShader - Error linking shaders:\n%1").arg(shader->log());
			ok_flag = false;
		}

		if (!ok_flag)
			shader = nullptr;

		auto& sinfo = m_shaders[type];
		sinfo.program = shader;
		sinfo.is_external = ok_flag && !fileNameFrag.startsWith(":");

		return ok_flag;
	}

	return false;
}

void QtGLView::unloadShader(int type)
{
	if (QGLShaderProgram::hasOpenGLShaderPrograms(context()))
	{
		QGLShaderProgram* shader = getShader(type);

		if (shader != nullptr)
		{
			shader->release();
			shader->removeAllShaders();

			delete shader;
			shader = nullptr;
		}
	}
}
void QtGLView::setDrawLightSource(bool draw)
{
	drawLightSource = draw;
	repaint();
}

void QtGLView::setLinkLightToCamera(bool link)
{
	linkLightToCamera = link;
	repaint();
}

void QtGLView::setAnimateState(bool enabled)
{
	if (animationIsStarted())
	{
		if (!enabled)
		{
			stopAnimation();
			repaint();
		}
	}
	else
	{
		if (enabled)
			startAnimation();
	}
}
