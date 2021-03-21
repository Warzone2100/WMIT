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
#ifndef WZM_HPP
#define WZM_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "Mesh.h"

#define WZM_MODEL_SIGNATURE "WZM"
#define WZM_MODEL_VERSION_FD 3 // First draft version

#define WZM_MODEL_DIRECTIVE_TEXTURE "TEXTURE"
#define WZM_MODEL_DIRECTIVE_TCMASK "TCMASK"
#define WZM_MODEL_DIRECTIVE_NORMALMAP "NORMALMAP"
#define WZM_MODEL_DIRECTIVE_SPECULARMAP "SPECULARMAP"
#define WZM_MODEL_DIRECTIVE_SHADERS "SHADERS"
#define WZM_MODEL_DIRECTIVE_MATERIAL "MATERIAL"
#define WZM_MODEL_DIRECTIVE_MESHES "MESHES"

class Pie3Model;

enum wzm_texture_type_t {WZM_TEX_DIFFUSE = 0, WZM_TEX_TCMASK, WZM_TEX_NORMALMAP, WZM_TEX_SPECULAR,
			 WZM_TEX__LAST, WZM_TEX__FIRST = WZM_TEX_DIFFUSE};

// do not reorder
enum wzm_material_t {WZM_MAT_EMISSIVE = 0, WZM_MAT_AMBIENT, WZM_MAT_DIFFUSE, WZM_MAT_SPECULAR,
		     WZM_MAT__LAST, WZM_MAT__FIRST = WZM_MAT_EMISSIVE};

struct WZMaterial
{
    const bool m_skipemissive;
    typedef WZMVertex val_type;
    val_type vals[WZM_MAT__LAST];
	GLfloat shininess;

    WZMaterial(bool skipemissive = false): m_skipemissive(skipemissive)
    {
        setDefaults();
    }

    const WZMaterial& operator=(const WZMaterial& rhs)
    {
        for (int i = WZM_MAT__FIRST; i < WZM_MAT__LAST; ++i)
            vals[i] = rhs.vals[i];
        shininess = rhs.shininess;
        return *this;
    }
	void setDefaults();
	bool isDefault() const;
};
std::istream& operator>> (std::istream& in, WZMaterial& mat);
std::ostream& operator<< (std::ostream& out, const WZMaterial& mat);

const static size_t MAX_CONNECTOR_COLORS = 10;
const static WZMVertex CONNECTOR_COLORS[MAX_CONNECTOR_COLORS] = {
	WZMVertex(1.f, 1.f, 0.f),
	WZMVertex(1.f, 0.9f, 0.2f),
	WZMVertex(1.f, 0.8f, 0.3f),
	WZMVertex(1.f, 0.7f, 0.4f),
	WZMVertex(1.f, 0.6f, 0.5f),
	WZMVertex(1.f, 0.5f, 0.6f),
	WZMVertex(1.f, 0.4f, 0.7f),
	WZMVertex(1.f, 0.3f, 0.8f),
	WZMVertex(1.f, 0.2f, 0.9f),
	WZMVertex(1.f, 0.1f, 1.f),
};

class WZM
{
public:
	WZM();
	WZM(const Pie3Model& p3);
	virtual ~WZM() {clear();}

	virtual operator Pie3Model() const;

	virtual bool read(std::istream& in);
	virtual void write(std::ostream& out) const;

	virtual bool importFromOBJ(std::istream& in, bool welder);
	virtual void exportToOBJ(std::ostream& out) const;

	virtual int version() const;
	virtual int meshes() const;

	virtual void setTextureName(wzm_texture_type_t type, std::string name);
	virtual std::string getTextureName(wzm_texture_type_t type) const;
	virtual bool isTextureSet(wzm_texture_type_t type) const;
	virtual void clearTextureNames();

	virtual WZMaterial getMaterial() const {return m_material;}
	virtual void setMaterial(const WZMaterial& mat) {m_material = mat;}

	static std::string texTypeToString(wzm_texture_type_t type);

	/// might throw out_of_range exception? not decided yet
	virtual Mesh& getMesh(int index);
	virtual void addMesh (const Mesh& mesh);
	virtual void rmMesh (int index);

	virtual bool isValid() const;
	virtual bool hasAnimObject (int mesh = -1) const;

	virtual void scale(GLfloat x, GLfloat y, GLfloat z, int mesh = -1);
	virtual void mirror(int axis, int mesh = -1); // x == 0, y == 1, z == 2
	virtual void reverseWinding(int mesh = -1);
	virtual void flipNormals(int mesh = -1);
	virtual void center(int mesh, int axis);
	virtual void recalculateTB(int mesh = -1);

	virtual WZMVertex calculateCenterPoint() const;
protected:
	virtual void clear();

	std::vector<Mesh> m_meshes;
	std::map<wzm_texture_type_t, std::string> m_textures;
	WZMaterial m_material;
	std::map<int, std::string> m_events;
	unsigned int m_ani_interpolate;
};

#endif // WZM_HPP
