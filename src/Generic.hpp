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
#ifndef GENERIC_HPP
#define GENERIC_HPP
#include <functional>
#include <algorithm>
#include <iterator>
#include <utility>
#include <sstream>
#include <string>
#include <cctype>
#include <iostream>
#include <vector>

inline void skipWhitespace(std::stringstream& ss)
{
	std::string str = ss.str();
	std::string::iterator itStr;
	itStr = std::find_if(str.begin() + ss.tellg(), str.end(), isalnum);
	ss.seekg(std::distance(str.begin(), itStr));
}

std::vector<std::string> split (std::string& str);
std::vector<std::string> split (std::string& str, char delim);
std::vector<std::string> split (std::istringstream& iss);
std::vector<std::string> split (std::istringstream& iss, char delim);

// For DIY copy_if
template <class Container, class F>
struct conditional_insert_iterator : public std::insert_iterator<Container>
{
	const F f;
public:
	explicit conditional_insert_iterator (Container& x, typename Container::iterator i,  const F& op)
	   : f(op), std::insert_iterator<Container>(x,i) {}
	conditional_insert_iterator& operator= (typename Container::const_reference value)
	{
		return f(value)?
						std::insert_iterator<Container>::operator =(value):
						*this;
	}
};

template <class Container, class F>
		struct conditional_back_insert_iterator : public std::back_insert_iterator<Container>
{
	const F f;
public:
	explicit conditional_back_insert_iterator (Container& x, const F& op)
	   : f(op), std::back_insert_iterator<Container>(x) {}
	conditional_back_insert_iterator& operator= (typename Container::const_reference value)
	{
		return f(value)?
						std::back_insert_iterator<Container>::operator =(value):
						*this;
	}
};

template <class F> class mybinder1st
	: public std::unary_function <typename F::second_argument_type,
						   typename F::result_type>
{
protected:
  const F f;
  typename F::first_argument_type lhs;
public:
  mybinder1st (const typename F::first_argument_type& x, const F& op = F()) : f(op), lhs(x) {}
  typename F::result_type
	operator() (const typename F::second_argument_type& rhs) const
	{ return f(lhs,rhs); }
};

template <class F> class mybinder2nd
	: public std::unary_function <typename F::second_argument_type,
									typename F::result_type>
{
protected:
  const F f;
  typename F::second_argument_type rhs;
public:
  mybinder2nd (const typename F::second_argument_type& y, const F& op = F()) : f(op), rhs(y) {}
  typename F::result_type
	operator() (const typename F::first_argument_type& lhs) const
	{ return f(lhs,rhs); }
};

template <class T1, class T2, class F1 = std::less<T1>, class F2 = std::less<T2> >
struct mypair : public std::pair<T1,T2>
{
	mypair(){}
	mypair(T1 x, T2 y) : std::pair<T1,T2>(x,y){}
	mypair(T1 x, T2 y, F1 fa, F2 fb) : std::pair<T1,T2>(x,y), f1(fa), f2(fb){}
	bool operator <(const mypair& rhs) const
	{
		if (!f1(this->first, rhs.first) && !f1(rhs.first, this->first))
		{
			return f2(this->second, rhs.second);
		}
		return f1(this->first, rhs.first);
	}
private:
	const F1 f1;
	const F2 f2;
};

template <class T1, class T2>
mypair<T1,T2> make_mypair (T1 x, T2 y)
{
	return ( mypair<T1,T2>(x,y) );
}

template <class T1, class T2, class F1, class F2>
mypair<T1, T2, F1, F2> make_mypair (T1 x, T2 y, F1 f1, F2 f2)
{
	return ( mypair<T1, T2, F1, F2>(x,y, f1, f2) );
}

#endif // GENERIC_HPP
