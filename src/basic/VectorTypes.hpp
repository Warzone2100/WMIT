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

	UV(const Vector<T,2>& rhs): Vector<T,2>(rhs) {}

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

	Vertex(T x, T y, T z)
	{
		this->x() = x;
		this->y() = y;
		this->z() = z;
	}

	Vertex(const Vector<T,3>& rhs): Vector<T,3>(rhs) {}

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

	inline void scale(T xfac, T yfac, T zfac) {
		this->operator [](0) *= xfac;
		this->operator [](1) *= yfac;
		this->operator [](2) *= zfac;
	}

	static Vertex crossProduct(const Vertex& lhs, const Vertex& rhs)
	{
		return Vertex(lhs.y() * rhs.z() - lhs.z() * rhs.y(),
			      lhs.z() * rhs.x() - lhs.x() * rhs.z(),
			      lhs.x() * rhs.y() - lhs.y() * rhs.x());
	}

	T operator *(const Vertex& rhs) const
	{
		return x() * rhs.x() + y() * rhs.y() + z() * rhs.z();
	}
};

#endif // VERTEXTYPES_HPP
