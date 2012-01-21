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
#include <tuple>

#include <QtOpenGL/qgl.h>

#include "VectorTypes.hpp"
#include "Polygon.hpp"

#include "OBJ.hpp"

typedef Vertex<GLfloat> WZMVertex;
typedef UV<GLclampf> WZMUV;
typedef std::tuple<WZMVertex, WZMUV, WZMVertex> WZMPoint;

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
	GLfloat time;
	GLfloat xTrans, yTrans, zTrans;
	GLfloat xRot, yRot, zRot;
};

class Pie3Level;
class Lib3dsMesh;
struct Mesh_exportToOBJ_InOutParams;

class Mesh
{
	friend class QWZM; // For rendering
public:
	Mesh();
	Mesh(const Pie3Level& p3);
	Mesh(const Lib3dsMesh& mesh3ds);
	virtual ~Mesh();

	static Pie3Level backConvert(const Mesh& wzmMesh);
	virtual operator Pie3Level() const;

	bool read(std::istream& in);
	void write(std::ostream& out) const;

	bool importFromOBJ(const std::vector<OBJTri>&	faces,
			   const std::vector<OBJVertex>& verts,
			   const std::vector<OBJUV>&	uvArray,
			   const std::vector<OBJVertex>& normals);

	std::stringstream* exportToOBJ(const Mesh_exportToOBJ_InOutParams& params) const;

	virtual operator Lib3dsMesh*() const;

	std::string getName() const;
	void setName(const std::string& name);

	bool teamColours() const;
	void setTeamColours(bool tc);

	const WZMConnector& getConnector(int index) const;
	void addConnector (const WZMConnector& conn);
	void rmConnector (int index);
	int connectors() const;

	unsigned vertices() const;
	unsigned faces() const;
	unsigned triangles() const;
	unsigned indices() const;
	unsigned frames() const;

	bool isValid() const;

	void scale(GLfloat x, GLfloat y, GLfloat z);
	void mirrorUsingLocalCenter(int axis); // x == 0, y == 1, z == 2
	void mirrorFromPoint(const WZMVertex& point, int axis); // x == 0, y == 1, z == 2
	void reverseWinding();

	WZMVertex getCenterPoint() const;

protected:
	std::string m_name;
	std::vector<Frame> m_frameArray;

	std::vector<WZMVertex> m_vertexArray;
	std::vector<WZMUV> m_textureArray;
	std::vector<WZMVertex> m_normalArray;
	std::vector<IndexedTri> m_indexArray;

	std::list<WZMConnector> m_connectors;

	bool m_teamColours;
	WZMVertex m_mesh_weightcenter, m_mesh_aabb_min, m_mesh_aabb_max;

	void clear();
	void recalculateBoundData();
private:
	void defaultConstructor();
};

WZMVertex normalizeVector(const WZMVertex &ver);


#endif // MESH_HPP
