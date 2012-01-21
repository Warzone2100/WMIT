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

#include "Mesh.hpp"

#include <algorithm>
#include <functional>
#include <iterator>
#include <map>
#include <set>

#include <cmath>

#include <sstream>

#ifdef LIB3DS_VERSION_1
#include <lib3ds/mesh.h>
#include <lib3ds/vector.h>
#include <lib3ds/matrix.h>
#include <lib3ds/material.h>
#else
#include <lib3ds.h>
#endif

#include "Generic.hpp"
#include "Util.hpp"
#include "Pie.hpp"
#include "Vector.hpp"

WZMVertex normalizeVector(const WZMVertex& ver)
{
	GLfloat sq = ver * ver;
	if (sq == 0.0f)
		return WZMVertex();
	return WZMVertex(ver / sqrt(sq));
}

struct compareWZMPoint_less_wEps: public std::binary_function<WZMPoint&, WZMPoint&, bool>
{
	const WZMVertex::less_wEps vertLess;
	const WZMUV::less_wEps uvLess;

public:
	compareWZMPoint_less_wEps(float vertEps = 0.0001, float uvEps = 0.0001):
		vertLess(vertEps), uvLess(uvEps) {}

	bool operator() (const WZMPoint& lhs, const WZMPoint& rhs) const
	{
		if (vertLess(std::get<0>(lhs), std::get<0>(rhs)))
		{
			return true;
		}
		else
		{
			if (uvLess(std::get<1>(lhs), std::get<1>(rhs)))
			{
				return true;
			}
			return vertLess(std::get<2>(lhs), std::get<2>(rhs));
		}
	}
};

WZMConnector::WZMConnector(GLfloat x, GLfloat y, GLfloat z):
	m_pos(x, y, z)
{
}

WZMConnector::WZMConnector(const WZMVertex &pos):
	m_pos(pos)
{
}

const WZMVertex& WZMConnector::getPos() const
{
	return m_pos;
}

Mesh::Mesh()
{
	defaultConstructor();
}

Mesh::Mesh(const Pie3Level& p3)
{
	std::vector<Pie3Polygon>::const_iterator itL;

	typedef std::set<WZMPoint, compareWZMPoint_less_wEps> t_tupleSet;
	t_tupleSet tupleSet;
	std::pair<t_tupleSet::iterator, bool> inResult;

	std::vector<unsigned> mapping;
	std::vector<unsigned>::iterator itMap;

	IndexedTri iTri;
	WZMVertex wzmVert, tmpNrm;
	WZMUV tmpUv;

	defaultConstructor();

	/*
	 *	Try to prevent duplicate vertices
	 *	(remember, different UV's, or animations,
	 *	 will cause unavoidable duplication)
	 *	so that our transformed vertex cache isn't
	 *	completely useless.
	 */

	// For each pie3 polygon
	for (itL = p3.m_polygons.begin(); itL != p3.m_polygons.end(); ++itL)
	{
		tmpNrm = normalizeVector(WZMVertex::crossProduct(WZMVertex(p3.m_points[itL->getIndex(1)]) - WZMVertex(p3.m_points[itL->getIndex(0)]),
							      WZMVertex(p3.m_points[itL->getIndex(2)]) - WZMVertex(p3.m_points[itL->getIndex(0)])));;

		// For all 3 vertices of the triangle
		for (int i = 0; i < 3; ++i)
		{
			wzmVert = p3.m_points[itL->getIndex(i)];
			tmpUv = itL->getUV(i, 0);

			inResult = tupleSet.insert(WZMPoint(wzmVert, tmpUv, tmpNrm));

			if (!inResult.second)
			{
				iTri[i] = mapping[std::distance(tupleSet.begin(), inResult.first)];
			}
			else
			{
				itMap = mapping.begin();
				std::advance(itMap, std::distance(tupleSet.begin(), inResult.first));
				mapping.insert(itMap, m_vertexArray.size());
				iTri[i] = m_vertexArray.size();
				m_vertexArray.push_back(wzmVert);
				m_textureArray.push_back(tmpUv);
				m_normalArray.push_back(tmpNrm);
			}
		}
		m_indexArray.push_back(iTri);
	}

	std::list<Pie3Connector>::const_iterator itC;

	// For each pie3 connector
	for (itC = p3.m_connectors.begin(); itC != p3.m_connectors.end(); ++itC)
	{
		addConnector(WZMConnector(itC->pos.operator[](0),
                                  itC->pos.operator[](1),
                                  itC->pos.operator[](2)));
	}

	recalculateBoundData();
}

