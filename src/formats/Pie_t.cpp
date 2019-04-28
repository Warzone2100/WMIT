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

// Optional directive
template<typename V, typename P, typename C>
bool APieLevel< V, P, C>::readAnimObjectDirective(std::istream& in)
{
	std::string str;
	std::streampos entrypoint = in.tellg();

	in >> str;
	if (in.eof())
		return true;
	if (in.fail())
		return false;

	if (str.compare(PIE_MODEL_DIRECTIVE_ANIMOBJECT) != 0)
	{
		in.seekg(entrypoint);
		return true;
	}

	return m_animobj.read(in);
}

// TODO: Write error messages to std::cerr
template<typename V, typename P, typename C>
bool APieLevel< V, P, C>::read(std::istream& in)
{
	std::string str;
	unsigned uint;
	std::streampos mark;

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
		m_points.emplace_back(point);
	}

	if (in.fail())
	{
		streamfail();
	}

	// Optional: NORMALS %u
	mark = in.tellg();
	in >> str >> uint;
	if ( !in.fail() && str.compare("NORMALS") == 0)
	{
		uint *= 3;
		for (; uint > 0; --uint)
		{
			PieNormal normal;
			in >> normal.x() >> normal.y() >> normal.z();
			if (in.fail())
			{
				/* Normals are optional, but if they're bad
				 * we return failure.
				 */
				streamfail();
			}
			m_normals.emplace_back(normal);
		}
	}
	else
	{
		// no normals or eof
		in.clear();
		in.seekg(mark);
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
		m_polygons.emplace_back(poly);
	}

	mark = in.tellg();

	// Optional: CONNECTORS %u
	in >> str >> uint;
	if ( !in.fail() && str.compare("CONNECTORS") == 0)
	{
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
			m_connectors.emplace_back(cnctr);
		}
	}
	else
	{
		// no connectors or eof
		in.clear();
		in.seekg(mark);
	}

	if (!in.eof() && !readAnimObjectDirective(in))
		streamfail();

	return true;
#undef streamfail
}

template<typename V, typename P, typename C>
void APieLevel< V, P, C>::write(std::ostream &out, const PieCaps &caps) const
{
	typename std::vector<V>::const_iterator ptIt;
	typename std::vector<P>::const_iterator polyIt;
	typename std::list<C>::const_iterator cIt;

	if (caps.test(PIE_OPT_DIRECTIVES::podMATERIALS) && !m_material.isDefault())
		out << PIE_MODEL_DIRECTIVE_MATERIALS << " " << m_material << '\n';

	if (caps.test(PIE_OPT_DIRECTIVES::podSHADERS) && !m_shader_vert.empty())
	{
		out << PIE_MODEL_DIRECTIVE_SHADERS << " " << 2 << " " << m_shader_vert
		    << " " << m_shader_frag << '\n';
	}

	out << "POINTS " << points() << '\n';
	for (ptIt = m_points.begin(); ptIt != m_points.end(); ++ptIt)
	{
		out << '\t' << ptIt->x() << ' ' << ptIt->y() << ' ' << ptIt->z() << '\n';
	}

	if (caps.test(PIE_OPT_DIRECTIVES::podNORMALS) && (normals() != 0))
	{
		size_t nCnt = 0;

		out << "NORMALS " << static_cast<int>(normals() / 3);
		out << std::fixed;
		for (auto nIt = m_normals.begin(); nIt != m_normals.end(); ++nIt)
		{
			if (nCnt++ % 3 == 0)
				out << "\n\t";
			out << nIt->x() << ' ' << nIt->y() << ' ' << nIt->z();
			if (nCnt % 3 != 0)
				out << ' ';
		}
		out.unsetf(std::ios::floatfield);
		out << '\n';
	}

	out << "POLYGONS " << polygons() << '\n';
	for (polyIt = m_polygons.begin(); polyIt != m_polygons.end(); ++polyIt)
	{
		out << "\t";
		polyIt->write(out);
	}

	if (caps.test(PIE_OPT_DIRECTIVES::podCONNECTORS) && (connectors() != 0))
	{
		out << "CONNECTORS " << connectors() << '\n';
		for (cIt = m_connectors.begin(); cIt != m_connectors.end(); ++cIt)
		{
			out << "\t";
			cIt->write(out);
		}
	}

	if (caps.test(PIE_OPT_DIRECTIVES::podANIMOBJECT) && (m_animobj.isValid()))
	{
		out << PIE_MODEL_DIRECTIVE_ANIMOBJECT;
		m_animobj.write(out);
	}
}

