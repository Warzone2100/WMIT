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
#ifdef POLYGON_T_CPP

#include <algorithm>


#include "Polygon.hpp" // Hack for autocomplete

/*
  *
  * Template implementations
  *
  *
*/

template<typename U, typename S, size_t MAX>
PiePolygon<U, S, MAX>::PiePolygon():
	m_vertices(0),
	m_flags(0),
	m_frames(0),
	m_playbackRate(0),
	m_width(0), m_height(0)
{
}

template<typename U, typename S, size_t MAX>
bool PiePolygon<U, S, MAX>::read(std::istream& in)
{
	unsigned i;
	clear();

	in >> std::hex >> m_flags >> std::dec >> m_vertices;
	if (in.fail() || m_vertices > MAX)
	{
		clear();
		return false;
	}

	for (i = 0; i < m_vertices; ++i)
	{
		in >> m_indices[i];
	}

	if (m_flags & 0x4000)
	{
		in >> m_frames >> m_playbackRate >>	m_width >> m_height;
	}
	else
	{
		m_frames = 1;
	}

	for (i = 0; i < m_vertices; ++i)
	{
		in >> m_texCoords[i].u() >> m_texCoords[i].v();
	}
	if (in.fail() && !in.eof())
	{
		clear();
		return false;
	}
	return true;
}

template<typename U, typename S, size_t MAX>
void PiePolygon<U, S, MAX>::write(std::ostream& out) const
{
	unsigned i;

	out << std::hex << m_flags << std::dec << '\t';
	out << m_vertices << '\t';

	for (i = 0; i < m_vertices; ++i)
	{
		out << m_indices[i] << '\t';
	}

	if (m_flags & 0x4000)
	{
		out << m_frames << '\t'
				<< m_playbackRate << '\t'
				<< m_width	<< '\t'
				<< m_height	<< '\t';
	}

	for (i = 0; i < m_vertices; ++i)
	{
		out << m_texCoords[i].u() << '\t'
				<< m_texCoords[i].v() << "\t\t";
	}
	out << '\n';
}

template<typename U, typename S, size_t MAX>
unsigned PiePolygon<U, S, MAX>::getFrames() const
{
	return m_frames;
}

template<typename U, typename S, size_t MAX>
unsigned PiePolygon<U, S, MAX>::getIndex(unsigned n) const
{
	//TODO: assert n >= vertices()
	return m_indices[n];
}

template<typename U, typename S, size_t MAX>
U PiePolygon<U, S, MAX>::getUV(int index) const
{
	return m_texCoords[index];
}

template<typename U, typename S, size_t MAX>
void PiePolygon<U, S, MAX>::clear()
{
	*this = PiePolygon();
}

template<typename U, typename S, size_t MAX>
unsigned short PiePolygon<U, S, MAX>::vertices() const
{
	return m_vertices;
}

template<typename U, typename S, size_t MAX>
unsigned short PiePolygon<U, S, MAX>::triangles() const
{
	return std::max(0,vertices() - 2);
}

#endif //POLYGON_T_CPP
