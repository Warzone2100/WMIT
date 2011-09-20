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
class Pie3Model;

class QWZM : public QObject, protected WZM, public ATCMaskRenderable, public IAnimatable
{
	Q_OBJECT
public:
	explicit QWZM(QObject *parent = 0);
	virtual ~QWZM();

	void operator=(const WZM& wzm);

	void render();
	void animate();
	//void setCenterPointState(bool draw = true);

	void setRenderTexture(QString fileName);
	void setTextureManager(IGLTextureManager * manager);
	void clearRenderTexture();

	void setTCMaskTexture(QString fileName);
	void clearTCMaskTexture();
	bool hasTCMaskTexture() const;

	void clear();

	QStringList getMeshNames();

public: // WZM interface - mesh control border
	virtual operator Pie3Model() const;
	inline bool read(std::istream& in) {return WZM::read(in);}
	inline void write(std::ostream& out) const {WZM::write(out);}

	bool importFromOBJ(std::istream& in);
	inline void exportToOBJ(std::ostream& out) const {WZM::exportToOBJ(out);}

	bool importFrom3DS(std::string fileName);
	inline bool exportTo3DS(std::string fileName) const {return WZM::exportTo3DS(fileName);}

	inline void setTextureName(wzm_texture_type_t type, std::string name) {WZM::setTextureName(type, name);}
	inline std::string getTextureName(wzm_texture_type_t type) const {return WZM::getTextureName(type);}

	inline bool couldHaveTCArrays() const {return WZM::couldHaveTCArrays();}
	inline void reverseWinding(int mesh = -1) {WZM::reverseWinding(mesh);}

	void addMesh (const Mesh& mesh);
	void rmMesh (int index);

signals:
	void meshCountChanged(int, QStringList);

public slots:
	void setScaleXYZ(GLfloat xyz);
	void setScaleX(GLfloat x);
	void setScaleY(GLfloat y);
	void setScaleZ(GLfloat z);
	void slotMirrorAxis(int axis);

	void setActiveMesh(int mesh = -1);
	void applyTransformations();

private:
	Q_DISABLE_COPY(QWZM)
	void defaultConstructor();
	void drawCenterPoint();

	GLuint m_texture, m_tcm;

	GLfloat scale_all, scale_xyz[3];
	static const GLint winding;

	int m_active_mesh;
};

#endif // QWZM_HPP
