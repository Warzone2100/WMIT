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

#include "Mesh.h"

#include <algorithm>
#include <functional>
#include <iterator>
#include <map>
#include <set>

#include <sstream>

#include "Generic.h"
#include "Util.h"
#include "Pie.h"
#include "Vector.h"

struct compareWZMPoint_less_wEps: public std::binary_function<WZMPoint&, WZMPoint&, bool>
{
	const WZMVertex::less_wEps vertLess;
	const WZMUV::less_wEps uvLess;
	const WZMVertex::equal_wEps vertEq;
	const WZMUV::equal_wEps uvEq;

public:
	compareWZMPoint_less_wEps(float vertEps = 0.0001, float uvEps = 0.0001):
		vertLess(vertEps), uvLess(uvEps), vertEq(vertEps), uvEq(uvEps) {}

	bool operator() (const WZMPoint& lhs, const WZMPoint& rhs) const
	{
		if (vertLess(std::tr1::get<0>(lhs), std::tr1::get<0>(rhs)))
		{
			return true;
		}
		else
		{
			if (vertEq(std::tr1::get<0>(lhs), std::tr1::get<0>(rhs)))
			{
				if (uvLess(std::tr1::get<1>(lhs), std::tr1::get<1>(rhs)))
				{
					return true;
				}
				else
				{
					if (uvEq(std::tr1::get<1>(lhs), std::tr1::get<1>(rhs)))
					{
						return vertLess(std::tr1::get<2>(lhs), std::tr1::get<2>(rhs));
					}
				}
			}
		}
		return false;
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
	WZMVertex tmpNrm;

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
		// pie2 integer-type problem?
		if (itL->getIndex(0) == itL->getIndex(1) || itL->getIndex(1) == itL->getIndex(2) || itL->getIndex(0) == itL->getIndex(2))
		{
			continue;
		}
		if (itL->m_texCoords[0] == itL->m_texCoords[1] || itL->m_texCoords[1] == itL->m_texCoords[2] || itL->m_texCoords[0] == itL->m_texCoords[2])
		{
			continue;
		}

		tmpNrm = WZMVertex(WZMVertex(p3.m_points[itL->getIndex(1)]) - WZMVertex(p3.m_points[itL->getIndex(0)]))
				.crossProduct(WZMVertex(p3.m_points[itL->getIndex(2)]) - WZMVertex(p3.m_points[itL->getIndex(0)]));
		tmpNrm.normalize();

		// For all 3 vertices of the triangle
		for (int i = 0; i < 3; ++i)
		{
			inResult = tupleSet.insert(WZMPoint(p3.m_points[itL->getIndex(i)], itL->getUV(i, 0), tmpNrm));

			if (!inResult.second)
			{
				iTri.operator[](i) = mapping[std::distance(tupleSet.begin(), inResult.first)];
			}
			else
			{
				itMap = mapping.begin();
				std::advance(itMap, std::distance(tupleSet.begin(), inResult.first));
				mapping.insert(itMap, vertices());
				iTri.operator[](i) = vertices();
				addPoint(*inResult.first);
			}
		}
		addIndices(iTri);
	}

	std::list<Pie3Connector>::const_iterator itC;

	// For each pie3 connector
	for (itC = p3.m_connectors.begin(); itC != p3.m_connectors.end(); ++itC)
	{
		addConnector(WZMConnector(itC->pos.operator[](0),
                                  itC->pos.operator[](1),
                                  itC->pos.operator[](2)));
	}

	finishImport();
	recalculateBoundData();
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
	if (in.fail() || str.compare(WZM_MESH_SIGNATURE) != 0)
	{
		std::cerr << "Mesh::read - Expected " << WZM_MESH_SIGNATURE << " directive found " << str;
		return false;
	}

	if (!isValidWzName(m_name))
	{
		std::cerr << "Mesh::read - Invalid mesh name: " << m_name;
		m_name = std::string();
	}

	in >> str >> m_teamColours;
	if (in.fail() || str.compare(WZM_MESH_DIRECTIVE_TEAMCOLOURS) != 0)
	{
		std::cerr << "Mesh::read - Expected " << WZM_MESH_DIRECTIVE_TEAMCOLOURS << " directive found " << str;
		return false;
	}

