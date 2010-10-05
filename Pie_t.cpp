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
#ifdef PIE_T_CPP

#include "Generic.hpp"
#include "Util.hpp"

#include "Pie.hpp" // Hack for autocomplete

/*
  *
  * Template implementations
  *
  *
*/

template<typename V, typename P, typename C>
APieLevel< V, P, C>::APieLevel()
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

	// POINTS %u
	in >> str >> uint;
	if ( in.fail() || str.compare("POINTS") != 0)
	{
		streamfail();
	}

	m_points.reserve(uint);
	for (; uint > 0; --uint)
	{
		V point;
		in >> point.x() >> point.y() >> point.z();
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

	out << "POINTS\t" << points() << '\n';
	for (ptIt = m_points.begin(); ptIt != m_points.end(); ++ptIt)
	{
		out << '\t' << ptIt->x()
				<< '\t' << ptIt->y()
				<< '\t' << ptIt->z() << '\n';
	}

	out << "POLYGONS\t" << polygons() << '\n';
	for (polyIt = m_polygons.begin(); polyIt != m_polygons.end(); ++polyIt)
	{
		out << "\t";
		polyIt->write(out);
	}

	if (connectors() != 0)
	{
		out << "CONNECTORS\t" << connectors() << '\n';
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
	out << pos.x() << '\t'
			<< pos.y() << '\t'
			<< pos.z() << '\n';
}

template <typename L>
void APieModel<L>::clearAll()
{
	m_levels.clear();
	m_type = 0;
}

template <typename L>
APieModel<L>::APieModel()
{
}

template <typename L>
APieModel<L>::~APieModel()
{

}

// TODO: Write error messages to std::cerr
template <typename L>
bool APieModel<L>::read(std::istream& in)
{
	std::string str;
	unsigned uint;

	std::streampos start = in.tellg();

	clearAll();

	#define streamfail() do {\
		clearAll();	\
		in.clear();	\
		in.seekg(start);	\
		return false; } while(0)

	// PIE %u
	in >> str >> uint;
	if ( in.fail() || str.compare("PIE") != 0)
	{
		streamfail();
	}

	// TYPE %x
	in >> str >> m_type;
	if ( in.fail() || str.compare("TYPE") != 0)
	{
		streamfail();
	}

	// TEXTURE 0 %s %u %u
	in >> str >> uint >> m_texture >> uint >> uint;
	if ( in.fail() || str.compare("TEXTURE") != 0)
	{
		streamfail();
	}

	if (!isValidWzName(m_texture))
	{
		streamfail();
	}

	// LEVELS %u
	in >> str >> uint;
	if ( in.fail() || str.compare("LEVELS") != 0)
	{
		streamfail();
	}

	for (; uint > 0; --uint)
	{
		L lvl;
		if (!lvl.read(in))
		{
			streamfail();
		}
		m_levels.push_back(lvl);
	}
	return true;
#undef streamfail
}

template <typename L>
void APieModel<L>::write(std::ostream& out) const
{
	typename std::vector<L>::const_iterator it;
	unsigned i = 1;

	out << "PIE\t"	<< version() << '\n';

	out << "TYPE\t"	<< m_type	<< '\n';

	out <<  "TEXTURE\t0\t" << m_texture << "\t"
			<< textureWidth() << "\t"
			<< textureHeight() << '\n';

	out << "LEVELS\t" << levels() << '\n';

	for (it = m_levels.begin(); it != m_levels.end(); ++it, ++i)
	{
		out << "LEVEL\t" << i << '\n';
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

#endif //PIE_T_CPP
