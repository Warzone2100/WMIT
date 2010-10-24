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

#include "WZM.hpp"

#include <algorithm>
#include <iterator>
#include <map>
#include <set>
#include <list>

#include <cmath>

#include <sstream>

#include <lib3ds/file.h>
#include <lib3ds/mesh.h>
#include <lib3ds/material.h>

#include "Generic.hpp"
#include "Util.hpp"
#include "Pie.hpp"
#include "Vector.hpp"

#include "OBJ.hpp"

WZM::WZM()
{
}

WZM::WZM(const Pie3Model &p3)
{
	std::vector<Pie3Level>::const_iterator it;
	std::stringstream ss;
	m_texName = p3.m_texture;
	for (it = p3.m_levels.begin(); it != p3.m_levels.end(); ++it)
	{
		m_meshes.push_back(*it);
		ss << m_meshes.size();
		m_meshes.back().setName(ss.str());
		ss.str(std::string());
	}
}

WZM::operator Pie3Model() const
{
	Pie3Model p3;
	p3.m_type = 0x10200;
	p3.m_texture = m_texName;
	std::transform(m_meshes.begin(), m_meshes.end(),
				   back_inserter(p3.m_levels), Mesh::backConvert);
	return p3;
}

bool WZM::read(std::istream& in)
{
	std::string str;
	int i,meshes;

	clear();
	in >> str;
	if (in.fail() || str.compare("WZM") != 0)
	{
		std::cerr << "WZM::read - Missing WZM header";
		return false;
	}

	in >> i;
	if (in.fail())
	{
		std::cerr << "WZM::read - Error reading WZM version";
		return false;
	}
	else if(i!=2)
	{
		std::cerr << "WZM::read - Unsupported WZM version " << i;
		return false;
	}

	// TEXTURE %s
	in >> str;
	if (str.compare("TEXTURE") != 0)
	{
		std::cerr << "WZM::read - Expected TEXTURE directive but got" << str;
		return false;
	}
	in >> m_texName;
	if (in.fail())
	{
		std::cerr << "WZM::read - Error reading WZM version";
		m_texName = std::string();
		return false;
	}

	// MESHES %u
	in >> str >> meshes;
	if (in.fail() || str.compare("MESHES") != 0)
	{
		std::cerr << "WZM::read - Expected MESHES directive but got " << str;
		clear();
		return false;
	}

	m_meshes.reserve(meshes);
	for(; meshes>0; --meshes)
	{
		Mesh mesh;
		if (!mesh.read(in))
		{
			clear();
			return false;
		}
		m_meshes.push_back(mesh);
	}
	return true;
}

void WZM::write(std::ostream& out) const
{
	std::vector<Mesh>::const_iterator it;
	out << "WZM\t" << version() << '\n';
	if (!m_texName.empty())
	{
		out << "TEXTURE " << m_texName << '\n';
	}
	else
	{
		out << "TEXTURE emptytex.png\n";
	}
	out << "MESHES " << meshes() << '\n';
	for (it = m_meshes.begin(); it != m_meshes.end(); ++it)
	{
		it->write(out);
	}
}

/*
 * This function does the parsing,
 * we'll let class Mesh do the WZM'izing
 */
