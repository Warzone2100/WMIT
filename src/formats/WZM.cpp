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

#include "WZM.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <set>
#include <list>

#include <cmath>

#include <sstream>

#include "Generic.h"
#include "Util.h"
#include "Pie.h"
#include "Vector.h"

#include "OBJ.h"

void WZMaterial::setDefaults()
{
	shininess = 10.f;

	for (int i = WZM_MAT_EMISSIVE + 1; i < WZM_MAT__LAST; ++i)
	{
		wzm_material_t mattype = static_cast<wzm_material_t>(i);
		vals[mattype] = WZMVertex4(1.f);
	}
}

bool WZMaterial::isDefault() const
{
	if (shininess != 10.f)
		return false;
	for (int i = WZM_MAT_EMISSIVE + 1; i < WZM_MAT__LAST; ++i)
	{
		wzm_material_t mattype = static_cast<wzm_material_t>(i);
		if (!vals[mattype].sameComponents(1.f))
			return false;
	}
	if (!vals[WZM_MAT_EMISSIVE].sameComponents(0.f))
		return false;
	return true;
}

std::istream& operator>> (std::istream& in, WZMaterial& mat)
{
	in >> mat.vals[WZM_MAT_EMISSIVE].x() >> mat.vals[WZM_MAT_EMISSIVE].y() >> mat.vals[WZM_MAT_EMISSIVE].z()
	   >> mat.vals[WZM_MAT_AMBIENT].x()  >> mat.vals[WZM_MAT_AMBIENT].y()  >> mat.vals[WZM_MAT_AMBIENT].z()
	   >> mat.vals[WZM_MAT_DIFFUSE].x()  >> mat.vals[WZM_MAT_DIFFUSE].y()  >> mat.vals[WZM_MAT_DIFFUSE].z()
	   >> mat.vals[WZM_MAT_SPECULAR].x() >> mat.vals[WZM_MAT_SPECULAR].y() >> mat.vals[WZM_MAT_SPECULAR].z();
	in >> mat.shininess;
	return in;
}

std::ostream& operator<< (std::ostream& out, const WZMaterial& mat)
{
	out << mat.vals[WZM_MAT_EMISSIVE].x() << ' ' << mat.vals[WZM_MAT_EMISSIVE].y() << ' ' << mat.vals[WZM_MAT_EMISSIVE].z() << ' '
	    << mat.vals[WZM_MAT_AMBIENT].x()  << ' ' << mat.vals[WZM_MAT_AMBIENT].y()  << ' ' << mat.vals[WZM_MAT_AMBIENT].z()  << ' '
	    << mat.vals[WZM_MAT_DIFFUSE].x()  << ' ' << mat.vals[WZM_MAT_DIFFUSE].y()  << ' ' << mat.vals[WZM_MAT_DIFFUSE].z()  << ' '
	    << mat.vals[WZM_MAT_SPECULAR].x() << ' ' << mat.vals[WZM_MAT_SPECULAR].y() << ' ' << mat.vals[WZM_MAT_SPECULAR].z() << ' ';
	out << mat.shininess;
	return out;
}

WZM::WZM()
{
	m_material.setDefaults();
}

WZM::WZM(const Pie3Model &p3)
{
	std::vector<Pie3Level>::const_iterator it;
	std::stringstream ss;

	m_material.setDefaults();

	setTextureName(WZM_TEX_DIFFUSE, p3.m_texture);
	setTextureName(WZM_TEX_NORMALMAP, p3.m_texture_normalmap);
	setTextureName(WZM_TEX_TCMASK, p3.m_texture_tcmask);

	for (it = p3.m_levels.begin(); it != p3.m_levels.end(); ++it)
	{
		m_meshes.push_back(*it);

		// name
		ss << m_meshes.size();
		m_meshes.back().setName(ss.str());
		ss.str(std::string());

		// per-mesh team colors
		m_meshes.back().setTeamColours(isTextureSet(WZM_TEX_TCMASK));
	}
}

