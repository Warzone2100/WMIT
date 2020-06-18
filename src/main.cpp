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

#if defined(Q_OS_WIN) && defined(QT_STATICPLUGIN)
#include <QtPlugin>
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
#endif

int main(int argc, char *argv[])
{
    //QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());

	if(argc == 2 && strcmp("--help", argv[1]) == 0)
	{
		printf("Usage:\n");
		printf("  WMIT (opens application)\n");
		printf("  WMIT --help (shows this message)\n");
		printf("  WMIT [filename] (opens a file)\n");
		printf("  WMIT [input] [output] (converts between formats wzm, pie and obj)\n");
		exit(0);
	}

	if (argc > 2)
	{
		// command line conversion mode
		QString inname = argv[1];

		ModelInfo info;
		WZM model;

		info.m_saveAsFile = argv[2];

		if (!MainWindow::loadModel(inname, model, info, true))
		{
			printf("Could not load model\n");
			return 1;
		}

		info.defaultPieCapsIfNeeded();

		if(!MainWindow::saveModel(model, info))
		{
			printf("Could not save model\n");
			return 1;
		}
	}
	else
	{
		QApplication a(argc, argv);

		a.setApplicationName(WMIT_APPNAME);
		a.setOrganizationName(WMIT_ORG);
		QSettings::setDefaultFormat(QSettings::IniFormat);

		QWZM model; // must be destructed *after* all of MainWindow's children to prevent a crash on exit (on some platforms)
		MainWindow w(model);
		w.show();

		if (argc == 2)
		{
			QString inname = argv[1];
			w.openFile(inname);
		}

		return a.exec();
	}
}
