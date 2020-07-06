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

#include <cmath>
#include <algorithm>
#include <iterator>
#include <fstream>

#include "Pie.h"

int pieVersion(std::istream& in)
{
	std::string pie;
	unsigned version;
	std::streampos start = in.tellg();

	// PIE %u
	in >> pie >> version;
	if (in.good() && pie.compare(PIE_MODEL_SIGNATURE) == 0)
	{
		in.seekg(start);
		if (version >= 2 || version <= 3)
		{
			return version;
		}
	}
	return -1;
}

bool tryToReadDirective(std::istream &in, const char* directive, const bool isOptional, std::function<bool(std::istream&)> dirLoaderFunc)
{
	std::string str;
	std::streampos entrypoint = in.tellg();

	in >> str;
	if (in.fail() || (str.compare(directive) != 0))
	{
		// Not a failure if some other directive found and this is optional, just rewind
		if (isOptional)
		{
			in.clear();
			in.seekg(entrypoint);
		}
		return isOptional;
	}

	return dirLoaderFunc(in);
}


/**********************************************
  Pie version 2
  *********************************************/

Pie2Model::Pie2Model(): APieModel(PIE2_CAPS)
{
}

Pie2Model::~Pie2Model()
{
}

unsigned Pie2Model::version() const
{
	return 2;
}

unsigned Pie2Model::textureHeight() const
{
	return 256;
}

unsigned Pie2Model::textureWidth() const
{
	return 256;
}

/**********************************************
  Pie version 3
  *********************************************/

Pie3UV::Pie3UV()
{
}

Pie3UV::Pie3UV(GLclampf u, GLclampf v)
{
	this->u() = u; this->v() = v;
}

Pie3UV::Pie3UV(const Pie2UV& p2)
{
	u() = p2.u()/256.f;
	v() = p2.v()/256.f;
}

Pie3UV::operator Pie2UV() const
{
	Pie2UV p2;
	p2.u() = round(u() * 256.f);
	p2.v() = round(v() * 256.f);
	return p2;
}

Pie3Vertex::Pie3Vertex(const Vertex<GLfloat>& vert)
{
	x() = vert.x();
	y() = vert.y();
	z() = vert.z();
}

Pie3Vertex::Pie3Vertex(const Pie2Vertex& p2)
{
	x() = p2.x();
	y() = p2.y();
	z() = p2.z();
}

Pie3Vertex Pie3Vertex::upConvert(const Pie2Vertex& p2)
{
	return Pie3Vertex(p2);
}

Pie2Vertex Pie3Vertex::backConvert(const Pie3Vertex& p3)
{
	return p3;
}

Pie3Vertex::operator Pie2Vertex() const
{
	Pie2Vertex p2;
	p2.x() = round(x());
	p2.y() = round(y());
	p2.z() = round(z());
	return p2;
}

Pie3Connector::Pie3Connector(const Pie2Connector& p2)
{
	pos.x() = p2.pos.x();
	pos.y() = p2.pos.y();
	pos.z() = p2.pos.z();
}

Pie3Connector Pie3Connector::upConvert(const Pie2Connector& p2)
{
	return Pie3Connector(p2);
}

Pie2Connector Pie3Connector::backConvert(const Pie3Connector& p3)
{
	return p3;
}

Pie3Connector::operator Pie2Connector() const
{
	Pie2Connector p2;
	p2.pos.x() = pos.x();
	p2.pos.y() = pos.y();
	p2.pos.z() = pos.z();
	return p2;
}

Pie3Polygon::Pie3Polygon()
{
	m_vertices = 3;
}

Pie3Polygon::~Pie3Polygon()
{
}

int Pie3Polygon::upConvert(const Pie2Polygon& pie2Poly, std::back_insert_iterator<std::vector<Pie3Polygon> > result)
{
	unsigned i;

	for(i = 0; i < pie2Poly.triangles(); ++i, ++result)
	{
		Pie3Polygon p3;
		p3.m_indices[0] = pie2Poly.m_indices[0];
		p3.m_texCoords[0] = pie2Poly.m_texCoords[0];

		p3.m_indices[1] = pie2Poly.m_indices[i+1];
		p3.m_texCoords[1] = pie2Poly.m_texCoords[i+1];

		p3.m_indices[2] = pie2Poly.m_indices[i+2];
		p3.m_texCoords[2] = pie2Poly.m_texCoords[i+2];

		p3.m_vertices = 3;
		p3.m_flags = pie2Poly.m_flags; // FIXME: need to check whether these flags are supported

		p3.m_frames = pie2Poly.m_frames;
		p3.m_playbackRate = pie2Poly.m_playbackRate;
		p3.m_width = pie2Poly.m_width/256.f;
		p3.m_height = pie2Poly.m_height/256.f;
		(*result) = p3;
	}
	return pie2Poly.triangles();
}

