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

#include <QString>
#include <QList>
#include <QSet>
#include <QPair>

#include <QSettings>

#include "QWZM.hpp"

class TransformDock;
class ImportDialog;
class ExportDialog;
class TextureDialog;
class UVEditor;

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow {
	Q_OBJECT
public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

	void clear();

	void loadModel(const QString& file);

protected:
	void changeEvent(QEvent *e);

signals:
	void textureSearchDirsChanged(QStringList);

private slots:
	void on_actionShaders_toggled(bool );
	void on_actionFixed_Pipeline_toggled(bool );
	void on_actionOpen_triggered();
	void on_actionSave_As_triggered();
	void on_actionSave_triggered();
	void on_actionClose_triggered();
	void on_actionTransform_triggered();
	void on_actionUVEditor_toggled(bool );

	void _on_viewerInitialized();

	// transformations
	void _on_scaleXYZChanged(double);
	void _on_scaleXChanged(double);
	void _on_scaleYChanged(double);
	void _on_scaleZChanged(double);
	void _on_reverseWindings(int mesh);
	void _on_mirrorAxis(int axis);

	void on_actionSetupTextures_triggered();

private:
	Ui::MainWindow* ui;

	ImportDialog* importDialog;
	ExportDialog* exportDialog;
	TransformDock* transformDock;

	TextureDialog* m_textureDialog;
	UVEditor* m_UVEditor;
	QSettings* m_settings;

	QString m_pathImport, m_pathExport, m_currentFile;

	QWZM model;

	bool fireTexConfigDialog(const bool reinit = false);
};

#endif // MAINWINDOW_HPP
