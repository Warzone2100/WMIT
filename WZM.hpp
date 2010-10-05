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
#ifndef WZM_HPP
#define WZM_HPP

#include <iostream>
#include <string>
#include <vector>

#include "Mesh.hpp"

#ifdef __GNUC__
# ifdef WARNMORE
#  pragma  GCC diagnostic warning "-Weffc++"
#  pragma  GCC diagnostic warning "-Wconversion"
# endif
#endif

class Pie3Model;

class WZM
{
public:
	WZM();
	WZM(const Pie3Model& p3);
	virtual ~WZM(){}

	virtual operator Pie3Model() const;

	bool read(std::istream& in);
	void write(std::ostream& out) const;

	bool importFromOBJ(std::istream& in);
	void exportToOBJ(std::ostream& out) const;

	bool importFrom3DS(std::string fileName);
	bool exportTo3DS(std::string fileName) const;

	int version() const;
	int meshes() const;

	void setTextureName(std::string name);
	std::string getTextureName() const;

	bool couldHaveTCArrays() const;

	/// might throw out_of_range exception? not decided yet
	Mesh& getMesh(int index);
	void addMesh (const Mesh& mesh);
	void rmMesh (int index);

	bool isValid() const;

protected:
	void clear();
	std::vector<Mesh> m_meshes;
	std::string m_texName;
};

#endif // WZM_HPP
