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
#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>

#include <QList>
#include <QSet>
#include <QPair>

#include <QSettings>
#include <QSignalMapper>

#include "QWZM.hpp"
#include "wmit.h"

class TransformDock;
class ImportDialog;
class ExportDialog;
class TextureDialog;
class UVEditor;

namespace Ui
{
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

	void clear();

	bool openFile(const QString& file);

	static bool loadModel(const QString& file, WZM& model);
	static bool guessModelTypeFromFilename(const QString &fname, wmit_filetype_t &type);
	static bool saveModel(const QString& file, const WZM& model, const wmit_filetype_t &type);
	static bool saveModel(const QString& file, const QWZM& model, const wmit_filetype_t &type);
protected:
	void changeEvent(QEvent *event);

private slots:
	void actionOpen();
	void actionSave();
	void actionSaveAs();
	void actionClose();
	void actionSetupTextures();
	void actionAppendModel();
	void actionTakeScreenshot();

	void viewerInitialized();
	void shaderAction(int);

	// transformations
	void scaleXYZChanged(double scale);
	void scaleXChanged(double scale);
	void scaleYChanged(double scale);
	void scaleZChanged(double scale);
	void reverseWindings(int mesh);
	void mirrorAxis(int axis);
	void removeMesh(int mesh);

private:
	Ui::MainWindow* m_ui;

	ImportDialog* m_importDialog;
	ExportDialog* m_exportDialog;
	TransformDock* m_transformDock;

	TextureDialog* m_textureDialog;
	UVEditor* m_UVEditor;
	QSettings* m_settings;

	QSignalMapper *m_shaderSignalMapper;
	QString m_pathImport, m_pathExport, m_currentFile;

	QWZM m_model;

	bool fireTextureDialog(const bool reinit = false);
};

#endif // MAINWINDOW_HPP
