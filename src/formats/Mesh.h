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

#ifndef MESH_HPP
#define MESH_HPP

#include <iostream>
#include <string>
#include <vector>
#include <list>

#include <QtOpenGL/qgl.h>

#include "VectorTypes.h"
#include "Polygon.h"

#include "OBJ.h"

#define WZM_MESH_SIGNATURE "MESH"
#define WZM_MESH_DIRECTIVE_TEAMCOLOURS "TEAMCOLOURS"
#define WZM_MESH_DIRECTIVE_MINMAXTSCEN "MINMAX_TSCEN"
#define WZM_MESH_DIRECTIVE_VERTICES "VERTICES"
#define WZM_MESH_DIRECTIVE_INDICES "INDICES"
#define WZM_MESH_DIRECTIVE_VERTEXARRAY "VERTEXARRAY"
#define WZM_MESH_DIRECTIVE_INDEXARRAY "INDEXARRAY"
#define WZM_MESH_DIRECTIVE_CONNECTORS "CONNECTORS"


typedef Vertex<GLfloat> WZMVertex;
typedef Vertex4<GLfloat> WZMVertex4;
typedef UV<GLclampf> WZMUV;

class Mesh;

class WZMConnector
{
	friend class Mesh;
public:
	WZMConnector(GLfloat x = 0., GLfloat y = 0., GLfloat z = 0.);
	WZMConnector(const WZMVertex& pos);
	virtual ~WZMConnector(){}

	const WZMVertex& getPos() const;
private:
	WZMVertex m_pos;
	///TODO: Types for wzm connectors
};

struct Frame
{
	WZMVertex trans, rot, scale;
};

class Pie3Level;
struct Mesh_exportToOBJ_InOutParams;

class Mesh
{
	friend class QWZM; // For rendering
public:
	Mesh();
	Mesh(const Pie3Level& p3);
	virtual ~Mesh();

	static Pie3Level backConvert(const Mesh& wzmMesh);
	virtual operator Pie3Level() const;

	bool read(std::istream& in);
	void write(std::ostream& out) const;

	bool importFromOBJ(const std::vector<OBJTri>&	faces,
			   const std::vector<OBJVertex>& verts,
			   const std::vector<OBJUV>&	uvArray,
			   const std::vector<OBJVertex>& normals,
			   bool welder);
	std::stringstream* exportToOBJ(const Mesh_exportToOBJ_InOutParams& params) const;

	std::string getName() const;
	void setName(const std::string& name);

	bool teamColours() const;
	void setTeamColours(bool tc);

	const WZMConnector& getConnector(int index) const;
	void addConnector (const WZMConnector& conn);
	void rmConnector (int index);
	size_t connectors() const;

	size_t vertices() const;
	size_t indices() const;
	size_t frames() const;

	bool isValid() const;

	void scale(GLfloat x, GLfloat y, GLfloat z);
	void mirrorUsingLocalCenter(int axis); // x == 0, y == 1, z == 2
	void mirrorFromPoint(const WZMVertex& point, int axis); // x == 0, y == 1, z == 2
	void reverseWinding();
	void flipNormals();

	WZMVertex getCenterPoint() const;
protected:
	std::string m_name;
	int m_frame_time, m_frame_cycles;
	std::vector<Frame> m_frameArray;

	std::vector<WZMVertex> m_vertexArray;
	std::vector<WZMUV> m_textureArray;
	std::vector<WZMVertex> m_normalArray;
	std::vector<WZMVertex4> m_tangentArray;
	std::vector<WZMVertex> m_bitangentArray;
	std::vector<IndexedTri> m_indexArray;

	std::list<WZMConnector> m_connectors;
	std::string m_shader_vert;
	std::string m_shader_frag;

	bool m_teamColours;
	WZMVertex m_mesh_weightcenter, m_mesh_aabb_min, m_mesh_aabb_max, m_mesh_tspcenter;

	void clear();
	void reservePoints(const unsigned size);
	void reserveIndices(const unsigned size);
	void addIndices(const IndexedTri& trio);
	void addPoint(const WZMVertex &vertex, const WZMUV &uv, const WZMVertex &normal);
	void finishImport();

	void recalculateBoundData();
private:
	void defaultConstructor();
};

#endif // MESH_HPP
