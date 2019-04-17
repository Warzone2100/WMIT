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
#ifndef PIE_HPP
#define PIE_HPP

#include <iostream>
#include <vector>
#include <map>
#include <list>
#include <QtOpenGL/qgl.h>
#include "VectorTypes.h"
#include "Polygon.h"

#include "WZM.h" // for friends

/**
  * Templates used to remove tedious code
  * duplication.
  *
  * See this doc for more details:
  * https://github.com/Warzone2100/warzone2100/blob/master/doc/PIE.md
  */

#define PIE_MODEL_SIGNATURE "PIE"
#define PIE_MODEL_DIRECTIVE_TYPE "TYPE"
#define PIE_MODEL_DIRECTIVE_TEXTURE "TEXTURE"
#define PIE_MODEL_DIRECTIVE_NORMALMAP "NORMALMAP"
#define PIE_MODEL_DIRECTIVE_MATERIALS "MATERIALS" // WZ 3.2 only
#define PIE_MODEL_DIRECTIVE_LEVELS "LEVELS"
#define PIE_MODEL_DIRECTIVE_SPECULARMAP "SPECULARMAP"
#define PIE_MODEL_DIRECTIVE_SHADERS "SHADERS"
#define PIE_MODEL_DIRECTIVE_EVENT "EVENT" // WZ 3.3
#define PIE_MODEL_DIRECTIVE_ANIMOBJECT "ANIMOBJECT" // WZ 3.3

#define PIE_MODEL_FEATURE_TEXTURED 0x200
#define PIE_MODEL_FEATURE_TCMASK 0x10000

#define PIE_MODEL_TEXPAGE_PREFIX "page-"
#define PIE_MODEL_TCMASK_SUFFIX "_tcmask"

class ApieAnimFrame
{
public:
	int num;
	Vertex<int> pos, rot;
	Vertex<float> scale;

	bool read(std::istream& in);
};

class ApieAnimObject
{
public:
	int time, cycles, numframes;
	std::vector<ApieAnimFrame> frames;

	bool isValid() const {return !frames.empty();}
	void clear() {frames.clear();}
	bool read(std::istream& in);
	//void write(std::ostream& out) const;
};

template<typename V, typename P, typename C>
class APieLevel
{
	friend Mesh::operator Pie3Level() const;
	friend Mesh::Mesh(const Pie3Level& p3);
public:
	APieLevel();
	virtual ~APieLevel(){}

	virtual bool read(std::istream& in);
	virtual void write(std::ostream& out) const;

	int points() const;
	int polygons() const;
	int connectors() const;

	bool isValid() const;

protected:
	void clearAll();
	bool readAnimObjectDirective(std::istream &in);

	std::vector<V> m_points;
	std::vector<P> m_polygons;
	std::list<C> m_connectors;
	WZMaterial m_material; // PIE3+
	std::string m_shader_vert;
	std::string m_shader_frag;
	ApieAnimObject m_animobj;
};

template <typename L>
class APieModel
{
public:
	APieModel();
	virtual ~APieModel();

	virtual unsigned version() const =0;

	virtual bool read(std::istream& in);
	virtual void write(std::ostream& out) const;

	unsigned levels() const;
	virtual unsigned getType() const;

	bool isValid() const;

	//virtual bool addFeature(unsigned feature);
	//virtual bool removeFeature(unsigned feature);
	virtual bool isFeatureSet(unsigned feature) const;
protected:

	void clearAll();

	virtual unsigned textureHeight() const =0;
	virtual unsigned textureWidth() const =0;

	virtual bool readHeaderBlock(std::istream& in);

	virtual bool readTexturesBlock(std::istream& in);
	bool readTextureDirective(std::istream& in);
	bool readNormalmapDirective(std::istream& in);
	bool readSpecmapDirective(std::istream& in);

	virtual bool readLevelsBlock(std::istream& in);
	bool readEventsDirective(std::istream& in);
	int readLevelsDirective(std::istream& in);
	bool readLevels(int levels, std::istream& in);
	bool readAnimObjectDirective(std::istream &in);

	std::string m_texture;
	std::string m_texture_normalmap;
	std::string m_texture_tcmask;
	std::string m_texture_specmap;
	std::map<int, std::string> m_events; // Animation events associated with this model