Mesh::Mesh(const Lib3dsMesh& mesh3ds)
{
	const bool swapYZ = true;
	const bool reverseWinding = true;
	const bool invertV = true;
	const bool transform = true;

	typedef std::set<WZMPoint, compareWZMPoint_less_wEps> t_tupleSet;
	t_tupleSet tupleSet;

	std::pair<t_tupleSet::iterator, bool> inResult;

	std::vector<unsigned> mapping;
	std::vector<unsigned>::iterator itMap;

	unsigned i, j;

	IndexedTri idx; // temporaries
	WZMVertex tmpVert, tmpNorm;
	WZMUV tmpUV;

#ifdef LIB3DS_VERSION_1
	m_vertexArray.reserve(mesh3ds.points);
	m_textureArray.reserve(mesh3ds.points);
	m_normalArray.reserve(mesh3ds.points);
	m_indexArray.reserve(mesh3ds.faces);

	Lib3dsVector *normals = new Lib3dsVector[mesh3ds.faces * 3];
	lib3ds_mesh_calculate_normals(const_cast<Lib3dsMesh*>(&mesh3ds), normals);
#else
	m_vertexArray.reserve(mesh3ds.nvertices);
	m_textureArray.reserve(mesh3ds.nvertices);
	m_normalArray.reserve(mesh3ds.nvertices);
	m_indexArray.reserve(mesh3ds.nfaces);

	Lib3dsVector *normals = new Lib3dsVector[mesh3ds.nfaces * 3]; //FIXME
	lib3ds_mesh_calculate_normals(&mesh3ds, &normals); //FIXME
#endif

	if (isValidWzName(mesh3ds.name))
	{
		m_name = mesh3ds.name;
	}

#ifdef LIB3DS_VERSION_1
	for (i = 0; i < mesh3ds.faces; ++i)
	{
		Lib3dsFace* face = &mesh3ds.faceL[i];
#else
	for (i = 0; i < mesh3ds.nfaces; ++i)
	{
		Lib3dsFace* face = &mesh3ds.faces[i];
#endif

		for (j = 0; j < 3; ++j)
		{
#ifdef LIB3DS_VERSION_1
			Lib3dsVector pos;

			if (transform)
			{
				lib3ds_vector_transform(pos,
										const_cast<Lib3dsMatrix&>(mesh3ds.matrix),
										mesh3ds.pointL[face->points[j]].pos);
			}
			else
			{
				lib3ds_vector_copy(pos,
								   mesh3ds.pointL[face->points[j]].pos);
			}
#else
			float pos[3];

			if (transform)
			{
				lib3ds_vector_transform(pos,
										const_cast<float (*)[4]>(mesh3ds.matrix),
										mesh3ds.vertices[face->index[j]]);
			}
			else
			{
				lib3ds_vector_copy(pos,
								   mesh3ds.vertices[face->index[j]]);
			}
#endif

			if (swapYZ)
			{
				tmpVert.x() = pos[0];
				tmpVert.y() = pos[2];
				tmpVert.z() = pos[1];
			}
			else
			{
				tmpVert.x() = pos[0];
				tmpVert.y() = pos[1];
				tmpVert.z() = pos[2];
			}

#ifdef LIB3DS_VERSION_1
			if (mesh3ds.points == mesh3ds.texels)
			{
				tmpUV.u() = mesh3ds.texelL[face->points[j]][0];
				if (invertV)
				{
					tmpUV.v() = 1.0f - mesh3ds.texelL[face->points[j]][1];
				}
				else
				{
					tmpUV.v() = mesh3ds.texelL[face->points[j]][1];
				}
			}
			else
			{
				tmpUV = WZMUV();
			}
#else
			tmpUV.u() = mesh3ds.texcos[face->index[j]][0];
			if (invertV)
			{
				tmpUV.v() = 1.0f - mesh3ds.texcos[face->index[j]][1];
			}
			else
			{
				tmpUV.v() = mesh3ds.texcos[face->index[j]][1];
			}
#endif
			// normals
			if (swapYZ)
			{
				tmpNorm.x() = normals[i * 3 + j][0];
				tmpNorm.y() = normals[i * 3 + j][2];
				tmpNorm.z() = normals[i * 3 + j][1];
			}
			else
			{
				tmpNorm.x() = normals[i * 3 + j][0];
				tmpNorm.y() = normals[i * 3 + j][1];
				tmpNorm.z() = normals[i * 3 + j][2];
			}

			inResult = tupleSet.insert(WZMPoint(tmpVert, tmpUV, tmpNorm));

			if (!inResult.second)
			{
				idx[j] = mapping[std::distance(tupleSet.begin(), inResult.first)];
			}
			else
			{
				itMap = mapping.begin();
				std::advance(itMap, std::distance(tupleSet.begin(), inResult.first));
				mapping.insert(itMap, m_vertexArray.size());
				idx[j] = m_vertexArray.size();
				m_vertexArray.push_back(tmpVert);
				m_textureArray.push_back(tmpUV);
				m_normalArray.push_back(tmpNorm);
			}
		}

		if (reverseWinding)
		{
			std::swap(idx.b(), idx.c());
		}

		m_indexArray.push_back(idx);
	}

	recalculateBoundData();

	// TODO: Check if 3DS animation data can be used with our Frames
}

Mesh::~Mesh()
{
}

Pie3Level Mesh::backConvert(const Mesh& wzmMesh)
{
	return wzmMesh;
}

Mesh::operator Pie3Level() const
{
	Pie3Level p3;

	std::vector<Pie3Vertex>::iterator itPV;

	std::vector<IndexedTri>::const_iterator itTri;

	unsigned i;

	/* Note:
	 * WZM will have duplicates due to uv map differences
	 * so we remove those when converting
	 */

	for (itTri = m_indexArray.begin(); itTri != m_indexArray.end(); ++itTri)
	{
		Pie3Polygon p3Poly;
		Pie3UV	p3UV;

		p3Poly.m_flags = 0x200;

		for (i = 0; i < 3; ++i)
		{
			typedef Pie3Vertex::equal_wEps equals;
			mybinder1st<equals> compare(m_vertexArray[(*itTri)[i]]);

			itPV = std::find_if(p3.m_points.begin(), p3.m_points.end(), compare);

			if (itPV == p3.m_points.end())
			{
				// add it now
				p3Poly.m_indices[i] = p3.m_points.size();
				p3.m_points.push_back(m_vertexArray[(*itTri)[i]]);
			}
			else
			{
				p3Poly.m_indices[i] = std::distance(p3.m_points.begin(), itPV);
			}

			// TODO: deal with UV animation
			p3UV.u() = m_textureArray[(*itTri)[i]].u();
			p3UV.v() = m_textureArray[(*itTri)[i]].v();
			p3Poly.m_texCoords[i] = p3UV;
		}
		p3.m_polygons.push_back(p3Poly);
	}

	std::list<WZMConnector>::const_iterator itC;

	// For each WZM connector
	for (itC = m_connectors.begin(); itC != m_connectors.end(); ++itC)
	{
		Pie3Connector conn;
		conn.pos.operator[](0) = itC->getPos().operator[](0);
		conn.pos.operator[](1) = itC->getPos().operator[](1);
		conn.pos.operator[](2) = itC->getPos().operator[](2);
		p3.m_connectors.push_back(conn);
	}

	return p3;
}

bool Mesh::read(std::istream& in)
{
	std::string str;
	unsigned i,vertices,indices;

	clear();

	in >> str >> m_name;
	if (in.fail() || str.compare("MESH") != 0)
	{
		std::cerr << "Mesh::read - Expected MESH directive found " << str;
		clear();
		return false;
	}

	if (!isValidWzName(m_name))
	{
		std::cerr << "Mesh::read - Invalid Mesh name.";
		m_name = std::string();
	}

	in >> str >> m_teamColours;
	if (in.fail() || str.compare("TEAMCOLOURS") != 0)
	{
		std::cerr << "Mesh::read - Expected TEAMCOLOURS directive found " << str;
		clear();
		return false;
	}

	in >> str >> vertices;
	if (in.fail() || str.compare("VERTICES") != 0)
	{
		std::cerr << "Mesh::read - Expected VERTICES directive found " << str;
		clear();
		return false;
	}

	in >> str >> indices;
	if (in.fail() || str.compare("FACES") != 0)
	{
		std::cerr << "Mesh::read - Expected FACES directive found " << str;
		clear();
		return false;
	}

	in >> str;
	if (in.fail() || str.compare("VERTEXARRAY") !=0)
	{
		std::cerr << "Mesh::read - Expected VERTEXARRAY directive found " << str;
		clear();
		return false;
	}

	m_vertexArray.reserve(vertices);
	m_textureArray.reserve(vertices);
	m_normalArray.reserve(vertices);

	WZMVertex vert, normal;
	WZMUV uv;

	for (; vertices > 0; --vertices)
	{
		in >> vert.x() >> vert.y() >> vert.z();
		if (in.fail())
		{
			std::cerr << "Mesh::read - Error reading vertex";
			clear();
			return false;
		}
		m_vertexArray.push_back(vert);

		in >> uv.u() >> uv.v();
		if (in.fail())
		{
			std::cerr << "Mesh::read - Error reading uv coords.";
			clear();
			return false;
		}
		else if (uv.u() > 1 || uv.v() > 1)
		{
			std::cerr << "Mesh::read - Error uv coords out of range";
			clear();
			return false;
		}
		m_textureArray.push_back(uv);

		in >> normal.x() >> normal.y() >> normal.z();
		if (in.fail())
		{
			std::cerr << "Mesh::read - Error reading normal";
			clear();
			return false;
		}
		m_normalArray.push_back(normal);
	}

	in >> str;
	if (str.compare("INDEXARRAY") != 0)
	{
		std::cerr << "Mesh::read - Expected INDEXARRAY directive found " << str;
		clear();
		return false;
	}

	m_indexArray.reserve(indices);
	for(;indices>0;indices--)
	{
		IndexedTri tri;

		in >> tri.a() >> tri.b() >> tri.c();

		if (in.fail())
		{
			std::cerr << "Mesh::read - Error reading indices";
			clear();
			return false;
		}
		m_indexArray.push_back(tri);
	}

	in >> str >> i;
	if (in.fail() || str.compare("CONNECTORS") != 0)
	{
		std::cerr << "Mesh::read - Expected CONNECTORS directive found " << str;
		clear();
		return false;
	}

	if (i > 0)
	{
		WZMVertex con;

		for(; i > 0; --i)
		{
			in >> con.x() >> con.y() >> con.z();
		}
		if (in.fail())
		{
			std::cerr << "Mesh::read - Error reading connectors";
			clear();
			return false;
		}
		m_connectors.push_back(con);
	}

	recalculateBoundData();

	return true;
}

void Mesh::write(std::ostream &out) const
{
	out << "MESH ";
	if (m_name.empty())
	{
		out << "_noname_\n";
	}
	else
	{
		out << m_name << '\n';
	}

	// noboolalpha should be default...
	out << "TEAMCOLOURS " << std::noboolalpha << teamColours() << '\n';

	out << "VERTICES " << vertices() << '\n';
	out << "FACES " << faces() << '\n';

	out << "VERTEXARRAY\n" ;
	std::vector<WZMVertex>::const_iterator vertIt, normIt;
	std::vector<WZMUV>::const_iterator texIt;

	for (vertIt = m_vertexArray.begin(), texIt = m_textureArray.begin(), normIt = m_normalArray.begin();
	     vertIt < m_vertexArray.end();
	     ++vertIt, ++texIt, ++normIt)
	{
		out << '\t';
		out		<< vertIt->x() << ' ' << vertIt->y() << ' ' << vertIt->z() << ' '
				<< texIt->u() << ' ' << texIt->v() << ' '
				<< normIt->x() << ' ' << normIt->y() << ' ' << normIt->z() << '\n' ;
	}

	out << "INDEXARRAY\n";
	std::vector<IndexedTri>::const_iterator indIt;
	for (indIt=m_indexArray.begin(); indIt < m_indexArray.end(); indIt++ )
	{
		out << '\t';
		out		<< indIt->a() << ' '
				<< indIt->b() << ' '
				<< indIt->c() << '\n';
	}

	out <<"CONNECTORS " << m_connectors.size() << "\n";
	std::list<WZMConnector>::const_iterator conIt;
	for (conIt = m_connectors.begin(); conIt != m_connectors.end(); ++conIt)
	{
		WZMVertex con = conIt->getPos();
		out << '\t';
		out		<< con.x() << ' '
				<< con.y() << ' '
				<< con.z() << '\n';
	}
}

bool Mesh::importFromOBJ(const std::vector<OBJTri>&	faces,
			 const std::vector<OBJVertex>&  verts,
			 const std::vector<OBJUV>&	uvArray,
			 const std::vector<OBJVertex>&  normals)
{
	typedef std::set<WZMPoint, compareWZMPoint_less_wEps> t_tupleSet;
	t_tupleSet tupleSet;

	std::vector<OBJTri>::const_iterator itFaces;
	std::pair<t_tupleSet::iterator, bool> inResult;

	std::vector<unsigned> mapping;
	std::vector<unsigned>::iterator itMap;

	unsigned i;

	IndexedTri tmpTri;
	WZMUV tmpUv;
	WZMVertex tmpNrm;

	clear();

	for (itFaces = faces.begin(); itFaces != faces.end(); ++itFaces)
	{
		for (i = 0; i < 3; ++i)
		{
			/* in the uv's and nrm's, -1 is "not specified," but the OBJ indices
			 * are 0 based, hence < 1
			 */
			tmpUv = itFaces->uvs.operator [](i) < 1 ? WZMUV() : uvArray[itFaces->uvs.operator [](i) - 1];
#pragma message "precalculate missing OBJ normal"
			tmpNrm = itFaces->nrm.operator [](i) < 1 ? WZMVertex() : normals[itFaces->nrm.operator [](i) - 1]; //FIXME
			inResult = tupleSet.insert(WZMPoint(verts[itFaces->tri[i]-1], tmpUv, tmpNrm));

			if (!inResult.second)
			{
				tmpTri[i] = mapping[std::distance(tupleSet.begin(), inResult.first)];
			}
			else
			{
				itMap = mapping.begin();
				std::advance(itMap, std::distance(tupleSet.begin(), inResult.first));
				mapping.insert(itMap, m_vertexArray.size());
				tmpTri[i] = m_vertexArray.size();
				m_vertexArray.push_back(verts[itFaces->tri[i]-1]);
				m_textureArray.push_back(tmpUv);
				m_normalArray.push_back(tmpNrm);
			}
		}
		m_indexArray.push_back(tmpTri);
	}

	recalculateBoundData();

	return true;
}

std::stringstream* Mesh::exportToOBJ(const Mesh_exportToOBJ_InOutParams& params) const
{
	const bool invertV = true;
	std::stringstream* out = new std::stringstream;

	std::pair<std::set<OBJVertex, OBJVertex::less_wEps>::iterator, bool> vertInResult;
	std::pair<std::set<OBJUV, OBJUV::less_wEps>::iterator, bool> uvInResult;
	std::pair<std::set<OBJVertex, OBJVertex::less_wEps>::iterator, bool> normInResult;

	std::vector<IndexedTri>::const_iterator itF;
	std::vector<unsigned>::iterator itMap;
	unsigned i;

	OBJUV uv;

	*out << "o " << m_name << "\n";

	for (itF = m_indexArray.begin(); itF != m_indexArray.end(); ++itF)
	{
		*out << "f";

		for (i = 0; i < 3; ++i)
		{
			*out << ' ';

			vertInResult = params.vertSet->insert(m_vertexArray[itF->operator [](i)]);

			if (!vertInResult.second)
			{
				*out << (*params.vertMapping)[std::distance(params.vertSet->begin(), vertInResult.first)] + 1;
			}
			else
			{
				itMap = params.vertMapping->begin();
				std::advance(itMap, std::distance(params.vertSet->begin(), vertInResult.first));
				params.vertMapping->insert(itMap, params.vertices->size());
				params.vertices->push_back(m_vertexArray[itF->operator [](i)]);
				*out << params.vertices->size();
			}

			*out << '/';

			uv = m_textureArray[itF->operator [](i)];
			if (invertV)
			{
				uv.v() = 1 - uv.v();
			}
			uvInResult = params.uvSet->insert(uv);

			if (!uvInResult.second)
			{
				*out << (*params.uvMapping)[std::distance(params.uvSet->begin(), uvInResult.first)] + 1;
			}
			else
			{
				itMap = params.uvMapping->begin();
				std::advance(itMap, std::distance(params.uvSet->begin(), uvInResult.first));
				params.uvMapping->insert(itMap, params.uvs->size());
				params.uvs->push_back(uv);
				*out << params.uvs->size();
			}

			*out << '/';

			normInResult = params.normSet->insert(m_normalArray[itF->operator [](i)]);

			if (!normInResult.second)
			{
				*out << (*params.normMapping)[std::distance(params.normSet->begin(), normInResult.first)] + 1;
			}
			else
			{
				itMap = params.normMapping->begin();
				std::advance(itMap, std::distance(params.normSet->begin(), normInResult.first));
				params.normMapping->insert(itMap, params.normals->size());
				params.normals->push_back(m_normalArray[itF->operator [](i)]);
				*out << params.normals->size();
			}
		}
		*out << '\n';
	}

	return out;
}

Mesh::operator Lib3dsMesh*() const
{
	const bool swapYZ = true;
	const bool reverseWinding = true;
	const bool invertV = true;

	Lib3dsMesh* mesh;
	unsigned i;

	if (m_name.length() >= 64)
	{
		char name[64];
		i = m_name.copy(name, 64 - 1);
		name[i] = '\0';
		mesh = lib3ds_mesh_new(name);
	}
	else
	{
		mesh = lib3ds_mesh_new(m_name.c_str());
	}

#ifdef LIB3DS_VERSION_1
	lib3ds_mesh_new_point_list(mesh, m_vertexArray.size());
	lib3ds_mesh_new_texel_list(mesh, m_vertexArray.size());

	for (i = 0; i < mesh->points; ++i)
	{
		if (swapYZ)
		{
			mesh->pointL[i].pos[0] = m_vertexArray[i].x();
			mesh->pointL[i].pos[2] = m_vertexArray[i].y();
			mesh->pointL[i].pos[1] = m_vertexArray[i].z();
		}
		else
		{
			mesh->pointL[i].pos[0] = m_vertexArray[i].x();
			mesh->pointL[i].pos[1] = m_vertexArray[i].y();
			mesh->pointL[i].pos[2] = m_vertexArray[i].z();
		}

		mesh->texelL[i][0] = m_textureArray[i].u();
		if (invertV)
		{
			mesh->texelL[i][1] = 1.0f - m_textureArray[i].v();
		}
		else
		{
			mesh->texelL[i][1] = m_textureArray[i].v();
		}

	}

	lib3ds_mesh_new_face_list(mesh, m_indexArray.size());

	for (i = 0; i < mesh->faces; ++i)
	{
		if (reverseWinding)
		{
			mesh->faceL[i].points[2] = m_indexArray[i].a();
			mesh->faceL[i].points[1] = m_indexArray[i].b();
			mesh->faceL[i].points[0] = m_indexArray[i].c();
		}
		else
		{
			mesh->faceL[i].points[0] = m_indexArray[i].a();
			mesh->faceL[i].points[1] = m_indexArray[i].b();
			mesh->faceL[i].points[2] = m_indexArray[i].c();
		}
	}
#else
	lib3ds_mesh_resize_vertices(mesh, m_vertexArray.size(), 1, 0);

	for (i = 0; i < mesh->nvertices; ++i)
	{
		if (swapYZ)
		{
			mesh->vertices[i][0] = m_vertexArray[i].x();
			mesh->vertices[i][2] = m_vertexArray[i].y();
			mesh->vertices[i][1] = m_vertexArray[i].z();
		}
		else
		{
			mesh->vertices[i][0] = m_vertexArray[i].x();
			mesh->vertices[i][1] = m_vertexArray[i].y();
			mesh->vertices[i][2] = m_vertexArray[i].z();
		}
		
		mesh->texcos[i][0] = m_textureArrays[0][i].u();
		if (invertV)
		{
			mesh->texcos[i][1] = 1.0f - m_textureArrays[0][i].v();
		}
		else
		{
			mesh->texcos[i][1] = m_textureArrays[0][i].v();
		}
		
	}

	lib3ds_mesh_resize_faces(mesh, m_indexArray.size());

	for (i = 0; i < mesh->nfaces; ++i)
	{
		if (reverseWinding)
		{
			mesh->faces[i].index[2] = m_indexArray[i].a();
			mesh->faces[i].index[1] = m_indexArray[i].b();
			mesh->faces[i].index[0] = m_indexArray[i].c();
		}
		else
		{
			mesh->faces[i].index[0] = m_indexArray[i].a();
			mesh->faces[i].index[1] = m_indexArray[i].b();
			mesh->faces[i].index[2] = m_indexArray[i].c();
		}
	}
#endif

	return mesh;
}

std::string Mesh::getName() const
{
	return m_name;
}

void Mesh::setName(const std::string& name)
{
	if (isValidWzName(name))
	{
		m_name = name;
	}
}

bool Mesh::teamColours() const
{
	return m_teamColours;
}

void Mesh::setTeamColours(bool tc)
{
	m_teamColours = tc;
}

const WZMConnector& Mesh::getConnector(int index) const
{
	std::list<WZMConnector>::const_iterator pos;
	std::advance(pos, index);
	return *pos;
}

void Mesh::addConnector (const WZMConnector& conn)
{
	m_connectors.push_back(conn);
}

void Mesh::rmConnector (int index)
{
	int i;
	std::list<WZMConnector>::iterator pos;
	for(i=0,pos=m_connectors.begin();i<index;i++,pos++);
	if(pos==m_connectors.end())
	{
		return;
	}
	m_connectors.erase(pos);
}

int Mesh::connectors() const
{
	return m_connectors.size();
}

unsigned Mesh::vertices() const
{
	return m_vertexArray.size();
}

unsigned Mesh::faces() const
{
	return triangles();
}

unsigned Mesh::triangles() const
{
	return m_indexArray.size();
}

unsigned Mesh::frames() const
{
	return m_frameArray.size();
}

unsigned Mesh::indices() const
{
	return m_indexArray.size();
}

bool Mesh::isValid() const
{
	// TODO: check m_frameArray, m_connectors
	if (!isValidWzName(m_name))
	{
		return false;
	}

	// Check that the values of the indices are in range
	std::vector<IndexedTri>::const_iterator it;
	for (it = m_indexArray.begin(); it != m_indexArray.end(); ++it)
	{
		if ((*it).a() >= m_vertexArray.size())
		{
			return false;
		}
		if ((*it).b() >= m_vertexArray.size())
		{
			return false;
		}
		if ((*it).c() >= m_vertexArray.size())
		{
			return false;
		}
	}
	return true;
}

void Mesh::defaultConstructor()
{
	m_name.clear();
	m_teamColours = false;
}

void Mesh::clear()
{
	m_name.clear();
	m_frameArray.clear();
	m_vertexArray.clear();
	m_textureArray.clear();
	m_normalArray.clear();
	m_indexArray.clear();
	m_connectors.clear();
	m_teamColours = false;
}

void Mesh::scale(GLfloat x, GLfloat y, GLfloat z)
{
	std::vector<WZMVertex>::iterator vertIt;
	for (vertIt = m_vertexArray.begin(); vertIt < m_vertexArray.end(); ++vertIt )
	{
		vertIt->scale(x, y, z);
	}

	std::list<WZMConnector>::iterator itC;
	for (itC = m_connectors.begin(); itC != m_connectors.end(); ++itC)
	{
		itC->m_pos.scale(x, y, z);
	}

	m_mesh_weightcenter.scale(x, y, z);
	m_mesh_aabb_min.scale(x, y, z);
	m_mesh_aabb_max.scale(x, y, z);

}

void Mesh::mirrorUsingLocalCenter(int axis)
{
	mirrorFromPoint(getCenterPoint(), axis);
}

void Mesh::mirrorFromPoint(const WZMVertex& point, int axis)
{
	std::vector<WZMVertex>::iterator vertIt;
	for (vertIt = m_vertexArray.begin(); vertIt < m_vertexArray.end(); ++vertIt )
	{
		switch (axis)
		{
		case 0:
			vertIt->x() = -vertIt->x() + 2 * point.x();
			break;
		case 1:
			vertIt->y() = -vertIt->y() + 2 * point.y();
			break;
		default:
			vertIt->z() = -vertIt->z() + 2 * point.z();
		}
	}

	std::list<WZMConnector>::iterator itC;
	for (itC = m_connectors.begin(); itC != m_connectors.end(); ++itC)
	{
		switch (axis)
		{
		case 0:
			itC->m_pos.operator[](0) = -itC->m_pos.operator[](0) + 2 * point.x();
			break;
		case 1:
			itC->m_pos.operator[](1) = -itC->m_pos.operator[](1) + 2 * point.y();
			break;
		default:
			itC->m_pos.operator[](2) = -itC->m_pos.operator[](2) + 2 * point.z();
		}
	}

	// for convenience
	reverseWinding();

	recalculateBoundData();
}

void Mesh::reverseWinding()
{
	std::vector<IndexedTri>::iterator it;
	for (it = m_indexArray.begin(); it != m_indexArray.end(); ++it)
	{
		std::swap((*it).b(), (*it).c());
	}
}

void Mesh::recalculateBoundData()
{
	WZMVertex weight, min, max;

	if (m_vertexArray.size())
	{
		min = max = m_vertexArray.at(0);

		std::vector<WZMVertex>::const_iterator vertIt;
		for (vertIt = m_vertexArray.begin(); vertIt < m_vertexArray.end(); ++vertIt )
		{
			weight.x() += vertIt->x();
			weight.y() += vertIt->y();
			weight.z() += vertIt->z();

			if (min.x() > vertIt->x()) min.x() = vertIt->x();
			if (min.y() > vertIt->y()) min.y() = vertIt->y();
			if (min.z() > vertIt->z()) min.z() = vertIt->z();

			if (max.x() < vertIt->x()) max.x() = vertIt->x();
			if (max.y() < vertIt->y()) max.y() = vertIt->y();
			if (max.z() < vertIt->z()) max.z() = vertIt->z();
		}

		weight.x() /= m_vertexArray.size();
		weight.y() /= m_vertexArray.size();
		weight.z() /= m_vertexArray.size();
	}

	m_mesh_weightcenter = weight;
	m_mesh_aabb_min = min;
	m_mesh_aabb_max = max;
}

WZMVertex Mesh::getCenterPoint() const
{
	WZMVertex center;

	center.x() = (m_mesh_aabb_max.x() + m_mesh_aabb_min.x()) / 2;
	center.y() = (m_mesh_aabb_max.y() + m_mesh_aabb_min.y()) / 2;
	center.z() = (m_mesh_aabb_max.z() + m_mesh_aabb_min.z()) / 2;

	return center;
}
