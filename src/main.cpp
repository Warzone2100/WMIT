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
#include <QSettings>

#include <iostream>
#include <fstream>

#include <QStringList>

#include "MainWindow.h"
#include "WZM.h"
#include "Pie.h"
#include "wmit.h"

#if defined(Q_OS_WIN) && defined(QT_STATICPLUGIN)
#include <QtPlugin>
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
#endif

static bool pieVersionToFileType(int version, wmit_filetype_t& type)
{
	switch (version)
	{
	case 2: type = WMIT_FT_PIE2; return true;
	case 3: type = WMIT_FT_PIE; return true;
	case 4: type = WMIT_FT_PIE4; return true;
	default: return false;
	}
}

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

	if(argc == 2 && strcmp("--help", argv[1]) == 0)
	{
		printWelcomeBanner(true);

		printf("Usage:\n");
		printf("  <no parameters> (opens GUI application)\n");
		printf("  --help (shows this message)\n");
		printf("  [filename] (opens a file in GUI)\n");
		printf("  [input] [output] (converts between formats PIE and OBJ. Deprecated WZM format is supported as input.)\n");
		printf("  --pie-version=N (writes PIE version N, one of 2, 3 or 4. Defaults to the version of the input file.)\n");
		exit(0);
	}

	QStringList files;
	int forcedPieVersion = 0;

	for (int i = 1; i < argc; ++i)
	{
		const QString arg(argv[i]);

		if (arg.startsWith("--pie-version="))
		{
			bool ok = false;
			wmit_filetype_t unused;
			forcedPieVersion = arg.section('=', 1).toInt(&ok);
			if (!ok || !pieVersionToFileType(forcedPieVersion, unused))
			{
				std::cerr << "PIE version must be 2, 3 or 4." << std::endl;
				return 1;
			}
			continue;
		}

		files << arg;
	}

	if (files.size() > 1)
	{
		printWelcomeBanner(false);
		std::cout << "Converting files:" << std::endl;
		std::cout << "Input file \"" << files[0].toStdString() << '"' << std::endl;
		std::cout << "Output file \"" << files[1].toStdString() << '"' << std::endl;
		std::cout << std::endl;

		// command line conversion mode
		QString inname = files[0];

		ModelInfo info;
		WZM model;

		info.m_saveAsFile = files[1];
		if (!MainWindow::guessModelTypeFromFilename(info.m_saveAsFile, info.m_save_type))
		{
			std::cerr << "Could not guess save model type from filename. Only PIE and OBJ formats are supported!" << std::endl;
			return 1;
		}

		std::cout << "Loading model..." << std::endl;
		if (!MainWindow::loadModel(inname, model, info, true))
		{
			printf("Could not load model\n");
			return 1;
		}

		if (isPieFileType(info.m_save_type))
		{
			if (forcedPieVersion)
			{
				pieVersionToFileType(forcedPieVersion, info.m_save_type);
			}
			else if (isPieFileType(info.m_read_type))
			{
				// Keep the version the file came with.
				info.m_save_type = info.m_read_type;
			}

			const QString downgrade = MainWindow::describePieDowngrade(info);
			if (!downgrade.isEmpty())
			{
				std::cerr << downgrade.toStdString() << std::endl;
			}
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

		if (!files.isEmpty())
		{
			w.openFile(files[0]);
		}

		return a.exec();
	}
}
