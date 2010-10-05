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

#include "Util.hpp"

#include <QtDebug>

#include <QFileInfo>
#include <QTextStream>

#include <lib3ds/file.h>
#include <lib3ds/material.h>

#include "Pie.hpp"

bool isValidWzName(const std::string name)
{
	static const std::string valid = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
									 "abcdefghijklmnopqrstuvwxyz"
									 "0123456789._\\-";
	std::string::size_type found = name.find_first_not_of(valid);
	if (found == std::string::npos)
	{
		return true;
	}
	return false;
}

inline QString getWZMTextureName(const QString& filePath);
inline QString getPIETextureName(const QString& filePath);
inline QString get3DSTextureName(const QString& filePath);
//inline QString getOBJTextureName(const QString& filePath); // OBJ uses material files which contain the texture info
QString getTextureName(const QString& filePath)
{
	QFileInfo modelFileNfo(filePath);
	if (!modelFileNfo.exists())
	{
		return QString();
	}
	else if (modelFileNfo.completeSuffix().compare(QString("wzm"), Qt::CaseInsensitive) == 0)
	{
		return getWZMTextureName(modelFileNfo.absoluteFilePath());
	}
	else if(modelFileNfo.completeSuffix().compare(QString("pie"), Qt::CaseInsensitive) == 0)
	{
		return getPIETextureName(modelFileNfo.absoluteFilePath());
	}
	else if(modelFileNfo.completeSuffix().compare(QString("3ds"), Qt::CaseInsensitive) == 0)
	{
		return getWZMTextureName(modelFileNfo.absoluteFilePath());
	}
#ifdef getOBJTextureName
	else if(modelFileNfo.completeSuffix().compare(QString("obj"), Qt::CaseInsensitive) == 0)
	{
		return getOBJTextureName(modelFileNfo.absoluteFilePath());
	}
#endif
	return QString();
}

inline QString getWZMTextureName(const QString& filePath)
{
	QFile f(filePath);
	if (!f.open(QFile::ReadOnly))
	{
		return QString();
	}
	QTextStream in(&f);
	QString qstr;
	unsigned uint;

	if (in.status() != QTextStream::Ok || qstr.compare("WZM") != 0)
	{
		return  QString();
	}

	in >> uint;
	if (in.status() != QTextStream::Ok)
	{
		return  QString();
	}

	in >> qstr;
	if (qstr.compare("TEXTURE") != 0)
	{
		return  QString();
	}
	in >> qstr;
	if (in.status() != QTextStream::Ok)
	{
		return  QString();
	}
	return qstr;
}

inline QString getPIETextureName(const QString& filePath)
{
	QFile f(filePath);
	if (!f.open(QFile::ReadOnly))
	{
		return QString();
	}
	QTextStream in(&f);
	QString qstr;
	unsigned uint;

	// PIE %u
	in >> qstr >> uint;
	if (in.status() != QTextStream::Ok || qstr.compare("PIE") != 0)
	{
		return QString();
	}

	// TYPE %x
	in >> qstr >> uint;
	if (in.status() != QTextStream::Ok || qstr.compare("TYPE") != 0)
	{
		return QString();
	}

	// TEXTURE 0 %s %u %u
	in >> qstr >> uint;
	if (in.status() != QTextStream::Ok || qstr.compare("TEXTURE") != 0)
	{
		return QString();
	}
	in >> qstr;
	if (in.status() != QTextStream::Ok)
	{
		return  QString();
	}
	return qstr;
}

inline QString get3DSTextureName(const QString& filePath)
{
	Lib3dsFile *f = lib3ds_file_load(filePath.toLocal8Bit().constData());
	Lib3dsMaterial *material;

	if (f == NULL)
	{
		std::cerr << "Loading 3DS file failed.\n";
		return QString();
	}

	material = f->materials;

	// Grab texture name
	if (material != NULL)
	{
		if (material->next != NULL)
		{
			std::cout << "WZM::importFrom3ds - Multiple textures not supported!\n";
		}
		return material->texture1_map.name;
	}
}

