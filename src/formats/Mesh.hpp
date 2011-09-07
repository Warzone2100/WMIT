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

#include <GL/gl.h>

#include "VectorTypes.hpp"
#include "Polygon.hpp"

#include "OBJ.hpp"

typedef Vertex<GLfloat> WZMVertex;
typedef UV<GLclampf> WZMUV;
typedef std::vector<WZMUV> TexArray;

class WZMConnector
{
public:
	WZMConnector(){}
	virtual ~WZMConnector(){}
private:
	Vertex<GLfloat> m_pos;
	///TODO: Types for wzm connectors
#pragma message "TODO"
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
	Mesh(const Pie3Level& p3, float uvEps = 0.0001, float vertEps = -1);
	Mesh(const Lib3dsMesh& mesh3ds);
	virtual ~Mesh();

	static Pie3Level backConvert(const Mesh& wzmMesh);
	virtual operator Pie3Level() const;

	bool read(std::istream& in);
	void write(std::ostream& out) const;

	bool importFromOBJ(const std::vector<OBJTri>&	faces,
					   const std::vector<OBJVertex>& verts,
					   const std::vector<OBJUV>&	uvArray);

	std::stringstream* exportToOBJ(const Mesh_exportToOBJ_InOutParams& params) const;

	virtual operator Lib3dsMesh*() const;

	std::string getName() const;
	void setName(const std::string& name);

	bool teamColours() const;
	void setTeamColours(bool tc);

	WZMConnector& getConnector(int index);
	void addConnector (const WZMConnector& conn);
	void rmConnector (int index);
	int connectors() const;


	int textureArrays() const;
	const TexArray& getTexArray (int index) const;

	unsigned vertices() const;
	unsigned faces() const;
	unsigned triangles() const;
	unsigned indices() const;
	unsigned frames() const;

	bool isValid() const;
protected:
	void clear();
	std::string m_name;
	std::vector<Frame> m_frameArray;
	std::vector<WZMVertex> m_vertexArray;
	std::vector<TexArray> m_textureArrays;
	std::vector<IndexedTri> m_indexArray;
	std::list<WZMConnector> m_connectors;
	bool m_teamColours;
private:
	void defaultConstructor();
};


#endif // MESH_HPP
