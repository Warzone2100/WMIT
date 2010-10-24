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
#include <iterator>
#include <map>
#include <set>

#include <cmath>

#include <sstream>

#include <lib3ds/mesh.h>
#include <lib3ds/vector.h>
#include <lib3ds/matrix.h>
#include <lib3ds/material.h>

#include "Generic.hpp"
#include "Util.hpp"
#include "Pie.hpp"
#include "Vector.hpp"

Mesh::Mesh()
{
	defaultConstructor();
}

Mesh::Mesh(const Pie3Level& p3, float uvEps, float vertEps)
{
	std::vector<Pie3Polygon>::const_iterator itL;

	WZMVertex::less_wEps compare(vertEps);
	std::multimap<WZMVertex, unsigned, WZMVertex::less_wEps> tmpMMap(compare);

	std::multimap<WZMVertex, unsigned>::iterator itMM;
	std::pair<std::multimap<WZMVertex, unsigned>::iterator,
				std::multimap<WZMVertex, unsigned>::iterator> ret;

	unsigned vIdx;
	unsigned vert;
	unsigned frame;
	bool foundMatch;

	defaultConstructor();

	/*
	 *	Try to prevent duplicate vertices
	 *	(remember, different UV's, or animations,
	 *	 will cause unavoidable duplication)
	 *	so that our transformed vertex cache isn't
	 *	completely useless.
	 */

	if (m_textureArrays.size() == 0)
	{
		m_textureArrays.push_back(TexArray());
	}

	// For each pie3 polygon
	for (itL = p3.m_polygons.begin(); itL != p3.m_polygons.end(); ++itL)
	{
		IndexedTri iTri;

		// For all 3 vertices of the triangle
		for (vert = 0; vert < 3; ++vert)
		{
			WZMVertex wzmVert = p3.m_points[itL->getIndex(vert)];
			ret = tmpMMap.equal_range(wzmVert);
			foundMatch = false;

			// Only check if match is possible
			if (itL->m_frames <= m_textureArrays.size())
			{
				// For each potential match
				for (itMM = ret.first; itMM != ret.second; ++itMM)
				{
					vIdx = itMM->second;

					// Compare each animation frame
					for (frame = 0; frame < itL->m_frames; ++frame)
					{
						// Approximate comparison, helps kill duplicates
						const WZMUV::equal_wEps compare(uvEps);
						if (!compare(m_textureArrays[frame][vIdx], itL->getUV(vert, frame)))
						{
							break; // Not equal
						}
					}

					// if all frames were equal
					if (!(frame < itL->m_frames))
					{
						foundMatch = true;
						break; // Stop looking
					}
				}
			}

			if (!foundMatch)
			{
				unsigned frames2Do = std::max(itL->m_frames, m_textureArrays.size());
				vIdx = m_vertexArray.size();
				// add the vertex to both the multimap and the vertex array
				m_vertexArray.push_back(wzmVert);
				tmpMMap.insert(std::make_pair(wzmVert, vIdx));

				m_textureArrays.reserve(frames2Do);

				// add the uv's to the texture arrays
				for (frame = 0; frame < frames2Do; ++frame)
				{
					if (m_textureArrays.size() < frame + 1)
					{
						// Expand the texture array arrays
						m_textureArrays.push_back(m_textureArrays.back());
						m_textureArrays[frame][vIdx] = itL->getUV(vert, frame % itL->m_frames);
					}
					else
					{
						m_textureArrays[frame].push_back(itL->getUV(vert, frame % itL->m_frames));
					}
				}
			}

			// Set the index
			iTri[vert] = vIdx;
		}

		m_indexArray.push_back(iTri);
	}
}