Pie2Polygon Pie3Polygon::backConvert(const Pie3Polygon& p3)
{
	return p3;
}

Pie3Polygon::operator Pie2Polygon() const
{
	Pie2Polygon p2;
	int i;

	for (i = 2; i >= 0; --i)
	{
		p2.m_indices[i] = m_indices[i];
		p2.m_texCoords[i] = m_texCoords[i];
	}

	p2.m_vertices = 3;

	p2.m_flags = m_flags;

	p2.m_frames = m_frames;
	p2.m_playbackRate = m_playbackRate;
	p2.m_width = ceil(m_width * 256.f);
	p2.m_height = ceil(m_height * 256.f);

	return p2;
}

/*
 * This function would be code duplication if
 * I had implemented the Pie2 version, in which
 * case I might consider adding template parameters...
 * don't tempt me!
 */
Pie3UV Pie3Polygon::getUV(unsigned index, unsigned frame) const
{
	double u, v;
	double width, height;
	int framesPerLine = 1;
	int frameH, frameV;

	if (frame == 0)
	{
		return m_texCoords[index];
	}

	if (m_width != 0)
	{
		framesPerLine = 1 / m_width;
	}

	/* This works because wrap around is only permitted if you start the animation at the
	 * left border of the texture. What a horrible hack this was.
	 * Note: It is done the same way in the Warzone source. */
	frameH = frame % framesPerLine;
	frameV = frame / framesPerLine;	// note the integer divsion

	width = m_texCoords[index].u() + m_width * frameH;
	height = m_texCoords[index].v() + m_height * frameV;

	u = width;
	v = height;

	return Pie3UV(u, v);
}

Pie3Level::Pie3Level()
{
}

Pie3Level::Pie3Level(const Pie2Level& p2)
{
	std::vector<Pie2Polygon>::const_iterator it;

	std::transform(p2.m_points.begin(), p2.m_points.end(),
				   back_inserter(m_points), Pie3Vertex::upConvert);

	for (it = p2.m_polygons.begin(); it != p2.m_polygons.end(); ++it)
	{
		Pie3Polygon::upConvert(*it, back_inserter(m_polygons));
	}

	std::transform(p2.m_connectors.begin(), p2.m_connectors.end(),
				   back_inserter(m_connectors), Pie3Connector::upConvert);

	// HACK to accomodate "flexible" wz PIE loader
	m_animobj = p2.m_animobj;
}

Pie3Level::~Pie3Level()
{
}

Pie3Level Pie3Level::upConvert(const Pie2Level& p2)
{
	return Pie3Level(p2);
}

Pie2Level Pie3Level::backConvert(const Pie3Level& p3)
{
	return p3;
}

Pie3Level::operator Pie2Level() const
{
	Pie2Level p2;

	std::transform(m_points.begin(), m_points.end(),
				   back_inserter(p2.m_points), Pie3Vertex::backConvert);

	std::transform(m_polygons.begin(), m_polygons.end(),
				   back_inserter(p2.m_polygons), Pie3Polygon::backConvert);

	std::transform(m_connectors.begin(), m_connectors.end(),
				   back_inserter(p2.m_connectors), Pie3Connector::backConvert);
	return p2;
}

Pie3Model::Pie3Model(): APieModel(PIE3_CAPS)
{
}

Pie3Model::Pie3Model(const Pie2Model& p2): APieModel(PIE3_CAPS)
{
	m_texture = p2.m_texture;
	m_texture_tcmask = p2.m_texture_tcmask;
	std::transform(p2.m_levels.begin(), p2.m_levels.end(),
				   back_inserter(m_levels), Pie3Level::upConvert);

	// HACK to accomodate "flexible" wz PIE loader
	m_events = p2.m_events;

	m_read_type = p2.m_read_type;
	m_caps = p2.m_caps;
}

Pie3Model::~Pie3Model()
{
}

unsigned Pie3Model::version() const
{
	return 3;
}

Pie3Model::operator Pie2Model() const
{
	Pie2Model p2;
	p2.m_texture = m_texture;
	p2.m_texture_tcmask = m_texture_tcmask;
	std::transform(m_levels.begin(), m_levels.end(),
				   back_inserter(p2.m_levels), Pie3Level::backConvert);
	p2.m_read_type = m_read_type;
	p2.m_caps = m_caps;
	return p2;
}

unsigned Pie3Model::textureHeight() const
{
	return 0;
}

unsigned Pie3Model::textureWidth() const
{
	return 0;
}

bool ApieAnimFrame::read(std::istream &in)
{
	in >> num;
	if ( in.fail())
		return false;

	in >> pos >> rot >> scale;
	return !in.fail();
}