bool WZM::importFromOBJ(std::istream& in)
{
	const bool invertV = true;
	std::vector<OBJVertex> vertArray;

	std::vector<OBJUV> uvArray;

	std::vector<OBJTri> groupedFaces;

	std::string name("Default"); //Default name of default obj group is default
	std::vector<std::string> vertices, indices;
	std::vector<std::string>::iterator itVStr;
	std::string str;
	std::stringstream ss;

	// Only give warnings once
	bool warnLine = false, warnPoint = false;

	// More temporaries
	OBJTri tri;
	OBJVertex vert;
	OBJUV uv;
	Mesh mesh;

	unsigned i, pos;

	clear();

	/*	Build "global" vertex and
	 *	texture coordinate arrays.
	 *	Also figure out the mesh/group boundaries
	 */

	/* Note: This program tolerates imperfect .obj files
	 * because it accepts any whitespace as a space.
	 */

	while (!(in.eof()|| in.fail()))
	{
		std::getline(in, str);
		ss.str(str);
		ss.seekg(0);
		ss.clear();

		// We're forgiving of leading whitespaces (though we don't need/shouldn't to be)
		skipWhitespace(ss);
		if (str.empty() || ss.fail())
		{
			continue;
		}

		switch(ss.get())
		{
		case 'v':
			switch(ss.get())
			{
			case 't':
				ss >> uv.u() >> uv.v();
				if (ss.fail())
				{
					return false;
				}
				else if (invertV)
				{
					uv.v() = 1 - uv.v();
				}
				uvArray.push_back(uv);
				break;
			case 'n':	// Ignore normals
			case 'p':	// and parameter vertices
				break;
			default:
				ss >> vert.x() >> vert.y() >> vert.z();
				if (ss.fail())
				{
					return false;
				}
				vertArray.push_back(vert);
			}
			break;
		case 'f':
			vertices = split(str);
			if (vertices.size() < 2 + 1)
			{
				 /* Should never happen, faces will have >= 3+1 tokens
				  * ( more than 3 vertices + the "f" directive)
				  */
				break;
			}

			itVStr = vertices.begin();
			++itVStr; // Advance to first vertex

			for (i = 0; itVStr != vertices.end(); ++itVStr, ++i)
			{
				if (i <= 2)
				{
					pos = i;
				}
				else
				{
					tri.uvs.operator [](1) = tri.uvs.operator [](2);
					tri.tri.operator [](1) = tri.tri.operator [](2);
					pos = 2;
				}

				indices = split(*itVStr, '/');

				ss.str(indices[0]), ss.clear(), ss.seekg(0);

				ss >> tri.tri.operator [](pos);

				if (indices.size() >= 2 && !indices[1].empty())
				{
					ss.str(indices[1]), ss.clear(), ss.seekg(0);
					ss >> tri.uvs.operator [](pos);
				}
				else
				{
					tri.uvs.operator [](pos) = -1;
				}

				if (indices.size() == 3 && !indices[2].empty())
				{} // ignoring normals

				if (i >= 2)
				{
					groupedFaces.push_back(tri);
				}
			}
			break;
		case 'l':
			// Ignore lines and give one warning to the user
			if (!warnLine)
			{
				std::cout << "WZM::importFromOBJ - Warning! Lines are not supported and will be ignored!";
				warnLine = true;
			}
			break;
		case 'p':
			// Ignore points and give one warning to the user
			if (!warnPoint)
			{
				std::cout << "Model::importFromOBJ - Warning! Points are not supported and will be ignored!";
				warnPoint = true;
			}
			break;
		case 'o':
			if (!groupedFaces.empty())
			{
				mesh.importFromOBJ(groupedFaces,vertArray,uvArray);
				mesh.setTeamColours(false);
				mesh.setName(name);
				m_meshes.push_back(mesh);
				groupedFaces.clear();
			}
			ss >> name;
			if (!isValidWzName(name))
			{
				ss.str(std::string()), ss.clear(), ss.seekg(0);
				ss << m_meshes.size();
				ss >> name;
			}
			break;
		}
	}
	if (!groupedFaces.empty())
	{
		mesh.importFromOBJ(groupedFaces,vertArray,uvArray);
		mesh.setTeamColours(false);
		mesh.setName(name);
		m_meshes.push_back(mesh);
	}
	return true;
}