Mesh::Mesh(const Lib3dsMesh& mesh3ds)
{
	const bool swapYZ = true;
	const bool reverseWinding = true;
	const bool invertV = true;
	const bool transform = true;

	const WZMVertex::less_wEps vertLess; // For make_mypair
	const WZMUV::less_wEps uvLess;

	typedef std::set<mypair<WZMVertex, WZMUV,
		WZMVertex::less_wEps, WZMUV::less_wEps> > t_pairSet;

	t_pairSet pairSet;

	std::pair<t_pairSet::iterator, bool> inResult;

	std::vector<unsigned> mapping;
	std::vector<unsigned>::iterator itMap;

	unsigned i, j;

	IndexedTri idx; // temporaries
	WZMVertex tmpVert;
	WZMUV tmpUV;

	m_vertexArray.reserve(mesh3ds.points);

	m_indexArray.reserve(mesh3ds.faces);

	m_textureArrays.push_back(TexArray()); // only one supported from 3DS
	m_textureArrays[0].reserve(mesh3ds.points);

	if (isValidWzName(mesh3ds.name))
	{
		m_name = mesh3ds.name;
	}

	for (i = 0; i < mesh3ds.faces; ++i)
	{
		Lib3dsFace* face = &mesh3ds.faceL[i];

		for (j = 0; j < 3; ++j)
		{
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

			inResult = pairSet.insert(make_mypair(tmpVert,
												  tmpUV,
												  vertLess,
												  uvLess));

			if (!inResult.second)
			{
				idx[j] = mapping[std::distance(pairSet.begin(), inResult.first)];
			}
			else
			{
				itMap = mapping.begin();
				std::advance(itMap, std::distance(pairSet.begin(), inResult.first));
				mapping.insert(itMap, m_vertexArray.size());
				idx[j] = m_vertexArray.size();
				m_vertexArray.push_back(tmpVert);
				m_textureArrays[0].push_back(tmpUV);
			}
		}

		if (reverseWinding)
		{
			std::swap(idx.b(), idx.c());
		}

		m_indexArray.push_back(idx);
	}

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

#pragma message "unfinished"
			// TODO: deal with UV animation
			p3UV.u() = m_textureArrays[0][(*itTri)[i]].u();
			p3UV.v() = m_textureArrays[0][(*itTri)[i]].v();
			p3Poly.m_texCoords[i] = p3UV;
		}
		p3.m_polygons.push_back(p3Poly);
	}

	return p3;
}

