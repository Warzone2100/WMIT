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
	inline T  operator [](unsigned i) const {
		return component[i];
	}
	inline T& operator [](unsigned i)       {
		return component[i];
	}
	inline operator const T*() const {
		return component;
	}

	struct equal_wEps : public std::binary_function<const Vector&, const Vector&, bool>
	{
		equal_wEps(T eps = std::numeric_limits<T>::epsilon())
		{
			if (eps < std::numeric_limits<T>::epsilon())
			{
				m_eps = std::numeric_limits<T>::epsilon();
			}
			else
			{
				m_eps = eps;
			}
		}
		bool operator() (const Vector& lhs, const Vector& rhs)const
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
		void setEps(double eps = std::numeric_limits<T>::epsilon())
		{
			m_eps = eps;
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

	struct less_wEps : public std::binary_function<const Vector&, const Vector&, bool>
	{
		less_wEps(T eps = -1): compare(eps) {}
		bool operator() (const Vector& lhs, const Vector& rhs)const
		{
			return  (lhs < rhs) && !compare(lhs, rhs);
		}
	private:
		equal_wEps compare;
	};

protected:
	T component[COMPONENTS];
};

#endif // VERTEX_HPP