	in >> str;
	if (in.fail() || str.compare(WZM_MESH_DIRECTIVE_MINMAXTSCEN) != 0)
	{
		std::cerr << "Mesh::read - Expected " << WZM_MESH_DIRECTIVE_MINMAXTSCEN << " directive found " << str;
		return false;
	}
	else
	{
		// ignore those, they will be recalculated later (support for manual editing for example)
		float f;
		in >> f >> f >> f >> f >> f >> f >> f >> f >> f;
		if (in.fail())
		{
			std::cerr << "Mesh::read - Error reading minmaxtspcen values";
			return false;
		}
	}

	in >> str >> vertices;
	if (in.fail() || str.compare(WZM_MESH_DIRECTIVE_VERTICES) != 0)
	{
		std::cerr << "Mesh::read - Expected " << WZM_MESH_DIRECTIVE_VERTICES << " directive found " << str;
		return false;
	}

	in >> str >> indices;
	if (in.fail() || str.compare(WZM_MESH_DIRECTIVE_INDICES) != 0)
	{
		std::cerr << "Mesh::read - Expected " << WZM_MESH_DIRECTIVE_INDICES << " directive found " << str;
		return false;
	}

	in >> str;
	if (in.fail() || str.compare(WZM_MESH_DIRECTIVE_VERTEXARRAY) != 0)
	{
		std::cerr << "Mesh::read - Expected " << WZM_MESH_DIRECTIVE_VERTEXARRAY << " directive found " << str;
		return false;
	}

	reservePoints(vertices);

	WZMVertex vert, normal;
	WZMVertex4 tangent;
	WZMUV uv;

	for (; vertices > 0; --vertices)
	{
		in >> vert.x() >> vert.y() >> vert.z();
		if (in.fail())
		{
			std::cerr << "Mesh::read - Error reading vertex";
			return false;
		}
		m_vertexArray.push_back(vert);

		in >> uv.u() >> uv.v();
		if (in.fail())
		{
			std::cerr << "Mesh::read - Error reading uv coords.";
			return false;
		}
		else if (uv.u() > 1 || uv.v() > 1)
		{
			std::cerr << "Mesh::read - Error uv coords out of range";
			return false;
		}
		m_textureArray.push_back(uv);

		in >> normal.x() >> normal.y() >> normal.z();
		if (in.fail())
		{
			std::cerr << "Mesh::read - Error reading normal";
			return false;
		}
		m_normalArray.push_back(normal);

		in >> tangent.x() >> tangent.y() >> tangent.z() >> tangent.w();
		if (in.fail())
		{
			std::cerr << "Mesh::read - Error reading t";
			return false;
		}
		m_tangentArray.push_back(tangent);

		m_bitangentArray.push_back(normal.crossProduct(tangent.xyz()) * tangent.w());
	}

	in >> str;
	if (str.compare(WZM_MESH_DIRECTIVE_INDEXARRAY) != 0)
	{
		std::cerr << "Mesh::read - Expected " << WZM_MESH_DIRECTIVE_INDEXARRAY << " directive found " << str;
		return false;
	}

	reserveIndices(indices);
	for(; indices > 0; --indices)
	{
		IndexedTri tri;

		in >> tri.a() >> tri.b() >> tri.c();

		if (in.fail())
		{
			std::cerr << "Mesh::read - Error reading indices";
			return false;
		}
		m_indexArray.push_back(tri);
	}

	in >> str >> i;
	if (in.fail() || str.compare(WZM_MESH_DIRECTIVE_CONNECTORS) != 0)
	{
		std::cerr << "Mesh::read - Expected " << WZM_MESH_DIRECTIVE_CONNECTORS << " directive found " << str;
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
			return false;
		}
		m_connectors.push_back(con);
	}

	recalculateBoundData();

	return true;
}

