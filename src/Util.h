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

#ifndef UTIL_HPP
#define UTIL_HPP
#include <string>
#include <QString>

class QSettings;
class QWidget;

bool isValidWzName(const std::string name);
std::string makeWzTCMaskName(const std::string& name);

QString getTextureName(const QString& filePath);

/*!
 * Restores a widget's size and position from \a settings using the keys
 * "\a groupKey/size" and "\a groupKey/position", discarding geometry that
 * would not land on a currently connected screen.
 */
void restoreWidgetGeometry(QWidget& widget, QSettings& settings, const QString& groupKey);


#endif // UTIL_HPP
