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
#ifndef VERTEXTYPES_HPP
#define VERTEXTYPES_HPP
#include "Vector.hpp"

template <typename T>
struct UV : public Vector<T,2>
{
	UV() {u() = 0, v() =0;}

	UV(T u, T v) {
		u() = u; v() = v;
	}

	inline T& u() {
		return this->operator [](0);
	}
	inline T u() const {
		return this->operator [](0);
	}

	inline T& v() {
		return this->operator [](1);
	}
	inline T v() const {
		return this->operator [](1);
	}
};

template <typename T>
struct Vertex : public Vector<T,3>
{
	Vertex()
	{
		x() = 0;
		y() = 0;
		z() = 0;
	}

	Vertex (T x, T y, T z)
	{
		x() = x;
		y() = y;
		z() = z;
	}

	inline T& x() {
		return this->operator [](0);
	}
	inline T x() const {
		return this->operator [](0);
	}

	inline T& y() {
		return this->operator [](1);
	}
	inline T y() const {
	return this->operator [](1);
	}

	inline T& z() {
		return this->operator [](2);
	}
	inline T z() const {
		return this->operator [](2);
	}
};

#endif // VERTEXTYPES_HPP
