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

#include <time.h>

#include <QtCore>
#include <QString>
#include <QStringList>
#include <QObject>
#include <QMap>

#include <GLee.h>

#include "WZM.h"
#include "IAnimatable.h"
#include "IGLTexturedRenderable.h"
#include "IGLShaderRenderable.h"

enum wz_shader_type_t {WZ_SHADER_NONE = 0,
		       WZ_SHADER_WZ32, // WIP. Do not move to last/default yet
		       WZ_SHADER_WZ31,
		       WZ_SHADER__LAST, WZ_SHADER__FIRST = WZ_SHADER_NONE};

class Pie3Model;

class QWZM: public QObject, public WZM, public IAnimatable,
		public IGLTexturedRenderable, public IGLShaderRenderable
{
	Q_OBJECT
public:
	explicit QWZM(QObject *parent = nullptr);
	virtual ~QWZM();

	void operator=(const WZM& wzm);

	void clear();
	QStringList getMeshNames() const;
	void getTexturesMap(QMap<wzm_texture_type_t, QString>& map) const;

	void disableShaders();
	static QString shaderTypeToString(wz_shader_type_t type);
	bool isFixedPipelineRenderer() const;

	// GLTexture controls
	void loadGLRenderTexture(wzm_texture_type_t type, QString fileName);
	void unloadGLRenderTexture(wzm_texture_type_t type);
	bool hasGLRenderTexture(wzm_texture_type_t type) const;
	void clearGLRenderTextures();

	// TCMask part
	void setTCMaskColor(const QColor& tcmaskColour);
	QColor getTCMaskColor();
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

	void setDrawNormalsFlag(bool draw);
	void setDrawCenterPointFlag(bool draw);
	void setDrawConnectors(bool draw);

public:
	/// IAnimatable
	void animate();

	/// IGLTexturedRenderable
	void render(const float *mtxModelView, const float *mtxProj, const float *posSun);
	void setTextureManager(IGLTextureManager * manager);

	/// IGLShaderRenderable
	bool initShader(int type);
	bool bindShader(int type);
	void releaseShader(int type);

	/// WZM
	virtual operator Pie3Model() const;
	void write(std::ostream& out) const;

	bool importFromOBJ(std::istream& in, bool welder);
	void exportToOBJ(std::ostream& out) const;

	void addMesh (const Mesh& mesh);
	void rmMesh (int index);

private:
	Q_DISABLE_COPY(QWZM)
	void defaultConstructor();
	void drawAPoint(const WZMVertex &center, const WZMVertex &scale, const WZMVertex &color);
	void drawCenterPoint();
	void drawNormals();
	void drawConnectors();

	bool setupTextureUnits(int type);
	void clearTextureUnits(int type);

	void applyPendingChangesToModel(WZM& model) const;
	void resetAllPendingChanges();

	std::map<wzm_texture_type_t, GLuint> m_gl_textures;

	GLfloat scale_all, scale_xyz[3];
	static const GLint winding;

	int m_active_mesh;
	bool m_pending_changes;
	clock_t m_timeAnimationStarted;

	QColor m_tcmaskColour;

	bool m_drawNormals;
	bool m_drawCenterPoint;

	float m_animation_elapsed_msecs;
	bool m_drawConnectors;
};

#endif // QWZM_HPP