void Mesh::write(std::ostream &out) const
{
	out << WZM_MESH_SIGNATURE << ' ' << (m_name.empty() ? "_noname_" : m_name ) << '\n';

	// noboolalpha should be default...
	out << WZM_MESH_DIRECTIVE_TEAMCOLOURS << " " << std::noboolalpha << teamColours() << '\n';

	out << WZM_MESH_DIRECTIVE_MINMAXTSCEN << " "
	    << m_mesh_aabb_min.x() << ' ' << m_mesh_aabb_min.y() << ' ' << m_mesh_aabb_min.z() << ' '
	    << m_mesh_aabb_max.x() << ' ' << m_mesh_aabb_max.y() << ' ' << m_mesh_aabb_max.z() << ' '
	    << m_mesh_tspcenter.x() << ' ' << m_mesh_tspcenter.y() << ' ' << m_mesh_tspcenter.z() << ' '
	    << '\n';

	out << WZM_MESH_DIRECTIVE_VERTICES << " " << vertices() << '\n';
	out << WZM_MESH_DIRECTIVE_INDICES << " " << indices() << '\n';

	out << WZM_MESH_DIRECTIVE_VERTEXARRAY << '\n';
	for (unsigned int i = 0; i < vertices(); ++i)
	{
		out << '\t';
		out << m_vertexArray[i].x() << ' ' << m_vertexArray[i].y() << ' ' << m_vertexArray[i].z() << ' ';
		out << m_textureArray[i].u() << ' ' << m_textureArray[i].v() << ' ';
		out << m_normalArray[i].x() << ' ' << m_normalArray[i].y() << ' ' << m_normalArray[i].z() << ' ';
		out << m_tangentArray[i].x() << ' ' << m_tangentArray[i].y() << ' ' << m_tangentArray[i].z() << ' '
		    << m_tangentArray[i].w() << '\n';
	}

	out << WZM_MESH_DIRECTIVE_INDEXARRAY << '\n';
	std::vector<IndexedTri>::const_iterator indIt;
	for (indIt = m_indexArray.begin(); indIt < m_indexArray.end(); ++indIt)
	{
		out << '\t';
		out << indIt->a() << ' ' << indIt->b() << ' ' << indIt->c() << '\n';
	}

	out << WZM_MESH_DIRECTIVE_CONNECTORS << " " << m_connectors.size() << "\n";
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
			 const std::vector<OBJVertex>&  normals,
			 bool welder)
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

	reservePoints(verts.size());
	reserveIndices(faces.size());

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

			if (welder)
			{
				inResult = tupleSet.insert(WZMPoint(verts[itFaces->tri[i]-1], tmpUv, tmpNrm));

				if (!inResult.second)
				{
					tmpTri[i] = mapping[std::distance(tupleSet.begin(), inResult.first)];
				}
				else
				{
					itMap = mapping.begin();
					std::advance(itMap, std::distance(tupleSet.begin(), inResult.first));
					mapping.insert(itMap, vertices());
					tmpTri[i] = vertices();
					addPoint(*inResult.first);
				}
			}
			else
			{
				tmpTri[i] = vertices();
				addPoint(WZMPoint(verts[itFaces->tri[i]-1], tmpUv, tmpNrm));
			}
		}
		addIndices(tmpTri);
	}

	finishImport();
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

inline int Mesh::connectors() const
{
	return m_connectors.size();
}

inline unsigned Mesh::vertices() const
{
	return m_vertexArray.size();
}

inline unsigned Mesh::frames() const
{
	return m_frameArray.size();
}

inline unsigned Mesh::indices() const
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
		if ((*it).a() >= vertices())
		{
			return false;
		}
		if ((*it).b() >= vertices())
		{
			return false;
		}
		if ((*it).c() >= vertices())
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
	m_tangentArray.clear();
	m_bitangentArray.clear();
	m_indexArray.clear();

	m_connectors.clear();
	m_teamColours = false;
}

inline void Mesh::reservePoints(const unsigned size)
{
	m_vertexArray.reserve(size);
	m_textureArray.reserve(size);
	m_normalArray.reserve(size);
	m_tangentArray.reserve(size);
	m_bitangentArray.reserve(size);
}

inline void Mesh::reserveIndices(const unsigned size)
{
	m_indexArray.reserve(size);
}

void Mesh::addPoint(const WZMPoint &point)
{
	m_vertexArray.push_back(std::tr1::get<0>(point));
	m_textureArray.push_back(std::tr1::get<1>(point));
	m_normalArray.push_back(std::tr1::get<2>(point));
	m_tangentArray.resize(m_tangentArray.size() + 1);
	m_bitangentArray.resize(m_bitangentArray.size() + 1);
}

