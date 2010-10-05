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
#ifndef POLYGON_HPP
#define POLYGON_HPP

#include <iostream>

#include <GL/gl.h>

#include "Vector.hpp"


struct IndexedTri : public Vector<GLushort,3>
{
	typedef GLushort indexType;
	indexType& a() {
		return component[0];
	}
	indexType a() const {
		return component[0];
	}

	indexType& b() {
		return component[1];
	}
	indexType b() const {
		return component[1];
	}

	indexType& c() {
		return component[2];
	}
	indexType c() const {
		return component[2];
	}
};

/*
  Pie polygons are triangle fans of with
  texture and texture animation data.
  */
template<typename U, typename S, size_t MAX>
class PiePolygon
{
	friend class Mesh;
public:
	PiePolygon();
	virtual ~PiePolygon(){}

	bool read(std::istream& in);
	void write(std::ostream& out) const;

	unsigned getFrames() const;
	unsigned getIndex(unsigned n) const;
	U getUV(int index) const;
	unsigned short vertices() const;
	unsigned short triangles() const;
protected:
	void clear();
	static const size_t MAX_VERTICES = MAX;
	unsigned short m_vertices;
	short m_indices[MAX];
	U m_texCoords[MAX];
	unsigned long m_flags;

	/// Texture Animation Data
	unsigned m_frames;
	unsigned m_playbackRate;
	S m_width, m_height;
};

// Include template implementations
#define POLYGON_T_CPP
#include "Polygon_t.cpp"
#undef POLYGON_T_CPP

#endif // POLYGON_HPP
