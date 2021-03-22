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

#include <iostream>
#include <fstream>

#include "MainWindow.h"
#include "WZM.h"
#include "Pie.h"
#include "wmit.h"

#if defined(Q_OS_WIN) && defined(QT_STATICPLUGIN)
#include <QtPlugin>
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
#endif

void printWelcomeBanner(const bool printLicense)
{
	std::cout << "Welcome to " WMIT_APPNAME " " WMIT_VER_STR << std::endl;

	if (printLicense)
	{
		std::cout << std::endl <<
		"Copyright (C) 2010-2021 Warzone 2100 Project" << std::endl <<
		"This program comes with ABSOLUTELY NO WARRANTY;" << std::endl <<
		"This is free software, and you are welcome to redistribute it" << std::endl <<
		"under certain conditions; see About in graphical UI for details." << std::endl;
	}

	std::cout << std::endl;
}

int main(int argc, char *argv[])
{
    //QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());

	if(argc == 2 && strcmp("--help", argv[1]) == 0)
	{
		printWelcomeBanner(true);

		printf("Usage:\n");
		printf("  <no parameters> (opens GUI application)\n");
		printf("  --help (shows this message)\n");
		printf("  [filename] (opens a file in GUI)\n");
		printf("  [input] [output] (converts between formats PIE and OBJ. Deprecated WZM format is supported as input.)\n");
		exit(0);
	}

	if (argc > 2)
	{
		printWelcomeBanner(false);
		std::cout << "Converting files:" << std::endl;
		std::cout << "Input file \"" << argv[1] << '"' << std::endl;
		std::cout << "Output file \"" << argv[2] << '"' << std::endl;
		std::cout << std::endl;

		// command line conversion mode
		QString inname = argv[1];

		ModelInfo info;
		WZM model;

		info.m_saveAsFile = argv[2];
		if (!MainWindow::guessModelTypeFromFilename(info.m_saveAsFile, info.m_save_type))
		{
			std::cerr << "Could not guess save model type from filename. Only PIE3, WZM, and OBJ formats are supported!" << std::endl;
			return 1;
		}

		if (info.m_save_type == WMIT_FT_WZM)
		{
			std::cerr << WMIT_WARN_DEPRECATED_WZM << std::endl;
			return 1;
		}

		std::cout << "Loading model..." << std::endl;
		if (!MainWindow::loadModel(inname, model, info, true))
		{
			printf("Could not load model\n");
			return 1;
		}

		if (info.m_read_type == WMIT_FT_WZM)
		{
			std::cout << WMIT_WARN_DEPRECATED_WZM << std::endl;
		}

		info.defaultPieCapsIfNeeded();

		std::cout << "Saving model..." << std::endl;
		if(!MainWindow::saveModel(model, info))
		{
			printf("Could not save model\n");
			return 1;
		}

		std::cout << "Done." << std::endl;
		return 0;
	}
	else
	{
		QApplication a(argc, argv);

		a.setWindowIcon(QIcon(WMIT_IMAGES_LOGO_64));
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