void WZM::exportToOBJ(std::ostream &out) const
{
	std::list<std::stringstream*> objectBuffers;

	Mesh_exportToOBJ_InOutParams params;

	OBJVertex::less_wEps vertCompare;
	std::set<OBJVertex, OBJVertex::less_wEps> vertSet(vertCompare);
	std::vector<OBJVertex> vertices;
	std::vector<unsigned> vertMapping;

	params.vertices = &vertices;
	params.vertSet = &vertSet;
	params.vertMapping = &vertMapping;

	OBJUV::less_wEps uvCompare;
	std::set<OBJUV, OBJUV::less_wEps> uvSet(uvCompare);
	std::vector<OBJUV> uvs;
	std::vector<unsigned> uvMapping;

	params.uvs = &uvs;
	params.uvSet = &uvSet;
	params.uvMapping = &uvMapping;

	std::vector<Mesh>::const_iterator itM;
	std::vector<OBJVertex>::iterator itVert;
	std::vector<OBJUV>::iterator	itUV;
	std::stringstream* pSSS;

	if (!m_texName.empty())
	{
		out << "# texture: " << m_texName << '\n';
	}

	for (itM = m_meshes.begin(); itM != m_meshes.end(); ++itM)
	{
		objectBuffers.push_back(itM->exportToOBJ(params));
	}

	for (itVert = vertices.begin(); itVert != vertices.end(); ++itVert)
	{
		writeOBJVertex(*itVert, out);
	}

	out << '\n';

	for (itUV = uvs.begin(); itUV != uvs.end(); ++itUV)
	{
		writeOBJUV(*itUV, out);
	}

	while (!objectBuffers.empty())
	{
		pSSS = objectBuffers.front();
		objectBuffers.pop_front();

		out << "\n\n" << pSSS->str();

		delete pSSS;
	}
}

bool WZM::importFrom3DS(std::string fileName)
{
	Lib3dsFile *f = lib3ds_file_load(fileName.c_str());
	Lib3dsMaterial *material;
	Lib3dsMesh *ps3dsMesh;

	if (f == NULL)
	{
		std::cerr << "Loading 3DS file failed.\n";
		return false;
	}

	clear();

	material = f->materials;

	// Grab texture name
	if (material != NULL)
	{
		m_texName = material->texture1_map.name;
		if (material->next != NULL)
		{
			std::cout << "WZM::importFrom3ds - Multiple textures not supported!\n";
		}
	}

	for (ps3dsMesh = f->meshes; ps3dsMesh != NULL; ps3dsMesh = ps3dsMesh->next)
	{
		m_meshes.push_back(*ps3dsMesh);
	}

	lib3ds_file_free(f);
	return true;
}


bool WZM::exportTo3DS(std::string fileName) const
{
	Lib3dsFile* f = lib3ds_file_new();
	Lib3dsBool retVal;
	Lib3dsMaterial* material = lib3ds_material_new();

	std::vector<Mesh>::const_iterator itM;
	unsigned i;

	i = m_texName.copy(material->texture1_map.name, 64 - 1);
	material->texture1_map.name[i] = '\0';
	lib3ds_file_insert_material(f, material);

	for (itM = m_meshes.begin(); itM != m_meshes.end(); ++itM)
	{
		/* FIXME 3DS meshes have unique names,
		 * and overwrite meshes with same names when inserting
		 */
		lib3ds_file_insert_mesh(f, *itM);
	}

	retVal = lib3ds_file_save (f, fileName.c_str());
	delete f;
	return retVal;
}

int WZM::version() const
{
	return 2;
}

int WZM::meshes() const
{
	return m_meshes.size();
}

void WZM::setTextureName(std::string name)
{
	m_texName = name;
}

std::string WZM::getTextureName() const
{
	return m_texName;
}

Mesh& WZM::getMesh(int index)
{
	return m_meshes.at(index);
}

void WZM::addMesh(const Mesh& mesh)
{
	m_meshes.push_back(mesh);
}

void WZM::rmMesh (int index)
{
	std::vector<Mesh>::iterator pos;
	pos=m_meshes.begin()+index;
	if(pos==m_meshes.end())
	{
		return;
	}
	m_meshes.erase(pos);
}

bool WZM::couldHaveTCArrays() const
{
	std::vector<Mesh>::const_iterator it;

	for (it = m_meshes.begin(); it != m_meshes.end(); ++it)
	{
		if (it->textureArrays() == 8)
		{
			return true;
		}
	}
	return false;
}

bool WZM::isValid() const
{
	std::vector<Mesh>::const_iterator it;

	if (!isValidWzName(m_texName))
	{
		return false;
	}

	for (it = m_meshes.begin(); it != m_meshes.end(); ++it)
	{
		if(!it->isValid())
		{
			return false;
		}
	}
	return true;
}

void WZM::clear()
{
	m_meshes.clear();
	m_texName = std::string();
}
