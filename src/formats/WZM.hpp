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

#include "Mesh.hpp"

class Pie3Model;

enum wzm_texture_type_t {WZM_TEX_DIFFUSE = 0, WZM_TEX_TCMASK, WZM_TEX_NORMALMAP,
			 WZM_TEX__LAST, WZM_TEX__FIRST = WZM_TEX_DIFFUSE};

enum wzm_material_t {WZM_MAT_EMISSIVE = 0, WZM_MAT_AMBIENT, WZM_MAT_DIFFUSE, WZM_MAT_SPECULAR,
		     WZM_MAT__LAST, WZM_MAT__FIRST = WZM_MAT_EMISSIVE};

struct WZMaterial
{
	GLfloat vals[WZM_MAT__LAST][4];
	GLfloat shininess;

	WZMaterial(): shininess(10.f)
	{
		for (int i = WZM_MAT__FIRST; i < WZM_MAT__LAST; ++i)
		{
			wzm_material_t mattype = static_cast<wzm_material_t>(i);
			vals[mattype][0] = vals[mattype][1] = vals[mattype][2] = vals[mattype][3] = 1.f;
		}
	}
};


class WZM
{
public:
	WZM();
	WZM(const Pie3Model& p3);
	virtual ~WZM(){}

	virtual operator Pie3Model() const;

	bool read(std::istream& in);
	void write(std::ostream& out) const;

	bool importFromOBJ(std::istream& in);
	void exportToOBJ(std::ostream& out) const;

	bool importFrom3DS(std::string fileName);
	bool exportTo3DS(std::string fileName) const;

	int version() const;
	int meshes() const;

	void setTextureName(wzm_texture_type_t type, std::string name);
	std::string getTextureName(wzm_texture_type_t type) const;
	void clearTextureNames();

	static std::string texTypeToString(wzm_texture_type_t type);

	bool couldHaveTCArrays() const;

	/// might throw out_of_range exception? not decided yet
	Mesh& getMesh(int index);
	void addMesh (const Mesh& mesh);
	void rmMesh (int index);

	bool isValid() const;

	void scale(GLfloat x, GLfloat y, GLfloat z, int mesh = -1);
	void mirror(int axis, int mesh = -1); // x == 0, y == 1, z == 2
	void reverseWinding(int mesh = -1);

	WZMVertex calculateCenterPoint() const;

protected:
	void clear();

	std::vector<Mesh> m_meshes;
	std::vector<WZMaterial> m_materials;
	std::map<wzm_texture_type_t, std::string> m_textures;
};

#endif // WZM_HPP
