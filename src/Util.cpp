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

#include "Util.h"

#include <QtDebug>

#include <QFileInfo>
#include <QTextStream>
#include <QRegExp>

#include "Pie.h"
#include "wmit.h"

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

std::string makeWzTCMaskName(const std::string& name)
{
/* Should add suffix before file ext, but wz waits for texpage mask
	std::string tcmask;
	std::string::size_type dotfound = name.find_last_of('.');

	if (dotfound != std::string::npos)
	{
		tcmask = name;
		tcmask.insert(dotfound, PIE_MODEL_TCMASK_SUFFIX);
	}

	return tcmask;
*/

	QString tcmask(QString::fromStdString(name));
	QRegExp pageNoRegX(WMIT_WZ_TEXPAGE_REMASK);

	if ((pageNoRegX.indexIn(tcmask) != -1) && tcmask.contains("."))
	{
		tcmask = pageNoRegX.cap(0).append(PIE_MODEL_TCMASK_SUFFIX) +
				tcmask.right(tcmask.size() - tcmask.lastIndexOf("."));
	}
	else
	{
		tcmask.clear();
	}

	return tcmask.toStdString();
}

inline QString getWZMTextureName(const QString& filePath);
inline QString getPIETextureName(const QString& filePath);
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
