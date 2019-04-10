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

#include <QApplication>
#include <QCoreApplication>
#include <QTextCodec>
#include <QSettings>

#include <fstream>

#include "MainWindow.h"
#include "WZM.h"
#include "Pie.h"
#include "wmit.h"

#if defined(Q_OS_WIN)
#include <QtPlugin>
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
#endif

int main(int argc, char *argv[])
{
    //QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());

	if (argc > 2)
	{
		// command line conversion mode
		QString inname = argv[1];
		QString outname = argv[2];

		wmit_filetype_t outtype;

		if (!MainWindow::guessModelTypeFromFilename(outname, outtype))
			return 1;

		WZM model;

		if (!MainWindow::loadModel(inname, model, true))
			return 1;

		return !MainWindow::saveModel(outname, model, outtype);
	}
	else
	{
		QApplication a(argc, argv);

		a.setApplicationName(WMIT_APPNAME);
		a.setOrganizationName(WMIT_ORG);
		QSettings::setDefaultFormat(QSettings::IniFormat);

		MainWindow w;
		w.show();

		if (argc == 2)
		{
			QString inname = argv[1];
			w.openFile(inname);
		}

		return a.exec();
	}
}