bool Mesh::read(std::istream& in)
{
	std::string str;
	unsigned i,j,vertices,indices;
	GLfloat f;

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
	if (str.compare("VERTEXARRAY") !=0)
	{
		std::cerr << "Mesh::read - Expected VERTEXARRAY directive found " << str;
		clear();
		return false;
	}

	m_vertexArray.reserve(vertices);
	for (; vertices > 0; --vertices)
	{
		WZMVertex vert;
		in >> vert.x() >> vert.y() >> vert.z();
		if (in.fail())
		{
			std::cerr << "Mesh::read - Error reading number of faces";
			clear();
			return false;
		}
		m_vertexArray.push_back(vert);
	}

	in >> str >> i;
	if (in.fail() || str.compare("TEXTUREARRAYS") != 0)
	{
		std::cerr << "Mesh::read - Expected TEXTUREARRAYS directive found " << str;
		clear();
		return false;
	}

	m_vertexArray.reserve(i);
	for (; i > 0; --i)
	{
		std::vector<WZMUV> tmp;
		tmp.clear();

		// j is currently ignored
		in >> str >> j;
		if ( in.fail() || str.compare("TEXTUREARRAY") != 0)
		{
			std::cerr << "Mesh::read - Expected TEXTUREARRAY directive found " << str;
			clear();
			return false;
		}

		for (j = 0; j < m_vertexArray.size(); ++j)
		{
			WZMUV uv;
			in >> uv.u() >> uv.v();
			if (in.fail())
			{
				std::cerr << "Mesh::read - Error reading uv coords.";
				clear();
				return false;
			}
			else if(uv.u()>1||uv.v()>1)
			{
				std::cerr << "Mesh::read - Error uv coords out of range";
				clear();
				return false;
			}
			tmp.push_back(uv);
		}
		m_textureArrays.push_back(tmp);
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
	if (in.fail() || str.compare("FRAMES") != 0)
	{
		std::cerr << "Mesh::read - Expected FRAMES directive found " << str;
		clear();
		return false;
	}

	if (i > 0)
	{
		// TODO: animation frames
#pragma message "unfinished"
		for(; i > 0; --i)
		{
			in >> f >> f >> f >> f >> f >> f >> f >> f;
		}
		if (in.fail())
		{
			std::cerr << "Mesh::read - Error reading frames";
			clear();
			return false;
		}
	}

	in >> str >> i;
	if (in.fail() || str.compare("CONNECTORS") != 0)
	{
		std::cerr << "Mesh::read - Expected CONNECTORS directive found " << str;
		clear();
		return false;
	}

	if ( i > 0)
	{
		// TODO: Connectors
#pragma message "unfinished"
		for(; i > 0; --i)
		{
			in >> str >> f >> f >> f >> f >> f >> f ;
		}
		if(in.fail())
		{
			std::cerr << "Mesh::read - Error reading connectors";
			clear();
			return false;
		}
	}

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

	std::vector<WZMVertex>::const_iterator vertIt;
	for (vertIt=m_vertexArray.begin(); vertIt < m_vertexArray.end(); vertIt++ )
	{
		out << '\t';
		out		<< vertIt->x() << ' '
				<< vertIt->y() << ' '
				<< vertIt->z() << '\n';
	}

	out << "TEXTUREARRAYS " << textureArrays() << '\n';

	std::vector< std::vector<WZMUV> >::const_iterator texArrIt;
	for (texArrIt=m_textureArrays.begin(); texArrIt < m_textureArrays.end(); texArrIt++ )
	{
		out << "TEXTUREARRAY " << std::distance(m_textureArrays.begin(),texArrIt) << '\n';
		std::vector<WZMUV>::const_iterator texIt;
		for (texIt=texArrIt->begin(); texIt < texArrIt->end(); texIt++ )
		{
			out << '\t';
			out	<< texIt->u() << ' '
					<< texIt->v() << '\n';
		}
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

	// TODO: Frames and connectors
#pragma message "unfinished"
	out <<"FRAMES 0\nCONNECTORS 0\n";
}

bool Mesh::importFromOBJ(const std::vector<OBJTri>&	faces,
						 const std::vector<OBJVertex>& verts,
						 const std::vector<OBJUV>&	uvArray)
{
	const WZMVertex::less_wEps vertLess; // For make_mypair
	const WZMUV::less_wEps uvLess;

	typedef std::set<mypair<WZMVertex, WZMUV,
		WZMVertex::less_wEps, WZMUV::less_wEps> > t_pairSet;

	t_pairSet pairSet;

	std::vector<OBJTri>::const_iterator itF;

	std::pair<t_pairSet::iterator, bool> inResult;

	std::vector<unsigned> mapping;
	std::vector<unsigned>::iterator itMap;

	unsigned i;

	IndexedTri tmpTri;
	WZMUV tmpUv;

	clear();

	m_textureArrays.push_back(TexArray());

	for (itF = faces.begin(); itF != faces.end(); ++itF)
	{
		for (i = 0; i < 3; ++i)
		{
			/* in the uv's, -1 is "not specified," but the OBJ indices
			 * are 0 based, hence < 1
			 */
			if (itF->uvs.operator [](i) < 1)
			{
				tmpUv.u() = 0;
				tmpUv.v() = 0;
			}
			else
			{
				tmpUv = uvArray[itF->uvs.operator [](i)-1];
			}
			inResult = pairSet.insert(make_mypair(verts[itF->tri[i]-1],
												  tmpUv,
												  vertLess,
												  uvLess));

			if (!inResult.second)
			{
				tmpTri[i] = mapping[std::distance(pairSet.begin(), inResult.first)];
			}
			else
			{
				itMap = mapping.begin();
				std::advance(itMap, std::distance(pairSet.begin(), inResult.first));
				mapping.insert(itMap, m_vertexArray.size());
				tmpTri[i] = m_vertexArray.size();
				m_vertexArray.push_back(verts[itF->tri[i]-1]);
				m_textureArrays[0].push_back(tmpUv);
			}
		}
		m_indexArray.push_back(tmpTri);
	}
	return true;
}

std::stringstream* Mesh::exportToOBJ(const Mesh_exportToOBJ_InOutParams& params) const
{
	const bool invertV = true;
	std::stringstream* out = new std::stringstream;

	std::pair<std::set<OBJVertex, OBJVertex::less_wEps>::iterator, bool> vertInResult;
	std::pair<std::set<OBJUV, OBJUV::less_wEps>::iterator, bool> uvInResult;

	std::vector<IndexedTri>::const_iterator itF;
	std::vector<unsigned>::iterator itMap;
	unsigned i;

	OBJUV uv;

	*out << "o " << m_name << "\n\n";

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

			uv = m_textureArrays[0][itF->operator [](i)];
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

		mesh->texelL[i][0] = m_textureArrays[0][i].u();
		if (invertV)
		{
			mesh->texelL[i][1] = 1.0f - m_textureArrays[0][i].v();
		}
		else
		{
			mesh->texelL[i][1] = m_textureArrays[0][i].v();
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

WZMConnector& Mesh::getConnector(int index)
{
	std::list<WZMConnector>::iterator pos;
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

int Mesh::textureArrays() const
{
	return m_textureArrays.size();
}

const TexArray& Mesh::getTexArray (int index) const
{
	return m_textureArrays.at(index);
}

#if 0
void Mesh::addTexArray (const TexArray& tex, int index)
{
	if(tex.size()!=indices())
	{
		return;
	}
	m_textureArrays.insert(m_textureArrays.begin() + index,tex);
}
#endif
#if 0
void Mesh::rmTexArray(int index)
{
	std::vector<TexArray>::iterator pos;
	pos=m_textureArrays.begin()+index;
	if(pos==m_textureArrays.end())
	{
		return;
	}
	m_textureArrays.erase(pos);
}
#endif

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
#pragma message "unfinished"
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
	m_name = std::string();
	m_teamColours = false;
}

void Mesh::clear()
{
	m_name = std::string();
	m_frameArray.clear();
	m_vertexArray.clear();
	m_textureArrays.clear();
	m_indexArray.clear();
	m_connectors.clear();
	m_teamColours = false;
}
