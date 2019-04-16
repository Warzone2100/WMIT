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
#pragma once

#include "Generic.h"
#include "Util.h"

#include "Pie.h" // Hack for autocomplete

/*
  *
  * Template implementations
  *
  *
*/

template<typename V, typename P, typename C>
APieLevel< V, P, C>::APieLevel(): m_material(true)
{
}

// TODO: Write error messages to std::cerr
template<typename V, typename P, typename C>
bool APieLevel< V, P, C>::read(std::istream& in)
{
	std::string str;
	unsigned uint;

	std::streampos cnctrStrt;

	clearAll();

	#define streamfail() do { clearAll();return false; } while(0)

	// LEVEL %u
	in >> str >> uint;
	if ( in.fail() || str.compare("LEVEL") != 0)
	{
		streamfail();
	}

	// POINTS %u || MATERIALS %u
	in >> str;
	if ( str.compare(PIE_MODEL_DIRECTIVE_MATERIALS) == 0)
	{
		in >> m_material;
		if (in.fail())
			streamfail();
		in >> str;
	}

	// Optional: shaders
	if (str.compare(PIE_MODEL_DIRECTIVE_SHADERS) == 0)
	{
		in >> str >> m_shader_vert >> m_shader_frag;
		if (in.fail())
			streamfail();
		in >> str;
	}

	if (in.fail() || str.compare("POINTS") != 0)
	{
		streamfail();
	}
	else
	{
		in >> uint;
	}

	m_points.reserve(uint);
	for (; uint > 0; --uint)
	{
		V point;
		in >> point.x() >> point.y() >> point.z();
		point.scale(-1.0, 1.0, 1.0);
		m_points.push_back(point);
	}

	if (in.fail())
	{
		streamfail();
	}

	// POLYGONS %u
	in >> str >> uint;
	if ( in.fail() || str.compare("POLYGONS") != 0)
	{
		streamfail();
	}

	m_polygons.reserve(uint);
	for (; uint > 0; --uint)
	{
		P poly;
		if (!poly.read(in))
		{
			streamfail();
		}
		m_polygons.push_back(poly);
	}

	// Optional: CONNECTORS %u
	cnctrStrt = in.tellg();
	in >> str >> uint;
	if ( in.fail() || str.compare("CONNECTORS") != 0)
	{
		// no connectors or eof
		in.clear();
		in.seekg(cnctrStrt);
		return true;
	}

	for (; uint > 0; --uint)
	{
		C cnctr;
		if (!cnctr.read(in))
		{
			/* Connectors are optional, but if they're bad
			 * we return failure.
			 */
			streamfail();
		}
		m_connectors.push_back(cnctr);
	}

	return true;
#undef streamfail
}

template<typename V, typename P, typename C>
void APieLevel< V, P, C>::write(std::ostream &out) const
{
	typename std::vector<V>::const_iterator ptIt;
	typename std::vector<P>::const_iterator polyIt;
	typename std::list<C>::const_iterator cIt;

	if (!m_material.isDefault())
		out << PIE_MODEL_DIRECTIVE_MATERIALS << " " << m_material << '\n';

	if (!m_shader_vert.empty())
	{
		out << PIE_MODEL_DIRECTIVE_SHADERS << " " << 2 << " " << m_shader_vert
		    << " " << m_shader_frag << '\n';
	}

	out << "POINTS " << points() << '\n';
	for (ptIt = m_points.begin(); ptIt != m_points.end(); ++ptIt)
	{
		V p;
		p = *ptIt;
		p.scale(-1.0, 1.0, 1.0);
		out << '\t' << p.x() << ' ' << p.y() << ' ' << p.z() << '\n';
	}

	out << "POLYGONS " << polygons() << '\n';
	for (polyIt = m_polygons.begin(); polyIt != m_polygons.end(); ++polyIt)
	{
		out << "\t";
		polyIt->write(out);
	}

	if (connectors() != 0)
	{
		out << "CONNECTORS " << connectors() << '\n';
		for (cIt = m_connectors.begin(); cIt != m_connectors.end(); ++cIt)
		{
			out << "\t";
			cIt->write(out);
		}
	}
}

