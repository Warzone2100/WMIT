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

#include "QWZM.hpp"

class TransformDock;
class ConfigDialog;
class ImportDialog;
class ExportDialog;
class UVEditor;

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
	~MainWindow();

protected:
    void changeEvent(QEvent *e);

signals:
	void textureSearchDirsChanged(QStringList);

private slots:
	void on_actionShaders_toggled(bool );
 void on_actionFixed_Pipeline_toggled(bool );
 void on_actionSave_As_triggered();
	void on_actionSave_triggered();
	void s_fileOpen();
	void s_updateTexSearchDirs(const QList<QPair<bool,QString> >&);
	void on_actionUVEditor_toggled(bool );
	void on_actionOpen_triggered();
	void on_actionConfig_triggered();
	void on_actionTransformWidget_toggled(bool );

	void _on_viewerInitialized();

	// transformations
	void _on_scaleXYZChanged(double);
	void _on_scaleXChanged(double);
	void _on_scaleYChanged(double);
	void _on_scaleZChanged(double);
	void _on_reverseWindings();

private:
	Ui::MainWindow* ui;
	ConfigDialog* configDialog;
	ImportDialog* importDialog;
	ExportDialog* exportDialog;
	TransformDock* transformDock;
	UVEditor* m_UVEditor;

	QSet<QString> textureSearchDirs;

	QWZM model;
};

#endif // MAINWINDOW_HPP