WZM::operator Pie3Model() const
{
	Pie3Model p3;

	p3.m_texture = getTextureName(WZM_TEX_DIFFUSE);
	p3.m_texture_normalmap = getTextureName(WZM_TEX_NORMALMAP);
	p3.m_texture_tcmask = getTextureName(WZM_TEX_TCMASK);

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
	if (in.fail() || str.compare(WZM_MODEL_SIGNATURE) != 0)
	{
		std::cerr << "WZM::read - Missing header";
		return false;
	}

	in >> i;
	if (in.fail())
	{
		std::cerr << "WZM::read - Error reading WZM version";
		return false;
	}
	else if(i != WZM_MODEL_VERSION_FD)
	{
		std::cerr << "WZM::read - Unsupported WZM version " << i;
		return false;
	}

	// TEXTURE %s
	in >> str;
	if (str.compare(WZM_MODEL_DIRECTIVE_TEXTURE) != 0)
	{
		std::cerr << "WZM::read - Expected " << WZM_MODEL_DIRECTIVE_TEXTURE << " directive but got" << str;
		return false;
	}
	in >> m_textures[WZM_TEX_DIFFUSE];
	if (in.fail())
	{
		std::cerr << "WZM::read - Error reading texture name";
		return false;
	}

	// read next token
	in >> str;

	// optional: team color mask
	if (!str.compare(WZM_MODEL_DIRECTIVE_TCMASK))
	{
		in >> m_textures[WZM_TEX_TCMASK];
		if (in.fail())
		{
			std::cerr << "WZM::read - Error reading TCMask name";
			return false;
		}

		// pre read next token
		in >> str;
	}

	// optional: normalmap
	if (!str.compare(WZM_MODEL_DIRECTIVE_NORMALMAP))
	{
		in >> m_textures[WZM_TEX_NORMALMAP];
		if (in.fail())
		{
			std::cerr << "WZM::read - Error reading NORMALMAP name";
			return false;
		}

		// pre read next token
		in >> str;
	}

	// optional: specularmap
	if (!str.compare(WZM_MODEL_DIRECTIVE_SPECULARMAP))
	{
		in >> m_textures[WZM_TEX_SPECULAR];
		if (in.fail())
		{
			std::cerr << "WZM::read - Error reading SPECULARMAP name";
			return false;
		}

		// pre read next token
		in >> str;
	}

	// optional: material
	if (!str.compare(WZM_MODEL_DIRECTIVE_MATERIAL))
	{
		in >> m_material;
		if (in.fail())
		{
			std::cerr << "WZM::read - Error reading material values";
			return false;
		}

		// pre read next token
		in >> str;
	}

	// token was pre read here
	// MESHES %u
	in >> meshes;
	if (in.fail() || str.compare("MESHES") != 0)
	{
		std::cerr << "WZM::read - Expected MESHES directive but got " << str;
		return false;
	}

	m_meshes.resize(meshes);
	for(int i = 0; i < meshes; ++i)
	{
		Mesh& mesh = m_meshes[i];
		if (!mesh.read(in))
		{
			std::cerr << "WZM::read - Error reading mesh " << meshes + 1;
			return false;
		}
	}
	return true;
}