void Mesh::addIndices(const IndexedTri &trio)
{
	// out of index
	if (trio.a() >= vertices() && trio.b() >= vertices() && trio.c() >= vertices())
	{
		return;
	}

	m_indexArray.push_back(trio);

	// TB-calculation part

	// Shortcuts for vertices
	WZMVertex &v0 = m_vertexArray[trio.a()];
	WZMVertex &v1 = m_vertexArray[trio.b()];
	WZMVertex &v2 = m_vertexArray[trio.c()];

	// Shortcuts for UVs
	WZMUV &uv0 = m_textureArray[trio.a()];
	WZMUV &uv1 = m_textureArray[trio.b()];
	WZMUV &uv2 = m_textureArray[trio.c()];

	// Edges of the triangle : postion delta
	WZMVertex deltaPos1 = v1 - v0;
	WZMVertex deltaPos2 = v2 - v0;

	// UV delta
	WZMUV deltaUV1 = uv1 - uv0;
	WZMUV deltaUV2 = uv2 - uv0;

	// check for nan
	float r = (deltaUV1.u() * deltaUV2.v() - deltaUV1.v() * deltaUV2.u());
	if (r)
		r = 1.f / r;

	WZMVertex4 tangent = WZMVertex((deltaPos1 * deltaUV2.v() - deltaPos2 * deltaUV1.v()) * r);
	WZMVertex bitangent = (deltaPos2 * deltaUV1.u() - deltaPos1 * deltaUV2.u()) * r;

	m_tangentArray[trio.a()] += tangent;
	m_tangentArray[trio.b()] += tangent;
	m_tangentArray[trio.c()] += tangent;

	m_bitangentArray[trio.a()] += bitangent;
	m_bitangentArray[trio.b()] += bitangent;
	m_bitangentArray[trio.c()] += bitangent;
}

void Mesh::finishImport()
{
	for (unsigned int i = 0; i < vertices(); ++i)
	{
		WZMVertex n = m_normalArray[i];

		// Gram-Schmidt orthogonalize
		m_tangentArray[i] = WZMVertex4(WZMVertex(m_tangentArray[i].xyz() - n * n.dotProduct(m_tangentArray[i].xyz())).normalize());

		// Calculate handedness
		if (n.crossProduct(m_tangentArray[i].xyz()).dotProduct(m_bitangentArray[i]) < 0.0f)
		{
			m_tangentArray[i].w() = -1.0f;
		}
		else
		{
			m_tangentArray[i].w() = 1.0f;
		}
	}
}

