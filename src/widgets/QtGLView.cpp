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

#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTextStream>
#include <QPixmap>
#include <QImage>
#include <QApplication>

#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLContext>
#include <QtDebug>

#include <QGLViewer/vec.h>

#include "IGLTexturedRenderable.h"
#include "IGLShaderRenderable.h"
#include "IAnimatable.h"
#include "WZLight.h"

using namespace qglviewer;

namespace {

/*!
 * Makes the viewer's OpenGL context current for the lifetime of the object,
 * and restores the previous state on scope exit.
 *
 * Every entry point of the texture and shader managers can be - and in practice
 * is - called straight from a UI slot (MainWindow::fireTextureDialog() calls
 * clearGLRenderTextures() / loadGLRenderTexture() right after the texture
 * dialog is accepted). QOpenGLWidget only makes its context current around
 * initializeGL()/paintGL()/resizeGL(), so those calls were creating, binding
 * and destroying QOpenGLTexture objects with *no* current context.
 *
 * Under Qt5 this was largely papered over by QGLWidget, whose bindTexture()
 * made the context current for us - QGLViewer derives from QOpenGLWidget in
 * Qt6 and no longer does. GL calls with no current context are undefined, and
 * the actual behaviour is driver-dependent.
 */
class ScopedGLContext
{
public:
	explicit ScopedGLContext(QOpenGLWidget *widget) : m_widget(nullptr)
	{
		if (widget == nullptr)
			return;

		QOpenGLContext *ctx = widget->context();

		// Nothing to do before initializeGL() has ever run, and nothing to do
		// if we are already inside a paintGL()/initializeGL() callback.
		if (ctx == nullptr || !ctx->isValid() || QOpenGLContext::currentContext() == ctx)
			return;

		widget->makeCurrent();
		m_widget = widget;
	}

	~ScopedGLContext()
	{
		if (m_widget != nullptr)
			m_widget->doneCurrent();
	}

	ScopedGLContext(const ScopedGLContext&) = delete;
	ScopedGLContext& operator=(const ScopedGLContext&) = delete;

private:
	QOpenGLWidget *m_widget;
};

} // anonymous namespace

const Vec lightPos(2.25, 6., 4.5);

QtGLView::QtGLView(QWidget *parent) :
		QGLViewer(parent),
		drawLightSource(true),
		linkLightToCamera(true)
{
	setStateFileName(QString());
	connect(&textureUpdater, SIGNAL(fileChanged(QString)), this, SLOT(textureChanged(QString)));

	setShortcut(DISPLAY_FPS, 0); // Disable stuff that won't work.
	setGridIsDrawn(true);
	setAxisIsDrawn(true);
}

QtGLView::~QtGLView()
{
	// The GL resources below can only be released while our context is current.
	// (The previously commented-out destroy() loop "crashing on F29" was the
	// same missing-current-context problem.)
	ScopedGLContext ctxGuard(this);

	foreach(IGLRenderable* obj, renderList)
	{
		dynamicManagedSetup(obj, true);
	}

	for (t_texIt texIt = m_textures.begin(); texIt != m_textures.end(); ++texIt)
	{
		if (texIt->pTexture != nullptr)
			texIt->pTexture->destroy();
	}
	m_textures.clear();
}

void QtGLView::animate()
{
	foreach(IAnimatable* obj, animateList)
	{
		obj->animate();
	}
}

void QtGLView::setLightColors()
{
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lightCol0[LIGHT_EMISSIVE].data());
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightCol0[LIGHT_AMBIENT].data());
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightCol0[LIGHT_DIFFUSE].data());
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightCol0[LIGHT_SPECULAR].data());
}

void QtGLView::init()
{
	// initialize GLEW
	//
	// glewExperimental is required whenever the context is not a legacy
	// compatibility context: without it GLEW decides an entry point is
	// unavailable based on the extension string alone and leaves the function
	// pointer NULL, and the first call through it jumps to address 0.
	glewExperimental = GL_TRUE;

	const GLenum glewStatus = glewInit();
	if (glewStatus != GLEW_OK)
	{
		// GLEW_ERROR_NO_GLX_DISPLAY is expected (and harmless) under Wayland.
		qWarning("glewInit() failed: %s", reinterpret_cast<const char*>(glewGetErrorString(glewStatus)));
	}

	// Log what we actually got
	const GLubyte *glVendor = glGetString(GL_VENDOR);
	const GLubyte *glRenderer = glGetString(GL_RENDERER);
	const GLubyte *glVersion = glGetString(GL_VERSION);
	qInfo("OpenGL vendor: %s", glVendor ? reinterpret_cast<const char*>(glVendor) : "(null)");
	qInfo("OpenGL renderer: %s", glRenderer ? reinterpret_cast<const char*>(glRenderer) : "(null)");
	qInfo("OpenGL version: %s", glVersion ? reinterpret_cast<const char*>(glVersion) : "(null)");

	setLightColors();
	glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, 1.0);
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
	setLightColors();

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
	update();
}

void QtGLView::removeFromRenderList(IGLRenderable* object)
{
	int index = renderList.indexOf(object, 0);
	if (index != -1)
	{
		renderList.removeAt(index);
		update();
	}
}

