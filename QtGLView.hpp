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

#ifndef QTGLVIEW_HPP
#define QTGLVIEW_HPP

#include <QString>
#include <QList>
#include <QHash>
#include <QBasicTimer>
#include <QFileSystemWatcher>

#include <QGLViewer/qglviewer.h>

#include "GLTexture.hpp"
#include "IGLTextureManager.hpp"

#include "ITCMaskProvider.hpp"

class IGLRenderable;
class ITexturedRenderable;
class ITCMaskRenderable;
class QGLShaderProgram;

class QtGLView : public QGLViewer , public IGLTextureManager, public ITCMaskProvider
{
	Q_OBJECT
public:
	explicit QtGLView(QWidget *parent = 0);
	~QtGLView();

	void draw();
	void postDraw();

	void addToRenderList(IGLRenderable* object);
	void addToRenderList(ITexturedRenderable* object);
	void addToRenderList(ITCMaskRenderable* object);
	void removeFromRenderList(IGLRenderable* object);
	void clearRenderList();

	/// GLTextureManager components
	GLTexture createTexture(const QString& fileName);
	GLTexture bindTexture(const QString& fileName); // We're hiding a few QGLWidget functions on purpose
	virtual QString idToFilePath(GLuint id);
	void deleteTexture(GLuint id);
	void deleteTexture(const QString& fileName);
	void deleteAllTextures();

	/// TCMaskProvider components
	unsigned tcmaskSupport() const;
	TCMaskMethod currentTCMaskMode() const;
	void setTCMaskMode(TCMaskMethod method);
	void setTCMaskEnvironment(const GLfloat tcmaskColor[4]);
	void setTCMaskEnvironment(const QColor& tcmaskColour);
	void resetTCMaskEnvironment();

signals:
	void viewerInitialized();

protected:

	void init();

	void timerEvent(QTimerEvent* event);

	struct ManagedGLTexture : public GLTexture
	{
		int users;
		bool update;
		ManagedGLTexture(GLuint id, GLsizei w, GLsizei h):
				GLTexture(id, w, h), users(1), update(false){}

		virtual ~ManagedGLTexture(){}
	};

private:

	QList<IGLRenderable*> renderList;

	/// GLTextureManager components
	QHash<QString, ManagedGLTexture> m_textures;
	typedef QHash<QString, ManagedGLTexture>::iterator t_texIt;
	typedef QHash<QString, ManagedGLTexture>::const_iterator t_cTexIt;

	void updateTextures();
	void _deleteTexture(t_texIt& texIt);

	QFileSystemWatcher textureUpdater;
	QBasicTimer updateTimer;

	/// TCMaskProvider components
	QGLShaderProgram* m_tcmaskShader;
	int m_colorLoc, m_baseTexLoc, m_tcTexLoc;
	TCMaskMethod m_currentMode;
	unsigned m_tcmSupport;

private slots:
	void textureChanged(const QString& fileName);
};

#endif // QTGLVIEW_HPP
