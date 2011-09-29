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
#include <QMap>

#include "GLee.h"

#include "WZM.hpp"
#include "IAnimatable.hpp"
#include "IGLTexturedRenderable.hpp"
#include "IGLShaderRenderable.h"

enum wz_shader_type_t {WZ_SHADER_NONE = 0, WZ_SHADER_PIE3, WZ_SHADER_PIE3_USER};

class Pie3Model;

class QWZM: public QObject, protected WZM, public IAnimatable,
		public IGLTexturedRenderable, public IGLShaderRenderable
{
	Q_OBJECT
public:
	explicit QWZM(QObject *parent = 0);
	virtual ~QWZM();

	void operator=(const WZM& wzm);

	void clear();
	QStringList getMeshNames() const;
	void getTexturesMap(QMap<wzm_texture_type_t, QString>& map) const;

	// GLTexture controls
	void loadGLRenderTexture(wzm_texture_type_t type, QString fileName);
	void unloadGLRenderTexture(wzm_texture_type_t type);
	bool hasGLRenderTexture(wzm_texture_type_t type) const;
	void clearGLRenderTextures();

	// TCMask part
	void setTCMaskEnvironment(const QColor& tcmaskColour);
	void resetTCMaskEnvironment();
	//void setCenterPointState(bool draw = true);
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

public:
	/// IAnimatable
	void animate();

	/// IGLTexturedRenderable
	void render();
	void setTextureManager(IGLTextureManager * manager);

	/// IGLShaderRenderable
	bool initShader(int type);
	bool bindShader(int type);
	void releaseShader(int type);

	/// WZM interface - mesh control border
	virtual operator Pie3Model() const;
	inline bool read(std::istream& in) {return WZM::read(in);}
	void write(std::ostream& out) const;

	bool importFromOBJ(std::istream& in);
	void exportToOBJ(std::ostream& out) const;

	bool importFrom3DS(std::string fileName);
	bool exportTo3DS(std::string fileName) const;

	inline void setTextureName(wzm_texture_type_t type, std::string name) {WZM::setTextureName(type, name);}
	inline std::string getTextureName(wzm_texture_type_t type) const {return WZM::getTextureName(type);}
	inline void clearTextureNames() {WZM::clearTextureNames();}

	inline bool couldHaveTCArrays() const {return WZM::couldHaveTCArrays();}
	inline void reverseWinding(int mesh = -1) {WZM::reverseWinding(mesh);}

	void addMesh (const Mesh& mesh);
	void rmMesh (int index);

private:
	Q_DISABLE_COPY(QWZM)
	void defaultConstructor();
	void drawCenterPoint();

	bool setupTextureUnits(int type);
	void clearTextureUnits(int type);

	void applyPendingChangesToModel(WZM& model) const;
	void resetAllPendingChanges();

	std::map<wzm_texture_type_t, GLuint> m_gl_textures;

	GLfloat scale_all, scale_xyz[3];
	static const GLint winding;

	int m_active_mesh;
	bool m_pending_changes;

	QColor m_tcmaskColour;
};

#endif // QWZM_HPP