void QtGLView::clearRenderList()
{
	renderList.clear();
	update();
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
	ScopedGLContext ctxGuard(this);

	t_texIt texIt;
	for (texIt = m_textures.begin(); texIt != m_textures.end(); ++texIt)
	{
		if (texIt.value().update)
		{
			QImage image(texIt.key());
			texIt.value().update = false;
			if (!image.isNull())
			{
				// NOT mirrored: this has to match the initial upload in
				// GLTexture(), which hands the QImage to QOpenGLTexture
				// unflipped. WZ2100 texture coordinates put v = 0 at the TOP
				// of the image (the .pie origin is top-left) and the game
				// uploads its textures the same way, so the unflipped form is
				// the correct one. Mirroring here made a texture flip
				// vertically the moment it was re-read from disk - i.e. every
				// time an artist saved from their paint package with the model
				// open.
				texIt.value().pTexture->setData(image);
			}
		}
	}
	update();
}

void QtGLView::_deleteTexture(t_texIt& texIt)
{
	textureUpdater.removePath(texIt.key());
	texIt.value().pTexture->destroy();
	texIt = m_textures.erase(texIt);
}

/// GLTextureManager components

QtGLView::ManagedGLTexture::ManagedGLTexture(QOpenGLTexture *pInputTexture):
	GLTexture(pInputTexture->textureId(), pInputTexture->width(), pInputTexture->height()),
	pTexture(pInputTexture),
	users(1),
	update(false)
{}

GLTexture QtGLView::createTexture(const QString& fileName)
{
	ScopedGLContext ctxGuard(this);

	if (!fileName.isEmpty())
	{
		t_texIt texIt = m_textures.find(fileName);
		if (texIt == m_textures.end())
		{
			QImage image(fileName);
			QOpenGLTexture *pTexture = new QOpenGLTexture(image);
			pTexture->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
			ManagedGLTexture texture(pTexture);
			texture.pTexture->bind();

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
	ScopedGLContext ctxGuard(this);

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
	ScopedGLContext ctxGuard(this);

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
	ScopedGLContext ctxGuard(this);

	// NOTE: _deleteTexture() erases and already advances texIt, so the loop
	// must not increment it again - doing so skipped every second entry and,
	// once the erased element was the last one, incremented an end() iterator.
	t_texIt texIt = m_textures.begin();
	while (texIt != m_textures.end())
	{
		_deleteTexture(texIt);
	}
	m_textures.clear();
}

/// IGLShaderManager component

/*!
 * Expand #include "..." directives in a shader source file.
 *
 * QOpenGLShaderProgram::addShaderFromSourceFile has no include mechanism, but
 * WZ's shader loader does (lib/ivis_opengl/gfx_api_gl.cpp), and the preview
 * shaders are most useful to the extent that they can share the engine's
 * conventions. Paths are resolved relative to the including file, matching WZ.
 *
 * Returns false, with the offending path in errString, if a file cannot be read
 * or the include depth is exceeded.
 */
static bool readShaderSource(const QString& fileName, QString& out, QString* errString, int depth = 0)
{
	if (depth > 5)
	{
		if (errString)
			*errString = QString("Nested #include depth > 5 at: %1").arg(fileName);
		return false;
	}

	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		if (errString)
			*errString = QString("Could not read shader: %1").arg(fileName);
		return false;
	}

	const QString basedir = QFileInfo(fileName).path();
	QTextStream in(&file);
	// Not multi-line and not anchored to the start of the file, so it matches
	// each directive on its own line.
	static const QRegularExpression re(QStringLiteral("^\\s*#include\\s+\"([^\"]+)\"\\s*$"));

	out.clear();
	QString line;
	while (!in.atEnd())
	{
		line = in.readLine();

		const QRegularExpressionMatch m = re.match(line);
		if (!m.hasMatch())
		{
			out += line;
			out += QLatin1Char('\n');
			continue;
		}

		QString included;
		if (!readShaderSource(basedir + QLatin1Char('/') + m.captured(1), included, errString, depth + 1))
			return false;

		out += included;
		out += QLatin1Char('\n');
	}

	return true;
}

bool QtGLView::loadShader(int type, const QString& fileNameVert, const QString& fileNameFrag,
                          QString* errString)
{
	if (QOpenGLShaderProgram::hasOpenGLShaderPrograms(context()))
	{
		QOpenGLShaderProgram* shader = getShader(type);
		bool ok_flag = true;

		if (shader != nullptr)
		{
			shader->release();
			shader->removeAllShaders();
		}
		else
		{
			shader = new QOpenGLShaderProgram(this);
		}

		QString vertSrc, fragSrc, readErr;

		if (!readShaderSource(fileNameVert, vertSrc, &readErr))
		{
			if (errString)
				*errString = QString("QtGLView::loadShader - %1").arg(readErr);
			ok_flag = false;
		}
		else if (!readShaderSource(fileNameFrag, fragSrc, &readErr))
		{
			if (errString)
				*errString = QString("QtGLView::loadShader - %1").arg(readErr);
			ok_flag = false;
		}
		else if (!shader->addShaderFromSourceCode(QOpenGLShader::Vertex, vertSrc))
		{
			if (errString)
				*errString = QString("QtGLView::loadShader - Error loading vertex shader:\n%1").arg(shader->log());
			ok_flag = false;
		}
		else if (!shader->addShaderFromSourceCode(QOpenGLShader::Fragment, fragSrc))
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
	if (QOpenGLShaderProgram::hasOpenGLShaderPrograms(context()))
	{
		QOpenGLShaderProgram* shader = getShader(type);

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
			stopAnimation();
	}
	else
	{
		if (enabled)
			startAnimation();
	}

	// Always ask for a frame.
	// When the animation loop is not running this is the only thing that will produce one, and callers reach here after the
	// scene has changed (a model was loaded or closed).
	update();
}
