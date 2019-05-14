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
#ifndef VERTEX_HPP
#define VERTEX_HPP
#include <cstddef>
#include <limits>
#include <cmath>
#include <functional>

template <typename T, size_t COMPONENTS>
struct Vector
{
	Vector() {}
	Vector(const Vector& rhs)
	{
		*this = rhs;
	}

	Vector& operator = (const Vector& rhs)
	{
		unsigned i;
		for (i = 0; i < COMPONENTS; ++i)
		{
			component[i] = rhs.component[i];
		}
		return *this;
	}

	inline T  operator [](size_t i) const {
		return component[i];
	}
	inline T& operator [](size_t i)       {
		return component[i];
	}
#if defined(_MSC_VER) && !defined(_WIN64)
	// for MSVC
	inline T  operator [](int i) const {
		return component[i];
	}
	// for MSVC
	inline T& operator [](int i) {
		return component[i];
	}
#endif
	inline operator const T*() const {
		return component;
	}

	struct equal_wEps : public std::binary_function<Vector, Vector, bool>
	{
		equal_wEps(T eps = std::numeric_limits<T>::epsilon())
		{
			setEps(eps);
		}
		bool operator() (Vector lhs, Vector rhs)const
		{
			unsigned i;
			for (i = 0; i < COMPONENTS; ++i)
			{
				if (std::abs(lhs[i] - rhs[i]) > m_eps)
				{
					return false;
				}
			}
			return true;
		}
		inline void setEps(T eps = std::numeric_limits<T>::epsilon())
		{
			if (eps <= std::numeric_limits<T>::epsilon())
			{
				m_eps = std::numeric_limits<T>::epsilon();
			}
			else
			{
				m_eps = eps;
			}
		}
	private:
		T m_eps;
	};

	bool operator == (const Vector& rhs) const
	{
		unsigned i;
		for (i = 0; i < COMPONENTS; ++i)
		{
			if (component[i] != rhs[i])
			{
				return false;
			}
		}
		return true;
	}

	Vector& operator += (const Vector& rhs)
	{
		unsigned i;
		for (i = 0; i < COMPONENTS; ++i)
		{
			component[i] += rhs.component[i];
		}
		return *this;
	}

	Vector& operator -= (const Vector& rhs)
	{
		unsigned i;
		for (i = 0; i < COMPONENTS; ++i)
		{
			component[i] -= rhs.component[i];
		}
		return *this;
	}

	Vector operator + (const Vector& rhs) const
	{
		unsigned i;
		Vector tmpV;
		for (i = 0; i < COMPONENTS; ++i)
		{
			tmpV.component[i] = component[i] + rhs.component[i];
		}
		return tmpV;
	}

	Vector operator - (const Vector& rhs) const
	{
		unsigned i;
		Vector tmpV;
		for (i = 0; i < COMPONENTS; ++i)
		{
			tmpV.component[i] = component[i] - rhs.component[i];
		}
		return tmpV;
	}

	Vector operator * (const Vector& rhs) const
	{
		unsigned i;
		Vector tmpV;
		for (i = 0; i < COMPONENTS; ++i)
		{
			tmpV.component[i] = component[i] * rhs.component[i];
		}
		return tmpV;
	}

	Vector& operator *= (const Vector& rhs)
	{
		unsigned i;
		for (i = 0; i < COMPONENTS; ++i)
		{
			component[i] *= rhs.component[i];
		}
		return *this;
	}

	Vector operator * (const T& s) const
	{
		unsigned i;
		Vector tmpV;
		for (i = 0; i < COMPONENTS; ++i)
		{
			tmpV.component[i] = component[i] * s;
		}
		return tmpV;
	}

	Vector operator / (const T& s) const
	{
		unsigned i;
		Vector tmpV;
		for (i = 0; i < COMPONENTS; ++i)
		{
			tmpV.component[i] = component[i] / s;
		}
		return tmpV;
	}

	bool operator < (const Vector& rhs) const
	{
		unsigned i;
		for (i = 0; i < COMPONENTS-1; ++i)
		{
			if (component[i] != rhs.component[i])
			{
				return component[i] < rhs.component[i];
			}
		}
		return component[i] < rhs.component[i];
	}

	struct less_wEps : public std::binary_function<Vector, Vector, bool>
	{
		less_wEps(T eps = -1): compare(eps) {}
		bool operator() (Vector lhs, Vector rhs)const
		{
			return  (lhs < rhs) && !compare(lhs, rhs);
		}
	private:
		equal_wEps compare;
	};

	bool sameComponents(const T& val) const
	{
		unsigned i;
		for (i = 0; i < COMPONENTS; ++i)
		{
			if (component[i] != val)
				return false;
		}
		return true;
	}

protected:
	T component[COMPONENTS];
};

#endif // VERTEX_HPP