template<typename V, typename P, typename C>
int APieLevel< V, P, C>::points() const
{
	return m_points.size();
}

template<typename V, typename P, typename C>
int APieLevel< V, P, C>::polygons() const
{
	return m_polygons.size();
}

template<typename V, typename P, typename C>
int APieLevel< V, P, C>::connectors() const
{
	return m_connectors.size();
}

template<typename V, typename P, typename C>
void APieLevel< V, P, C>::clearAll()
{
	m_points.clear();
	m_polygons.clear();
	m_connectors.clear();
}

template<typename V, typename P, typename C>
bool APieLevel<V, P, C>::isValid() const
{
	typename std::vector<P>::const_iterator it;
	unsigned i;

	for (it = m_polygons.begin(); it != m_polygons.end(); ++it)
	{
		for (i = 0; i < it->vertices(); ++i)
		{
			if (it->getIndex(i) >= m_points.size())
			{
				return false;
			}
		}
	}
	return true;
}

template <typename V>
bool PieConnector<V>::read(std::istream& in)
{
	in >> pos.x() >> pos.y() >> pos.z();
	return in.good() || in.eof();
}

template <typename V>
void PieConnector<V>::write(std::ostream& out) const
{
	out << pos.x() << ' '
			<< pos.y() << ' '
			<< pos.z() << '\n';
}

template <typename L>
void APieModel<L>::clearAll()
{
	m_levels.clear();
	m_type = 0;

	m_texture.clear();
	m_texture_tcmask.clear();
	m_texture_normalmap.clear();
    m_texture_specmap.clear();
}

template <typename L>
APieModel<L>::APieModel()
{
}

template <typename L>
APieModel<L>::~APieModel()
{

}

template <typename L>
unsigned APieModel<L>::getType() const
{
	unsigned type = 0;

	if (!m_texture.empty())
	{
		type |= PIE_MODEL_FEATURE_TEXTURED;
	}

	if (!m_texture_tcmask.empty())
	{
		type |= PIE_MODEL_FEATURE_TCMASK;
	}

	return type;
}

template <typename L>
inline bool APieModel<L>::isFeatureSet(unsigned feature) const
{
	return (m_type & feature);
}

#define streamfail() do {\
	clearAll();	\
	in.clear();	\
	in.seekg(start);	\
	return false; } while(0)

// TODO: Write error messages to std::cerr
template <typename L>
bool APieModel<L>::read(std::istream& in)
{
	std::streampos start = in.tellg();

	clearAll();

	if (readHeaderBlock(in) && readTexturesBlock(in) && readLevelsBlock(in))
	{
		return true;
	}

	// do error macro
	streamfail();
}

#undef streamfail

template <typename L>
bool APieModel<L>::readHeaderBlock(std::istream& in)
{
	std::string str;
	unsigned uint;

	// PIE %u
	in >> str >> uint;
	if ( in.fail() || str.compare(PIE_MODEL_SIGNATURE) != 0)
	{
		return false;
	}

	// TYPE %x
	in >> str >> std::hex >> m_type >> std::dec;
	if ( in.fail() || str.compare(PIE_MODEL_DIRECTIVE_TYPE) != 0)
	{
		return false;
	}

	return true;
}

template <typename L>
bool APieModel<L>::readTexturesBlock(std::istream& in)
{
    return readTextureDirective(in) && readNormalmapDirective(in) && readSpecmapDirective(in);
}

// PIE2 specialization
template <>
inline bool APieModel<Pie2Level>::readTexturesBlock(std::istream& in)
{
	return readTextureDirective(in);
}