void ApieAnimFrame::write(std::ostream &out) const
{
	out << num  << ' ' << pos  << ' ' << rot  << ' ' << scale;
}

bool ApieAnimObject::read(std::istream &in)
{
	clear();

	// time cycles frames
	in >> time >> cycles >> numframes;

	if ( in.fail())
	{
		return false;
	}

	ApieAnimFrame curFrame;
	for (int i = 0; i < numframes; ++i)
	{
		if (!curFrame.read(in))
		{
			clear();
			return false;
		}
		frames.push_back(curFrame);
	}
	return true;
}

void ApieAnimObject::write(std::ostream &out) const
{
	out << ' ' << time << ' ' << cycles << ' ' << numframes;
	for (size_t i = 0; i < static_cast<size_t>(numframes); ++i)
	{
		out << "\n\t";
		frames[i].write(out);
	}
	out << '\n';
}

bool ApieAnimObject::readStandaloneAniFile(const char *file)
{
	std::ifstream fin;

	clear();

	fin.open(file, std::ios::in | std::ios::binary);

	if (!fin.is_open())
		return false;

	return readStandaloneAniStream(fin);
}

bool ApieAnimObject::readStandaloneAniStream(std::istream &fin)
{
	std::string str;
	std::streampos entrypoint = fin.tellg();

	clear();

	std::getline(fin, str);
	if (fin.fail())
	{
		return false;
	}

	// Cut off CR
	if (!str.empty() && str.back() == '\r')
		str = str.substr(0, str.size() - 1);

	// Attempt to read mesh name
	if (str.find(PIE_MODEL_DIRECTIVE_ANIMOBJECT) == 0)
	{
		str.clear();

		fin.clear();
		fin.seekg(entrypoint);
	}

	if (tryToReadDirective(fin, PIE_MODEL_DIRECTIVE_ANIMOBJECT, false,
		[this](std::istream& inn)
		{
			return read(inn);
		}))
	{
			name = str;
			// read up to next line
			std::getline(fin, str);
			return true;
	}
	return false;
}

const char *getPieDirectiveName(PIE_OPT_DIRECTIVES dir)
{
	switch (dir) {
	case PIE_OPT_DIRECTIVES::podNORMALMAP: return PIE_MODEL_DIRECTIVE_NORMALMAP;
	case PIE_OPT_DIRECTIVES::podSPECULARMAP: return PIE_MODEL_DIRECTIVE_SPECULARMAP;
	case PIE_OPT_DIRECTIVES::podEVENT: return PIE_MODEL_DIRECTIVE_EVENT;
	case PIE_OPT_DIRECTIVES::podMATERIALS: return PIE_MODEL_DIRECTIVE_MATERIALS;
	case PIE_OPT_DIRECTIVES::podSHADERS: return PIE_MODEL_DIRECTIVE_SHADERS;
	case PIE_OPT_DIRECTIVES::podNORMALS: return PIE_MODEL_DIRECTIVE_NORMALS;
	case PIE_OPT_DIRECTIVES::podCONNECTORS: return PIE_MODEL_DIRECTIVE_CONNECTORS;
	case PIE_OPT_DIRECTIVES::podANIMOBJECT: return PIE_MODEL_DIRECTIVE_ANIMOBJECT;
	default:
		return "";
	}
}

const char *getPieDirectiveDescription(PIE_OPT_DIRECTIVES dir)
{
	switch (dir) {
	case PIE_OPT_DIRECTIVES::podNORMALMAP: return "Sets the normal map texture page for the model.";
	case PIE_OPT_DIRECTIVES::podSPECULARMAP: return "Sets the specular map texture page for the model.";
	case PIE_OPT_DIRECTIVES::podEVENT: return "An animation event associated with this model. If the event type is triggered, the model is replaced with the specified model for the duration of the event.";
	case PIE_OPT_DIRECTIVES::podMATERIALS: return "Specifies the material properties of a mesh.";
	case PIE_OPT_DIRECTIVES::podSHADERS: return "Create a specific shader program for this mesh.";
	case PIE_OPT_DIRECTIVES::podNORMALS: return "This allows presence of per-vertex normals in a mesh.";
	case PIE_OPT_DIRECTIVES::podCONNECTORS: return "Connectors are used to place and orient other components against each other.";
	case PIE_OPT_DIRECTIVES::podANIMOBJECT: return "If the mesh is animated, this directive will tell the game how to animate it.";
	default:
		return "";
	}
}

bool ApieAnimList::readAniFile(const char *file)
{
	std::ifstream fin;

	clear();

	fin.open(file, std::ios::in | std::ios::binary);

	if (!fin.is_open())
		return false;

	ApieAnimObject nextAnim;
	while (nextAnim.readStandaloneAniStream(fin))
	{
		anims.emplace_back(nextAnim);
	};
	return count() > 0;
}
