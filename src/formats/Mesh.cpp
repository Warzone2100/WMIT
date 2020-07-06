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
#include <tuple>

#include <sstream>

#include "Generic.h"
#include "Util.h"
#include "Pie.h"
#include "Vector.h"
#include "Mesh.h"

typedef std::tuple<WZMVertex, WZMUV, WZMVertex> WZMPoint;

// Scale animation numbers from int to float
#define INT_SCALE       1000
static const float FROM_INT_SCALE = 0.001f;

struct compareWZMPoint_less_wEps: public std::binary_function<WZMPoint&, WZMPoint&, bool>
{
	const WZMVertex::less_wEps vertLess;
	const WZMUV::less_wEps uvLess;
	const WZMVertex::equal_wEps vertEq;
	const WZMUV::equal_wEps uvEq;

public:
	compareWZMPoint_less_wEps(float vertEps = 0.0001f, float uvEps = 0.0001f):
		vertLess(vertEps), uvLess(uvEps), vertEq(vertEps), uvEq(uvEps) {}

	bool operator() (const WZMPoint& lhs, const WZMPoint& rhs) const
	{
		if (vertLess(std::get<0>(lhs), std::get<0>(rhs)))
		{
			return true;
		}
		else
		{
			if (vertEq(std::get<0>(lhs), std::get<0>(rhs)))
			{
				if (uvLess(std::get<1>(lhs), std::get<1>(rhs)))
				{
					return true;
				}
				else
				{
					if (uvEq(std::get<1>(lhs), std::get<1>(rhs)))
					{
						return vertLess(std::get<2>(lhs), std::get<2>(rhs));
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

WZMVertex &WZMConnector::getPos()
{
	return m_pos;
}

Mesh::Mesh()
{
	clear();
}

Mesh::Mesh(const Pie3Level& p3)
{
	std::vector<Pie3Polygon>::const_iterator itL;

	typedef std::set<WZMPoint, compareWZMPoint_less_wEps> t_tupleSet;
	t_tupleSet tupleSet;
	std::pair<t_tupleSet::iterator, bool> inResult;

	std::vector<int> mapping;
	std::vector<int>::iterator itMap;

	IndexedTri iTri;
	TexAnimData texAnim;
	WZMVertex tmpNrm;
	WZMVertex v[3];

	auto nrmIt = p3.m_normals.begin();
	bool noPieNormals = p3.normals() == 0;
	bool hasTexAnim = false;

	clear();

	/*
	 *	Try to prevent duplicate vertices
	 *	(remember, different UV's, or animations,
	 *	 will cause unavoidable duplication)
	 *	so that our transformed vertex cache isn't
	 *	completely useless.
	 */

	reservePoints(p3.m_points.size());
	reserveIndices(p3.m_polygons.size());

	itL = p3.m_polygons.begin();
	hasTexAnim = (itL != p3.m_polygons.end()) && (itL->m_flags & 0x4000);
	if (hasTexAnim)
	{
		m_texAnimFrames = itL->m_frames;
		m_texAnimPlaybackRate = itL->m_playbackRate;
		reserveTexAnimation(p3.m_polygons.size());
	}

	// For each pie3 polygon
	for (; itL != p3.m_polygons.end(); ++itL)
	{
		// Presumably those issues are no longer present in newer models.
		// N.B. Make sure to advance normals when skipping with normals present
		if (noPieNormals)
		{
			// pie2 integer-type problem?
			if (itL->getIndex(0) == itL->getIndex(1) || itL->getIndex(1) == itL->getIndex(2) ||
					itL->getIndex(0) == itL->getIndex(2))
			{
				continue;
			}
			if (itL->m_texCoords[0] == itL->m_texCoords[1] || itL->m_texCoords[1] == itL->m_texCoords[2] ||
					itL->m_texCoords[0] == itL->m_texCoords[2])
			{
				continue;
			}
		}

		v[0] = WZMVertex(p3.m_points[itL->getIndex(0)]);
		v[1] = WZMVertex(p3.m_points[itL->getIndex(1)]);
		v[2] = WZMVertex(p3.m_points[itL->getIndex(2)]);

		if (noPieNormals)
			tmpNrm = WZMVertex(v[1] - v[0]).crossProduct(v[2] - v[0]).normalize();

		// For all 3 vertices of the triangle
		for (size_t i = 0; i < 3; ++i)
		{
			if (p3.normals() != 0)
				tmpNrm = *nrmIt++;

			inResult = tupleSet.insert(WZMPoint(v[i], itL->getUV(i, 0), tmpNrm));

			t_tupleSet::difference_type dist = std::distance(tupleSet.begin(), inResult.first);

			if (!inResult.second)
			{
				iTri[i] = static_cast<GLushort>(mapping[static_cast<size_t>(dist)]);
			}
			else
			{
				itMap = mapping.begin();
				std::advance(itMap, dist);
				mapping.insert(itMap, static_cast<int>(vertices()));
				iTri[i] = static_cast<GLushort>(vertices());

				const WZMPoint& curPoint(*inResult.first);
				addPoint(std::get<0>(curPoint), std::get<1>(curPoint), std::get<2>(curPoint));
			}
		}
		addIndices(iTri);
		if (hasTexAnim)
		{
			texAnim.width = itL->m_width;
			texAnim.height = itL->m_height;
			m_texAnimArray.emplace_back(texAnim);
		}
	}

	std::list<Pie3Connector>::const_iterator itC;

	// For each pie3 connector
	for (itC = p3.m_connectors.begin(); itC != p3.m_connectors.end(); ++itC)
	{
		addConnector(WZMConnector(itC->pos[0], itC->pos[2], itC->pos[1]));
	}

	// shaders
	m_shader_frag = p3.m_shader_frag;
	m_shader_vert = p3.m_shader_vert;

	// Anim object
	importPieAnimation(p3.m_animobj);

	finishImport();
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
	std::vector<TexAnimData>::const_iterator itTexAni;
	std::vector<IndexedTri>::const_iterator itTri;

	unsigned i;

	/* Note:
	 * WZM will have duplicates due to uv map differences
	 * so we remove those when converting
	 */

	IndexedTri tri;
	Pie3Polygon p3Poly;
	Pie3UV	p3UV;
	WZMVertex fixedVert;
	typedef Pie3Vertex::equal_wEps equals;

	p3Poly.m_flags = 0x200;
	if (m_texAnimFrames > 0)
	{
		p3Poly.m_flags |= 0x4000;
		p3Poly.m_frames = m_texAnimFrames;
		p3Poly.m_playbackRate = m_texAnimPlaybackRate;
	}

	itTexAni = m_texAnimArray.begin();

	for (itTri = m_indexArray.begin(); itTri != m_indexArray.end(); ++itTri)
	{
		tri = *itTri;
		for (i = 0; i < 3; ++i)
		{
			auto curIndex = tri[i];
			fixedVert = m_vertexArray[curIndex];
			mybinder1st<equals> compare(fixedVert, equals(0.0001f));

			itPV = std::find_if(p3.m_points.begin(), p3.m_points.end(), compare);

			if (itPV == p3.m_points.end())
			{
				// add it now
				p3Poly.m_indices[i] = p3.m_points.size();
				p3.m_points.push_back(fixedVert);
			}
			else
			{
				p3Poly.m_indices[i] = std::distance(p3.m_points.begin(), itPV);
			}

			p3UV.u() = m_textureArray[curIndex].u();
			p3UV.v() = m_textureArray[curIndex].v();
			p3Poly.m_texCoords[i] = p3UV;

			p3.m_normals.push_back(m_normalArray[curIndex]);
		}

		if (m_texAnimFrames > 0)
		{
			p3Poly.m_width = itTexAni->width;
			p3Poly.m_height = itTexAni->height;
			++itTexAni;
		}
		p3.m_polygons.push_back(p3Poly);
	}

	std::list<WZMConnector>::const_iterator itC;

	// For each WZM connector
	for (itC = m_connectors.begin(); itC != m_connectors.end(); ++itC)
	{
		Pie3Connector conn;
		conn.pos[0] = itC->getPos()[0];
		conn.pos[1] = itC->getPos()[2];
		conn.pos[2] = itC->getPos()[1];
		p3.m_connectors.push_back(conn);
	}

	// shaders
	p3.m_shader_frag = m_shader_frag;
	p3.m_shader_vert = m_shader_vert;

	// Anim object
	p3.m_animobj.time = m_frame_time;
	p3.m_animobj.cycles = m_frame_cycles;
	p3.m_animobj.numframes = static_cast<int>(m_frameArray.size());
	ApieAnimFrame p3Frame;
	int cur_num = 0;
	for (const auto& curFrame: m_frameArray)
	{
		p3Frame.num = cur_num++;
		p3Frame.pos = Vertex<int>(static_cast<int>(curFrame.trans.x() * INT_SCALE),
					  static_cast<int>(curFrame.trans.z() * INT_SCALE),
					  static_cast<int>(curFrame.trans.y() * INT_SCALE));
		p3Frame.rot = Vertex<int>(static_cast<int>(-curFrame.rot.x() * INT_SCALE),
					  static_cast<int>(-curFrame.rot.z() * INT_SCALE),
					  static_cast<int>(-curFrame.rot.y() * INT_SCALE));
		p3Frame.scale = Vertex<float>(curFrame.scale.x(), curFrame.scale.z(), curFrame.scale.y());
		p3.m_animobj.frames.push_back(p3Frame);
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

	unsigned int i;

	IndexedTri tmpTri;
	WZMUV tmpUv;
	WZMVertex tmpNrm, v[3];

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
			tmpUv = itFaces->uvs[i] < 1 ? WZMUV() : uvArray[static_cast<size_t>(itFaces->uvs[i] - 1)];

			if (itFaces->nrm[i] < 1)
			{
				v[0] = WZMVertex(verts[itFaces->tri[0]-1]);
				v[1] = WZMVertex(verts[itFaces->tri[1]-1]);
				v[2] = WZMVertex(verts[itFaces->tri[2]-1]);
				tmpNrm = WZMVertex(v[1] - v[0]).crossProduct(v[2] - v[0]).normalize();
			}
			else
				tmpNrm = normals[static_cast<size_t>(itFaces->nrm[i] - 1)];

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

					const WZMPoint& curPoint(*inResult.first);
					addPoint(std::get<0>(curPoint), std::get<1>(curPoint), std::get<2>(curPoint));
				}
			}
			else
			{
				tmpTri[i] = vertices();
				addPoint(verts[itFaces->tri[i]-1], tmpUv, tmpNrm);
			}
		}
		addIndices(tmpTri);
	}

	finishImport();

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
	std::list<WZMConnector>::const_iterator pos = m_connectors.begin();
	std::advance(pos, index);
	return *pos;
}

WZMConnector &Mesh::getConnector(int index)
{
	std::list<WZMConnector>::iterator pos = m_connectors.begin();
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

size_t Mesh::connectors() const
{
	return m_connectors.size();
}

void Mesh::replaceConnectors(const Mesh &fromMesh)
{
	m_connectors.clear();
	for (const auto& curConn: fromMesh.m_connectors)
		m_connectors.push_back(curConn);
}

size_t Mesh::vertices() const
{
	return m_vertexArray.size();
}

size_t Mesh::frames() const
{
	return m_frameArray.size();
}

size_t Mesh::indices() const
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

void Mesh::clear()
{
	m_name.clear();
	m_frame_time = m_frame_cycles = 0.f;
	m_texAnimFrames = m_texAnimPlaybackRate = 0;
	m_frameArray.clear();
	m_texAnimArray.clear();

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

void Mesh::reserveTexAnimation(const unsigned size)
{
	m_texAnimArray.reserve(size);
}

void Mesh::addPoint(const WZMVertex& vertex, const WZMUV& uv, const WZMVertex &normal)
{
	m_vertexArray.push_back(vertex);
	m_textureArray.push_back(uv);
	m_normalArray.push_back(normal);
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
	calculateTBForIndices(trio);
}

void Mesh::calculateTBForIndices(const IndexedTri &trio)
{
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

	m_tangentArray[trio.a()] -= tangent;
	m_tangentArray[trio.b()] -= tangent;
	m_tangentArray[trio.c()] -= tangent;

	m_bitangentArray[trio.a()] -= bitangent;
	m_bitangentArray[trio.b()] -= bitangent;
	m_bitangentArray[trio.c()] -= bitangent;
}

void Mesh::finishTBCalculation()
{
	for (unsigned int i = 0; i < vertices(); ++i)
	{
		WZMVertex n = m_normalArray[i];

		// Gram-Schmidt orthogonalize
		WZMVertex t = m_tangentArray[i].xyz();
		t = WZMVertex(t - n * n.dotProduct(t)).normalize();

		// Calculate handedness
		if (n.crossProduct(t).dotProduct(m_bitangentArray[i]) < 0.0f)
		{
			m_tangentArray[i] = WZMVertex4(t, -1.f);
		}
		else
		{
			m_tangentArray[i] = WZMVertex4(t, 1.f);
		}
	}
}

void Mesh::finishImport()
{
	finishTBCalculation();
	recalculateBoundData();
}

void Mesh::scale(GLfloat x, GLfloat y, GLfloat z)
{
	std::vector<WZMVertex>::iterator vertIt;
	for (vertIt = m_vertexArray.begin(); vertIt < m_vertexArray.end(); ++vertIt)
	{
		vertIt->scale(x, y, z);
	}

	if ((x < 0.f) || (y < 0.f) || (z < 0.f))
	{
		WZMVertex nrmScaler(x < 0.f ? -1.f: 1.f, y < 0.f ? -1.f: 1.f, z < 0.f ? -1.f: 1.f);
		WZMVertex4 tgtScaler(nrmScaler, 1.f);

		for (unsigned int i = 0; i < vertices(); ++i)
		{
			m_normalArray[i].operator*=(nrmScaler);
			m_tangentArray[i].operator*=(tgtScaler);
			m_bitangentArray[i].operator*=(nrmScaler);
		}
	}

	std::list<WZMConnector>::iterator itC;
	for (itC = m_connectors.begin(); itC != m_connectors.end(); ++itC)
	{
		itC->m_pos.scale(x, y, z);
	}

	m_mesh_weightcenter.scale(x, y, z);
	m_mesh_aabb_min.scale(x, y, z);
	m_mesh_aabb_max.scale(x, y, z);

	// Update animation
	for (auto& curFrame: m_frameArray)
		curFrame.trans.scale(x, y, z);
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
			m_tangentArray[i].w() = -1.0f;
		else
			m_tangentArray[i].w() = 1.0f;
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

	// Update animation
	/*
	for (auto& curFrame: m_frameArray)
	{

	}
	*/
}

void Mesh::reverseWinding()
{
	std::vector<IndexedTri>::iterator it;
	for (it = m_indexArray.begin(); it != m_indexArray.end(); ++it)
	{
		std::swap((*it).b(), (*it).c());
	}
}

void Mesh::flipNormals()
{
	std::vector<WZMVertex>::iterator nit;
	for (nit = m_normalArray.begin(); nit != m_normalArray.end(); ++nit)
	{
		nit->scale(-1.,-1.,-1.);
	}

	for (unsigned int i = 0; i < vertices(); ++i)
	{
		// Recalculate handedness
		if (m_normalArray[i].crossProduct(m_tangentArray[i].xyz()).dotProduct(m_bitangentArray[i]) < 0.0f)
			m_tangentArray[i].w() = -1.0f;
		else
			m_tangentArray[i].w() = 1.0f;
	}
}

void Mesh::move(const WZMVertex &moveby)
{
	for (size_t i = 0; i < vertices(); ++i)
	{
		m_vertexArray[i] += moveby;
	}
	m_mesh_weightcenter += moveby;
	m_mesh_aabb_min += moveby;
	m_mesh_aabb_max += moveby;
	m_mesh_tspcenter += moveby;
}

void Mesh::center(int axis)
{
	if (m_mesh_weightcenter == WZMVertex())
		return;

	WZMVertex moveby = WZMVertex() - getCenterPoint();

	switch (axis)
	{
	case 0:
		moveby.y() = moveby.z() = 0.f;
		break;
	case 1:
		moveby.x() = moveby.z() = 0.f;
		break;
	case 2:
		moveby.x() = moveby.y() = 0.f;
		break;
	}

	move(moveby);
}

void Mesh::recalculateTB()
{
	size_t vert_num = vertices();

	// Zero-out arrays
	m_tangentArray.resize(vert_num);
	std::fill(m_tangentArray.begin(), m_tangentArray.end(), WZMVertex4());
	m_bitangentArray.resize(vert_num);
	std::fill(m_bitangentArray.begin(), m_bitangentArray.end(), WZMVertex());

	// TB-calculation part
	for (auto& curTrio : m_indexArray)
		calculateTBForIndices(curTrio);
	finishTBCalculation();
}

void Mesh::importPieAnimation(const ApieAnimObject &animobj)
{
	// replace current animation
	m_frameArray.clear();
	m_frameArray.reserve(animobj.frames.size());

	m_frame_time = animobj.time;
	m_frame_cycles = animobj.cycles;

	Frame curFrame;
	for (const auto& pieFrame: animobj.frames)
	{
		curFrame.trans = WZMVertex(pieFrame.pos.x(), pieFrame.pos.z(), pieFrame.pos.y());
		curFrame.trans.scale(FROM_INT_SCALE, FROM_INT_SCALE, FROM_INT_SCALE);
		curFrame.rot = WZMVertex(-pieFrame.rot.x(), -pieFrame.rot.z(), -pieFrame.rot.y());
		curFrame.rot.scale(FROM_INT_SCALE, FROM_INT_SCALE, FROM_INT_SCALE);
		curFrame.scale = WZMVertex(pieFrame.scale.x(), pieFrame.scale.z(), pieFrame.scale.y());
		m_frameArray.push_back(curFrame);
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