template<typename V, typename P, typename C>
int APieLevel< V, P, C>::points() const
{
	return m_points.size();
}

template<typename V, typename P, typename C>
int APieLevel< V, P, C>::normals() const
{
	return m_normals.size();
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
	m_normals.clear();
	m_polygons.clear();
	m_connectors.clear();
	m_material.setDefaults();
	m_shader_frag.clear();
	m_shader_vert.clear();
	m_animobj.clear();
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
	out << pos.x() << ' ' << pos.y() << ' ' << pos.z() << '\n';
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
	m_events.clear();
}

template <typename L>
APieModel<L>::APieModel(const PieCaps& def_caps): m_def_caps(def_caps)
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

// Optional directive
template <typename L>
bool APieModel<L>::readEventsDirective(std::istream& in)
{
	std::string str;
	std::streampos entrypoint = in.tellg();

	// EVENT type filename.pie
	in >> str;
	if (in.fail())
	{
		in.seekg(entrypoint);
		return false;
	}

	if (str.compare(PIE_MODEL_DIRECTIVE_EVENT) != 0)
	{
		in.seekg(entrypoint);
		// not really a fail, but will work for a "while"
		return false;
	}

	int type;

	in >> type >> str;
	if (in.fail())
		return false;

	m_events.emplace(type, str);
	return true;
}

template <typename L>
bool APieModel<L>::readLevelsBlock(std::istream& in)
{
	// Optional sequence of event directives
	while (readEventsDirective(in)) {}

	int levels = readLevelsDirective(in);

	if (levels < 0)
		return false;

	if (!readLevels(levels, in))
		return false;

	return true;
}

/*
 * WZ loader basically ignores PIE version and reads any directive,
 * but real PIE2 would work like that:
 *
// PIE2 specialization
template <>
inline bool APieModel<Pie2Level>::readLevelsBlock(std::istream& in)
{
	int levels = readLevelsDirective(in);

	if (levels < 0)
		return false;

	return readLevels(levels, in);
}
*/

template <typename L>
bool APieModel<L>::readLevels(int levels, std::istream& in)
{
	for (; levels > 0; --levels)
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

template <typename L>
int APieModel<L>::readLevelsDirective(std::istream& in)
{
	std::string str;
	unsigned uint;

	// LEVELS %u
	in >> str >> uint;
	if ( in.fail() || str.compare(PIE_MODEL_DIRECTIVE_LEVELS) != 0)
	{
		return -1;
	}

	return static_cast<int>(uint);
}

template <typename L>
void APieModel<L>::write(std::ostream& out, const PieCaps *piecaps) const
{
	typename std::vector<L>::const_iterator it;
	unsigned i = 1;

	const PieCaps& caps(piecaps ? *piecaps : m_def_caps);

	out << PIE_MODEL_SIGNATURE << " " << version() << '\n';

	out << PIE_MODEL_DIRECTIVE_TYPE << " " << std::hex << getType() << std::dec << '\n';

	out << PIE_MODEL_DIRECTIVE_TEXTURE << " 0 " << m_texture << ' '
			<< textureWidth() << ' '
			<< textureHeight() << '\n';

	if (caps.test(PIE_OPT_DIRECTIVES::podNORMALMAP) && !m_texture_normalmap.empty())
	{
		out << PIE_MODEL_DIRECTIVE_NORMALMAP << " 0 " << m_texture_normalmap << '\n';
	}

	if (caps.test(PIE_OPT_DIRECTIVES::podSPECULARMAP) && !m_texture_specmap.empty())
	{
		out << PIE_MODEL_DIRECTIVE_SPECULARMAP << " 0 " << m_texture_specmap << '\n';
	}

	if (caps.test(PIE_OPT_DIRECTIVES::podEVENT) && !m_events.empty())
	{
		for (const auto& evt : m_events)
		{
			out << PIE_MODEL_DIRECTIVE_EVENT << " " << evt.first << " " << evt.second << '\n';
		}
	}

	out << PIE_MODEL_DIRECTIVE_LEVELS << " " << levels() << '\n';

	for (it = m_levels.begin(); it != m_levels.end(); ++it, ++i)
	{
		out << "LEVEL " << i << '\n';
		it->write(out, caps);
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
