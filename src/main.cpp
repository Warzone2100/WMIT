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

#include <QtGui/QApplication>
#include <QCoreApplication>
#include <QTextCodec>
#include "MainWindow.hpp"

int main(int argc, char *argv[])
{
	QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
	//	if (command line mode)
	//		QCoreApplication a(argc, argv);
	//		TODO: Command line functionality
	//	else // gui mode1
	QApplication a(argc, argv);
	a.setApplicationName("WMIT");
	a.setOrganizationName("WMIT");
	MainWindow w;
	w.show();
	return a.exec();
}