	std::vector<L> m_levels;

	unsigned m_type; // FIXME used as helper for 2->3 conversion, ignored for write
};

template <typename V>
struct PieConnector
{
	virtual ~PieConnector(){}
	bool read(std::istream& in);
	void write(std::ostream& out) const;
	V pos;
};


/** Returns the Pie version
  *
  *	@param	in	istream to a Pie file, the stream's position will be
  *		returned to where it was (on success).
  *	@return	int Version of the pie version.
  */
int pieVersion(std::istream& in);

/**********************************************
  Pie version 2
  *********************************************/
typedef UV<GLushort> Pie2UV;
typedef Vertex<GLint> Pie2Vertex;
typedef PieConnector<Pie2Vertex> Pie2Connector;

class Pie2Polygon : public PiePolygon<Pie2UV, GLushort, 16>
{
	friend class Pie3Polygon; // only for operator thisclass() and thatclass(const thisclass&)
public:
	Pie2Polygon(){}
	virtual ~Pie2Polygon(){}
};

class Pie2Level : public APieLevel<Pie2Vertex, Pie2Polygon, Pie2Connector>
{
	friend class Pie3Level; // only for operator thisclass() and thatclass(const thisclass&)
public:
	Pie2Level(){}
	virtual ~Pie2Level(){}
};

class Pie2Model : public APieModel<Pie2Level>
{
	friend class Pie3Model; // only for operator thisclass() and thatclass(const thisclass&)
public:
	Pie2Model();
	virtual ~Pie2Model();

	unsigned version() const;

	unsigned textureHeight() const;
	unsigned textureWidth() const;
};

/**********************************************
  Pie version 3
  *********************************************/
class Pie3UV : public UV<GLclampf>
{
public:
	Pie3UV();
	Pie3UV(GLclampf u, GLclampf v);
	Pie3UV(const Pie2UV& p2);
	operator Pie2UV() const;
};

class Pie3Vertex : public Vertex<GLfloat>
{
public:
	Pie3Vertex(){}
	Pie3Vertex(const Vertex<GLfloat>& vert);
	Pie3Vertex(const Pie2Vertex& p2);

	static inline Pie3Vertex upConvert(const Pie2Vertex& p2);
	static Pie2Vertex backConvert(const Pie3Vertex& p3);
	operator Pie2Vertex() const;
};

class Pie3Connector : public PieConnector<Pie3Vertex>
{
public:
	Pie3Connector(){}
	Pie3Connector(const Pie2Connector& p2);

	static Pie3Connector upConvert(const Pie2Connector& p2);
	static Pie2Connector backConvert(const Pie3Connector& p3);
	operator Pie2Connector() const;
};

class Pie3Polygon : public PiePolygon<Pie3UV, GLclampf, 3>
{
public:
	Pie3Polygon();
	virtual ~Pie3Polygon();

	static int upConvert(const Pie2Polygon& pie2Poly, std::back_insert_iterator<std::vector<Pie3Polygon> > result);
	static Pie2Polygon backConvert(const Pie3Polygon& p3);
	operator Pie2Polygon() const;

	Pie3UV getUV(unsigned index, unsigned frame) const;

private:
};

class Pie3Level : public APieLevel<Pie3Vertex, Pie3Polygon, Pie3Connector>
{
    friend WZM::WZM(const Pie3Model &p3);
    friend WZM::operator Pie3Model() const;
public:
	Pie3Level();
	Pie3Level(const Pie2Level& p2);
	virtual ~Pie3Level();

	static Pie3Level upConvert(const Pie2Level& p2);
	static Pie2Level backConvert(const Pie3Level& p3);
	operator Pie2Level() const;
};

class Pie3Model : public APieModel<Pie3Level>
{
	friend WZM::WZM(const Pie3Model &p3);
	friend WZM::operator Pie3Model() const;
public:
	Pie3Model();
	Pie3Model(const Pie2Model& pie2);
	virtual ~Pie3Model();

	unsigned version() const;

	operator Pie2Model() const;

	bool setType(int type);

protected:
	unsigned textureHeight() const;
	unsigned textureWidth() const;
};

// Include template implementations
#include "Pie_t.cpp"

#endif // PIE_HPP