template <typename L>
bool APieModel<L>::readTextureDirective(std::istream& in)
{
	std::string str;
	unsigned uint;

	// TEXTURE 0 %s %u %u
	in >> str >> uint >> m_texture >> uint >> uint;
	if ( in.fail() || str.compare(PIE_MODEL_DIRECTIVE_TEXTURE) != 0)
	{
		return false;
	}

	if (!isValidWzName(m_texture))
	{
		return false;
	}

	if (isFeatureSet(PIE_MODEL_FEATURE_TCMASK))
	{
		m_texture_tcmask = makeWzTCMaskName(m_texture);
	}

	return true;
}

// Optional directive
template <typename L>
bool APieModel<L>::readNormalmapDirective(std::istream& in)
{
	std::string str;
	unsigned uint;
	std::streampos entrypoint = in.tellg();

	// NORMALMAP 0 %s
	in >> str >> uint >> m_texture_normalmap;
	if ( in.fail())
	{
		return false;
	}

	if (str.compare(PIE_MODEL_DIRECTIVE_NORMALMAP) != 0)
	{
		m_texture_normalmap.clear();
		in.seekg(entrypoint);
	}

	// no constraits for normalmap name afaik

	return true;
}

// Optional directive
template <typename L>
bool APieModel<L>::readSpecmapDirective(std::istream& in)
{
    std::string str;
    unsigned uint;
    std::streampos entrypoint = in.tellg();

    // <TYPE> 0 %s
    in >> str >> uint >> m_texture_specmap;
    if ( in.fail())
    {
        return false;
    }

    if (str.compare(PIE_MODEL_DIRECTIVE_SPECULARMAP) != 0)
    {
        m_texture_specmap.clear();
        in.seekg(entrypoint);
    }

    // no constraits for name afaik

    return true;
}

template <typename L>
bool APieModel<L>::readLevelsBlock(std::istream& in)
{
	std::string str;
	unsigned uint;

	// LEVELS %u
	in >> str >> uint;
	if ( in.fail() || str.compare(PIE_MODEL_DIRECTIVE_LEVELS) != 0)
	{
		return false;
	}

	for (; uint > 0; --uint)
	{
		L lvl;
		if (!lvl.read(in))
		{
			return false;
		}
		m_levels.push_back(lvl);
	}
	return true;
}

//FIXME add PIE2 specialization (no NM!)
template <typename L>
void APieModel<L>::write(std::ostream& out) const
{
	typename std::vector<L>::const_iterator it;
	unsigned i = 1;

	out << PIE_MODEL_SIGNATURE << " " << version() << '\n';

	out << PIE_MODEL_DIRECTIVE_TYPE << " " << std::hex << getType() << std::dec << '\n';

	out << PIE_MODEL_DIRECTIVE_TEXTURE << " 0 " << m_texture << ' '
			<< textureWidth() << ' '
			<< textureHeight() << '\n';

	if (!m_texture_normalmap.empty())
	{
		out << PIE_MODEL_DIRECTIVE_NORMALMAP << " 0 " << m_texture_normalmap << '\n';
	}

	if (!m_texture_specmap.empty())
	{
		out << PIE_MODEL_DIRECTIVE_SPECULARMAP << " 0 " << m_texture_specmap << '\n';
	}

	out << PIE_MODEL_DIRECTIVE_LEVELS << " " << levels() << '\n';

	for (it = m_levels.begin(); it != m_levels.end(); ++it, ++i)
	{
		out << "LEVEL " << i << '\n';
		it->write(out);
	}
}

template <typename L>
unsigned APieModel<L>::levels() const
{
	return m_levels.size();
}

template<typename L>
bool APieModel<L>::isValid() const
{
	typename std::vector<L>::const_iterator it;

	if (!isValidWzName(m_texture))
	{
		return false;
	}

	for (it = m_levels.begin(); it != m_levels.end(); ++it)
	{
		if(!it->isValid())
		{
			return false;
		}
	}
	return true;
}