void WZM::write(std::ostream& out) const
{
	std::vector<Mesh>::const_iterator it;

	out << "WZM " << version() << '\n';

	// TEXTURE
	if (isTextureSet(WZM_TEX_DIFFUSE))
	{
		out << WZM_MODEL_DIRECTIVE_TEXTURE << ' ' << getTextureName(WZM_TEX_DIFFUSE) << '\n';
	}
	else
	{
		// do it in a fail-safe way
		out << WZM_MODEL_DIRECTIVE_TEXTURE << " notexture.set\n";
	}

	// optional: team color mask
	if (isTextureSet(WZM_TEX_TCMASK))
	{
		out << WZM_MODEL_DIRECTIVE_TCMASK << ' ' << getTextureName(WZM_TEX_TCMASK) << '\n';
	}

	// optional: normalmap
	if (isTextureSet(WZM_TEX_NORMALMAP))
	{
		out << WZM_MODEL_DIRECTIVE_NORMALMAP << ' ' << getTextureName(WZM_TEX_NORMALMAP) << '\n';
	}

	// optional: specularmap
	if (isTextureSet(WZM_TEX_SPECULAR))
	{
		out << WZM_MODEL_DIRECTIVE_SPECULARMAP << ' ' << getTextureName(WZM_TEX_SPECULAR) << '\n';
	}

	// optional: material
	//if (!m_material.isDefault()) //FIXME: disable till material GUI is ready
	{
		out << WZM_MODEL_DIRECTIVE_MATERIAL << ' ' << m_material << '\n';
	}

	// MESHES
	out << WZM_MODEL_DIRECTIVE_MESHES << " " << meshes() << '\n';
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
	std::vector<OBJVertex> vertArray, normArray;
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

		if (str.empty() || ss.fail())
		{
			continue;
		}

		switch(ss.get())
		{
		case '#':
			// ignore comments
			continue;
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
			case 'n':	// normals
				ss >> vert.x() >> vert.y() >> vert.z();
				if (ss.fail())
				{
					return false;
				}
				normArray.push_back(vert);
				break;
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
					tri.nrm.operator [](pos) = -1;
				}

				if (indices.size() == 3 && !indices[2].empty())
				{
					ss.str(indices[2]), ss.clear(), ss.seekg(0);
					ss >> tri.nrm.operator [](pos);
				}
				else
				{
					tri.nrm.operator [](pos) = -1;
				}

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
				m_meshes.push_back(Mesh());
				Mesh& mesh = m_meshes.back();
				mesh.importFromOBJ(groupedFaces, vertArray, uvArray, normArray);
				mesh.setTeamColours(false);
				mesh.setName(name);
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
		m_meshes.push_back(Mesh());
		Mesh& mesh = m_meshes.back();
		mesh.importFromOBJ(groupedFaces, vertArray, uvArray, normArray);
		mesh.setTeamColours(false);
		mesh.setName(name);
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

	OBJVertex::less_wEps normCompare;
	std::set<OBJVertex, OBJVertex::less_wEps> normSet(normCompare);
	std::vector<OBJVertex> normals;
	std::vector<unsigned> normMapping;

	params.normals = &normals;
	params.normSet = &normSet;
	params.normMapping = &normMapping;

	std::vector<Mesh>::const_iterator itM;
	std::vector<OBJVertex>::iterator itVert;
	std::vector<OBJUV>::iterator	itUV;
	std::vector<OBJVertex>::iterator itNorm;
	std::stringstream* pSSS;

	if (!getTextureName(WZM_TEX_DIFFUSE).empty())
	{
		out << "mtllib " << getTextureName(WZM_TEX_DIFFUSE) << ".mtl\nusemtl " << getTextureName(WZM_TEX_DIFFUSE) << "\n\n";
	}

	for (itM = m_meshes.begin(); itM != m_meshes.end(); ++itM)
	{
		objectBuffers.push_back(itM->exportToOBJ(params));
	}

	out << "# " << vertices.size() << " vertices\n";
	for (itVert = vertices.begin(); itVert != vertices.end(); ++itVert)
	{
		writeOBJVertex(*itVert, out);
	}

	out << '\n';

	out << "# " << uvs.size() << " texture coords\n";
	for (itUV = uvs.begin(); itUV != uvs.end(); ++itUV)
	{
		writeOBJUV(*itUV, out);
	}

	out << '\n';

	out << "# " << normals.size() << " vertex normals\n";
	for (itNorm = normals.begin(); itNorm != normals.end(); ++itNorm)
	{
		writeOBJNormal(*itNorm, out);
	}

	while (!objectBuffers.empty())
	{
		pSSS = objectBuffers.front();
		objectBuffers.pop_front();

		out << "\n" << pSSS->str();

		delete pSSS;
	}
}

int WZM::version() const
{
	return WZM_MODEL_VERSION_FD;
}

int WZM::meshes() const
{
	return m_meshes.size();
}

void WZM::setTextureName(wzm_texture_type_t type, std::string name)
{
	m_textures[type] = name;
}

