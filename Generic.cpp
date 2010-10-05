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

#include "Generic.hpp"

std::vector<std::string> split(std::istringstream& iss)
{
	using namespace std;
	vector<string> tokens;
	copy(istream_iterator<string>(iss),
		 istream_iterator<string>(),
		 back_inserter<vector<string> >(tokens));
	return tokens;
}

std::vector<std::string> split(std::istringstream& iss, char delim)
{
	using namespace std;
	vector<string> tokens;
	string str;
	while (!(iss.eof()|| iss.fail()))
	{
		getline(iss, str, delim);
		tokens.push_back(str);
	}
	return tokens;
}

std::vector<std::string> split(std::string& str)
{
	std::istringstream ss(str);
	return split(ss);
}

std::vector<std::string> split(std::string& str, char delim)
{
	std::istringstream ss(str);
	return split(ss, delim);
}