void Mesh::scale(GLfloat x, GLfloat y, GLfloat z)
{
	std::vector<WZMVertex>::iterator vertIt;
	for (vertIt = m_vertexArray.begin(); vertIt < m_vertexArray.end(); ++vertIt)
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
	for (unsigned int i = 0; i < vertices(); ++i)
	{
		switch (axis)
		{
		case 0:
			m_vertexArray[i].x() = -m_vertexArray[i].x() + 2 * point.x();
			m_normalArray[i].x() = -m_normalArray[i].x();
			m_tangentArray[i].x() = -m_tangentArray[i].x();
			m_bitangentArray[i].x() = -m_bitangentArray[i].x();
			break;
		case 1:
			m_vertexArray[i].y() = -m_vertexArray[i].y() + 2 * point.y();
			m_normalArray[i].y() = -m_normalArray[i].y();
			m_tangentArray[i].y() = -m_tangentArray[i].y();
			m_bitangentArray[i].y() = -m_bitangentArray[i].y();
			break;
		default:
			m_vertexArray[i].z() = -m_vertexArray[i].z() + 2 * point.z();
			m_normalArray[i].z() = -m_normalArray[i].z();
			m_tangentArray[i].z() = -m_tangentArray[i].z();
			m_bitangentArray[i].z() = -m_bitangentArray[i].z();
		}

		// Recalculate handedness
		if (m_normalArray[i].crossProduct(m_tangentArray[i].xyz()).dotProduct(m_bitangentArray[i]) < 0.0f)
		{
			m_tangentArray[i].w() = -1.0f;
		}
		else
		{
			m_tangentArray[i].w() = 1.0f;
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
	WZMVertex weight, min, max, vxmin, vxmax, vymin, vymax, vzmin, vzmax;

	if (!vertices())
	{
		return;
	}

	min = max = vxmax = vymax = vzmax = vxmin = vymin = vzmin = m_vertexArray.at(0);

	std::vector<WZMVertex>::const_iterator vertIt;
	for (vertIt = m_vertexArray.begin(); vertIt < m_vertexArray.end(); ++vertIt)
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

		if (vxmin.x() > vertIt->x()) vxmin = *vertIt;
		if (vymin.y() > vertIt->y()) vymin = *vertIt;
		if (vzmin.z() > vertIt->z()) vzmin = *vertIt;

		if (vxmax.x() < vertIt->x()) vxmax = *vertIt;
		if (vymax.y() < vertIt->y()) vymax = *vertIt;
		if (vzmax.z() < vertIt->z()) vzmax = *vertIt;
	}

	weight.x() /= vertices();
	weight.y() /= vertices();
	weight.z() /= vertices();

	m_mesh_weightcenter = weight;
	m_mesh_aabb_min = min;
	m_mesh_aabb_max = max;

// START: tight bounding sphere

	double dx, dy, dz, rad_sq, rad, old_to_p_sq, old_to_p, old_to_new;
	double xspan, yspan, zspan, maxspan;
	WZMVertex dia1, dia2, cen;

	// set xspan = distance between 2 points xmin & xmax (squared)
	dx = vxmax.x() - vxmin.x();
	dy = vxmax.y() - vxmin.y();
	dz = vxmax.z() - vxmin.z();
	xspan = dx*dx + dy*dy + dz*dz;

	// same for yspan
	dx = vymax.x() - vymin.x();
	dy = vymax.y() - vymin.y();
	dz = vymax.z() - vymin.z();
	yspan = dx*dx + dy*dy + dz*dz;

	// and ofcourse zspan
	dx = vzmax.x() - vzmin.x();
	dy = vzmax.y() - vzmin.y();
	dz = vzmax.z() - vzmin.z();
	zspan = dx*dx + dy*dy + dz*dz;

	// set points dia1 & dia2 to maximally seperated pair
	// assume xspan biggest
	dia1 = vxmin;
	dia2 = vxmax;
	maxspan = xspan;

	if (yspan > maxspan)
	{
		maxspan = yspan;
		dia1 = vymin;
		dia2 = vymax;
	}

	if (zspan > maxspan)
	{
		dia1 = vzmin;
		dia2 = vzmax;
	}

	// dia1, dia2 diameter of initial sphere
	cen.x() = (dia1.x() + dia2.x()) / 2.;
	cen.y() = (dia1.y() + dia2.y()) / 2.;
	cen.z() = (dia1.z() + dia2.z()) / 2.;

	// calc initial radius
	dx = dia2.x() - cen.x();
	dy = dia2.y() - cen.y();
	dz = dia2.z() - cen.z();

	rad_sq = dx*dx + dy*dy + dz*dz;
	rad = sqrt((double)rad_sq);

	// second pass (find tight sphere)
	for (vertIt = m_vertexArray.begin(); vertIt < m_vertexArray.end(); ++vertIt)
	{
		dx = vertIt->x() - cen.x();
		dy = vertIt->y() - cen.y();
		dz = vertIt->z() - cen.z();
		old_to_p_sq = dx*dx + dy*dy + dz*dz;

		// do r**2 first
		if (old_to_p_sq>rad_sq)
		{
			// this point outside current sphere
			old_to_p = sqrt((double)old_to_p_sq);
			// radius of new sphere
			rad = (rad + old_to_p) / 2.;
			// rad**2 for next compare
			rad_sq = rad*rad;
			old_to_new = old_to_p - rad;
			// centre of new sphere
			cen.x() = (rad * cen.x() + old_to_new * vertIt->x()) / old_to_p;
			cen.y() = (rad * cen.y() + old_to_new * vertIt->y()) / old_to_p;
			cen.z() = (rad * cen.z() + old_to_new * vertIt->z()) / old_to_p;
		}
	}

	m_mesh_tspcenter = cen;

// END: tight bounding sphere
}

WZMVertex Mesh::getCenterPoint() const
{
	WZMVertex center;

	center.x() = (m_mesh_aabb_max.x() + m_mesh_aabb_min.x()) / 2;
	center.y() = (m_mesh_aabb_max.y() + m_mesh_aabb_min.y()) / 2;
	center.z() = (m_mesh_aabb_max.z() + m_mesh_aabb_min.z()) / 2;

	return center;
}
