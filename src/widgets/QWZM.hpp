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

#ifndef QWZM_HPP
#define QWZM_HPP

#include <QString>
#include <QStringList>
#include <QObject>

#include "GLee.h"

#include "WZM.hpp"

#include "IGLRenderable.hpp"
#include "TexturedRenderable.hpp"
#include "TCMaskRenderable.hpp"
#include "IAnimatable.hpp"

class IGLTextureManager;

class QWZM : public QObject, public WZM, public ATCMaskRenderable, public IAnimatable
{
	Q_OBJECT
public:
	explicit QWZM(QObject *parent = 0);
	virtual ~QWZM();

	void operator=(const WZM& wzm);

	void render();
	void animate();

	void setRenderTexture(QString fileName);
	void setTextureManager(IGLTextureManager * manager);
	void clearRenderTexture();

	void setTCMaskTexture(QString fileName);
	void clearTCMaskTexture();
	bool hasTCMaskTexture() const;

	void setScaleXYZ(GLfloat xyz);
	void setScaleX(GLfloat x);
	void setScaleY(GLfloat y);
	void setScaleZ(GLfloat z);

	void applyTransformations(int mesh = -1);

	void clear();

	QStringList getMeshNames();

	// mesh control wrappers
	void addMesh (const Mesh& mesh);
	void rmMesh (int index);
	bool importFromOBJ(std::istream& in);
	bool importFrom3DS(std::string fileName);

signals:
	void meshCountChanged(int, QStringList);

private:
	Q_DISABLE_COPY(QWZM)
	void defaultConstructor();

	GLuint m_texture, m_tcm;

	GLfloat scale_all, scale_xyz[3];
	static const GLint winding;
};

#endif // QWZM_HPP
