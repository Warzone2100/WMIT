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

#ifndef OBJ_HPP
#define OBJ_HPP

#include <iostream>
#include <vector>
#include <set>

#include <GL/gl.h>

#include "VectorTypes.hpp"
#include "Polygon.hpp"

typedef Vertex<GLfloat> OBJVertex;
typedef UV<GLclampf> OBJUV;

struct OBJTri
{
	IndexedTri tri;
	// ignoring normals
	Vector<short, 3> uvs; // signed short: -1 means not specified
	bool operator == (const OBJTri& rhs)
	{
		return (tri == rhs.tri) && (uvs == rhs.uvs);
	}
	bool operator < (const OBJTri& rhs)
	{
		if (tri == rhs.tri)
		{
			return uvs < rhs.uvs;
		}
		return tri < rhs.tri;
	}
};
inline void writeOBJVertex(const OBJVertex& vert, std::ostream& out)
{
	out << "v " << vert.x() << ' '
			<< vert.y()  << ' '
			<< vert.z() << '\n';
}

inline void writeOBJUV(const OBJUV& uv, std::ostream& out)
{
	out << "vt " << uv.u() << ' '
			<< uv.v() << '\n';
}

/* Ugh, those wretched template parameters have a way of fuglying things up
 * so I'm burrying them here so that hopefuly no-one sees them...
 * these are function parameters, currently assumed to be valid pointers,
 * these are treated like references.
 */
struct Mesh_exportToOBJ_InOutParams
{
	std::vector<OBJVertex>* vertices;
	std::set<OBJVertex, OBJVertex::less_wEps>* vertSet;
	std::vector<unsigned>* vertMapping;
	std::vector<OBJUV>* uvs;
	std::set<OBJUV, OBJUV::less_wEps>* uvSet;
	std::vector<unsigned>* uvMapping;
};

#endif // OBJ_HPP
