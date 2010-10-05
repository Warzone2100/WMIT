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

#include "Pie.hpp"

int pieVersion(std::istream& in)
{
	std::string pie;
	unsigned version;
	std::streampos start = in.tellg();

	// PIE %u
	in >> pie >> version;
	if (in.good() && pie.compare("PIE") == 0)
	{
		in.seekg(start);
		if (version >= 2 || version <= 3)
		{
			return version;
		}
	}
	return -1;
}

Pie2Model::Pie2Model()
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
#pragma message "FIXME"
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

Pie3Model::Pie3Model()
{
}

Pie3Model::Pie3Model(const Pie2Model& p2)
{
	m_texture = p2.m_texture;
	std::transform(p2.m_levels.begin(), p2.m_levels.end(),
				   back_inserter(m_levels), Pie3Level::upConvert);
	m_type = p2.m_type; // FIXME: need to check whether these flags are supported
#pragma message "FIXME"
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
	std::transform(m_levels.begin(), m_levels.end(),
				   back_inserter(p2.m_levels), Pie3Level::backConvert);
	p2.m_type = m_type; // FIXME: need to check whether these flags are supported
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