std::string WZM::getTextureName(wzm_texture_type_t type) const
{
	std::map<wzm_texture_type_t, std::string>::const_iterator it;

	it = m_textures.find(type);
	if (it != m_textures.end())
		return it->second;

	return std::string();
}

bool WZM::isTextureSet(wzm_texture_type_t type) const
{
	std::map<wzm_texture_type_t, std::string>::const_iterator it;

	it = m_textures.find(type);
	if (it == m_textures.end())
		return false;

	return !(it->second.empty());
}

void WZM::clearTextureNames()
{
	m_textures.clear();
}

std::string WZM::texTypeToString(wzm_texture_type_t type)
{
	std::string str;

	switch (type)
	{
	case WZM_TEX_DIFFUSE:
		str = "Diffuse";
		break;
	case WZM_TEX_TCMASK:
		str = "TCMask";
		break;
	case WZM_TEX_NORMALMAP:
		str = "Normal map";
		break;
	case WZM_TEX_SPECULAR:
		str = "Specular map";
		break;
	default:
		str = "Unknown!";
	}

	return str;
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

	pos = m_meshes.begin() + index;
	if (pos == m_meshes.end())
	{
		return;
	}

	m_meshes.erase(pos);
}

bool WZM::isValid() const
{
	std::vector<Mesh>::const_iterator it;

	if (!isValidWzName(getTextureName(WZM_TEX_DIFFUSE)))
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
	m_textures.clear();
	m_material.setDefaults();
}

void WZM::scale(GLfloat x, GLfloat y, GLfloat z, int mesh)
{
	// All or a single mesh
	if (mesh < 0)
	{
		std::vector<Mesh>::iterator it;
		for (it = m_meshes.begin(); it != m_meshes.end(); ++it)
		{
			it->scale(x, y, z);
		}
	}
	else
	{
		if (m_meshes.size() > (std::vector<Mesh>::size_type)mesh)
		{
			m_meshes[(std::vector<Mesh>::size_type)mesh].scale(x, y, z);
		}
	}
}

void WZM::mirror(int axis, int mesh)
{
	if (axis < 0 || axis > 5 || mesh >= (int)m_meshes.size())
		return;

	// All or a single mesh
	if (mesh < 0)
	{
		WZMVertex center = calculateCenterPoint();

		std::vector<Mesh>::iterator it;
		for (it = m_meshes.begin(); it != m_meshes.end(); ++it)
		{
			switch (axis)
			{
			case 3:
			case 4:
			case 5:
				it->mirrorFromPoint(WZMVertex(), axis - 3);
				break;
			default:
				it->mirrorFromPoint(center, axis);
			}

			// for convenience
			it->reverseWinding();
		}
	}
	else
	{
		Mesh& meshobj = m_meshes[(std::vector<Mesh>::size_type)mesh];
		switch (axis)
		{
		case 3:
		case 4:
		case 5:
			meshobj.mirrorFromPoint(WZMVertex(), axis - 3);
			break;
		default:
			meshobj.mirrorUsingLocalCenter(axis);
		}

		// for convenience
		meshobj.reverseWinding();
	}
}

void WZM::reverseWinding(int mesh)
{
	// All or a single mesh
	if (mesh < 0)
	{
		std::vector<Mesh>::iterator it;
		for (it = m_meshes.begin(); it != m_meshes.end(); ++it)
		{
			it->reverseWinding();
		}
	}
	else
	{
		if (m_meshes.size() > (std::vector<Mesh>::size_type)mesh)
		{
			m_meshes[(std::vector<Mesh>::size_type)mesh].reverseWinding();
		}
	}

}

WZMVertex WZM::calculateCenterPoint() const
{
	WZMVertex center, meshcenter;

	if (!m_meshes.size())
		return center;

	std::vector<Mesh>::const_iterator it;
	for (it = m_meshes.begin(); it != m_meshes.end(); ++it)
	{
		meshcenter = it->getCenterPoint();
		center.x() += meshcenter.x();
		center.y() += meshcenter.y();
		center.z() += meshcenter.z();
	}

	center.x() /= m_meshes.size();
	center.y() /= m_meshes.size();
	center.z() /= m_meshes.size();

	return center;
}
