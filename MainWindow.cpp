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

#include "MainWindow.hpp"
#include "ui_MainWindow.h"
#include "ConfigDialog.hpp"
#include "TransformDock.hpp"
#include "ImportDialog.hpp"
#include "ExportDialog.hpp"
#include "UVEditor.hpp"

#include <fstream>

#include <QFileInfo>
#include <QFileDialog>
#include <QSettings>
#include <QtDebug>
#include <QVariant>

#include "Pie.hpp"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	configDialog(new ConfigDialog(this)),
	importDialog(new ImportDialog(this)),
	exportDialog(NULL),
	transformDock(new TransformDock(this)),
	m_UVEditor(new UVEditor(this))
{
	ui->setupUi(this);
	QSettings settings(QSettings::IniFormat, QSettings::UserScope,"WMIT", "WMIT");

	// A work around to add actions in the order we want
	ui->menuBar->clear();
	ui->menuBar->addMenu(ui->menuFile);
	ui->menuBar->addAction(ui->actionConfig);
	ui->menuBar->addMenu(ui->menuTextures);
	ui->menuBar->addMenu(ui->menuTeam_Colours);
	ui->menuBar->addAction(ui->actionTransformWidget);
//	ui->menuBar->addAction(ui->actionUVEditor);

	configDialog->hide();

	transformDock->setAllowedAreas(Qt::BottomDockWidgetArea|Qt::TopDockWidgetArea);
	transformDock->hide();
	this->addDockWidget(Qt::BottomDockWidgetArea, transformDock, Qt::Horizontal);

	m_UVEditor->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
	m_UVEditor->hide();
	this->addDockWidget(Qt::RightDockWidgetArea, m_UVEditor, Qt::Horizontal);

	connect(ui->centralWidget, SIGNAL(viewerInitialized()), this, SLOT(_on_viewerInitialized()));
	connect(importDialog, SIGNAL(accepted()), this, SLOT(s_fileOpen()));
	connect(this, SIGNAL(textureSearchDirsChanged(QStringList)), importDialog, SLOT(scanForTextures(QStringList)));
	connect(configDialog, SIGNAL(updateTextureSearchDirs(QList<QPair<bool,QString> >)), this, SLOT(s_updateTexSearchDirs(const QList<QPair<bool,QString> >&)));
	connect(this, SIGNAL(textureSearchDirsChanged(QStringList)), configDialog, SLOT(setTextureSearchDirs(QStringList)));

	// transformations
	connect(transformDock, SIGNAL(scaleXYZChanged(double)), this, SLOT(_on_scaleXYZChanged(double)));
	connect(transformDock, SIGNAL(scaleXChanged(double)), this, SLOT(_on_scaleXChanged(double)));
	connect(transformDock, SIGNAL(scaleYChanged(double)), this, SLOT(_on_scaleYChanged(double)));
	connect(transformDock, SIGNAL(scaleZChanged(double)), this, SLOT(_on_scaleZChanged(double)));
	connect(transformDock, SIGNAL(reverseWindings()), this, SLOT(_on_reverseWindings()));

	textureSearchDirs = QSet<QString>::fromList(settings.value("textureSearchDirs", QStringList()).toStringList());
	if (!textureSearchDirs.empty())
	{
		emit textureSearchDirsChanged(textureSearchDirs.toList());
	}
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
	switch (e->type())
	{
    case QEvent::LanguageChange:
		ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::s_fileOpen()
{
	QFileInfo modelFileNfo(importDialog->modelFilePath());
	std::ifstream f;
	if (!modelFileNfo.exists())
	{
		return;
	}

	if (modelFileNfo.completeSuffix().compare(QString("wzm"), Qt::CaseInsensitive) == 0)
	{
		f.open(modelFileNfo.absoluteFilePath().toLocal8Bit(), std::ios::in);
		model.read(f);
	}
	else if(modelFileNfo.completeSuffix().compare(QString("pie"), Qt::CaseInsensitive) == 0)
	{
		f.open(modelFileNfo.absoluteFilePath().toLocal8Bit(), std::ios::in);
		int version = pieVersion(f);
		if (version == 3)
		{
			Pie3Model p3;
			p3.read(f);
			model = WZM(p3);
		}
		else if (version == 2)
		{
			Pie2Model p2;
			p2.read(f);
			model = WZM(Pie3Model(p2));
		}
	}
	else if(modelFileNfo.completeSuffix().compare(QString("3ds"), Qt::CaseInsensitive) == 0)
	{
		model.importFrom3DS(std::string(modelFileNfo.absoluteFilePath().toLocal8Bit().constData()));
	}
	else if(modelFileNfo.completeSuffix().compare(QString("obj"), Qt::CaseInsensitive) == 0)
	{
		f.open(modelFileNfo.absoluteFilePath().toLocal8Bit(), std::ios::in);
		model.importFromOBJ(f);
	}

	model.setRenderTexture(importDialog->textureFilePath());
	setWindowTitle(QString("%1 - WMIT").arg(modelFileNfo.baseName()));

	if (importDialog->tcmaskChecked() && !importDialog->tcmaskFilePath().isEmpty())
	{
		model.setTCMaskTexture(importDialog->tcmaskFilePath());
		if (ui->centralWidget->tcmaskSupport() & FixedPipeline)
		{
			ui->actionFixed_Pipeline->setEnabled(true);
		}
		if (ui->centralWidget->tcmaskSupport() & Shaders)
		{
			ui->actionShaders->setEnabled(true);
		}

		if (ui->actionShaders->isEnabled())
		{
			ui->actionShaders->setChecked(true);
		}
		else if (ui->actionFixed_Pipeline->isEnabled())
		{
			ui->actionFixed_Pipeline->setChecked(true);
		}
	}
	else if (model.couldHaveTCArrays())
	{
		ui->actionTexture_Frames->setEnabled(true);
	}
}

void MainWindow::s_updateTexSearchDirs(const QList<QPair<bool,QString> >& changes)
{
	bool changed = false;
	QSettings settings(QSettings::IniFormat, QSettings::UserScope,"WMIT", "WMIT");
	typedef QPair<bool,QString> t_change; // Work around foreach macro 3 argument error
	foreach (t_change change, changes)
	{
		if (change.first) // adding
		{
			if (!textureSearchDirs.contains(change.second))
			{
				textureSearchDirs.insert(change.second);
				if (!changed)
				{
					changed = true;
				}
			}
		}
		else // removing
		{
			if (textureSearchDirs.remove(change.second) && !changed)
			{
				changed = true;
			}
		}
	}
	if (changed)
	{
		emit textureSearchDirsChanged(textureSearchDirs.toList());
	}

	settings.setValue("textureSearchDirs", QVariant(textureSearchDirs.toList()));
}

void MainWindow::on_actionConfig_triggered()
{
	configDialog->show();
}

void MainWindow::on_actionTransformWidget_toggled(bool show)
{
	show ? transformDock->show() : transformDock->hide();
}

void MainWindow::on_actionOpen_triggered()
{
	importDialog->show();
}

void MainWindow::on_actionUVEditor_toggled(bool show)
{
	show? m_UVEditor->show() : m_UVEditor->hide();
}

void MainWindow::on_actionSave_triggered()
{
//todo
}

void MainWindow::on_actionSave_As_triggered()
{
	QFileDialog* fDialog = new QFileDialog();
	std::ofstream out;
	QFileInfo nfo;

	enum { WZM = 0, PIE, _3DS, OBJ, NONE} type;

	fDialog->setFileMode(QFileDialog::AnyFile);
	fDialog->setFilter("All Compatible (*.wzm *.pie *.3ds *.obj);;"
					  "WZM models (*.wzm);;"
					  "PIE models (*.pie);;"
					  "3DS files (*.3ds);;"
					  "OBJ files (*.obj)");
	fDialog->setWindowTitle(tr("Choose output file"));
	fDialog->selectFilter("WZM models (*.wzm)");
	fDialog->exec();

	if (fDialog->result() != QDialog::Accepted)
	{
		return;
	}

	nfo.setFile(fDialog->selectedFiles().first());

	if (nfo.completeSuffix().compare(QString("wzm"), Qt::CaseInsensitive) == 0)
	{
		type = WZM;
	}
	else if (nfo.completeSuffix().compare(QString("pie"), Qt::CaseInsensitive) == 0)
	{
		type = PIE;
	}
	else if (nfo.completeSuffix().compare(QString("3ds"), Qt::CaseInsensitive) == 0)
	{
		type = _3DS;
	}
	else if (nfo.completeSuffix().compare(QString("obj"), Qt::CaseInsensitive) == 0)
	{
		type = OBJ;
	}
	else
	{
		type = NONE;
	}

	if (type == PIE)
	{
		exportDialog = new PieExportDialog(this);
		exportDialog->exec();
	}
	else
	{
		exportDialog = new ExportDialog(this);
		exportDialog->exec();
	}

	if (exportDialog->result() != QDialog::Accepted)
	{
		return;
	}

	if (exportDialog->optimisationSelected() == 0)
	{
//		model.optimizeForsyth();
	}
	delete exportDialog;
	exportDialog = NULL;

	if (type == WZM)
	{
		out.open(nfo.absoluteFilePath().toLocal8Bit().constData());
		model.write(out);
	}
	else if(type == PIE)
	{
		out.open(nfo.absoluteFilePath().toLocal8Bit().constData());
		Pie3Model p3 = model;
		p3.write(out);
	}
	else if(type == _3DS)
	{
		model.exportTo3DS(std::string(nfo.absoluteFilePath().toLocal8Bit().constData()));
	}
	else if(type == OBJ)
	{
		out.open(nfo.absoluteFilePath().toLocal8Bit().constData());
		model.exportToOBJ(out);
	}
}

void MainWindow::_on_viewerInitialized()
{
	ui->centralWidget->addToRenderList(&model);
}

void MainWindow::_on_scaleXYZChanged(double val)
{
	model.setScaleXYZ(val);
	ui->centralWidget->updateGL();
}

void MainWindow::_on_scaleXChanged(double val)
{
	model.setScaleX(val);
	ui->centralWidget->updateGL();
}

void MainWindow::_on_scaleYChanged(double val)
{
	model.setScaleY(val);
	ui->centralWidget->updateGL();
}

void MainWindow::_on_scaleZChanged(double val)
{
	model.setScaleZ(val);
	ui->centralWidget->updateGL();
}

void MainWindow::_on_reverseWindings()
{
	model.reverseWindings();
	ui->centralWidget->updateGL();
}

void MainWindow::on_actionFixed_Pipeline_toggled(bool checked)
{
	if (checked)
	{
		ui->actionShaders->setChecked(false);
		ui->centralWidget->setTCMaskMode(FixedPipeline);
	}
	else
	{
		if (!ui->actionShaders->isChecked())
		{
			ui->centralWidget->setTCMaskMode(None);
		}
	}
}

void MainWindow::on_actionShaders_toggled(bool checked)
{
	if (checked)
	{
		ui->actionFixed_Pipeline->setChecked(false);
		ui->centralWidget->setTCMaskMode(Shaders);
	}
	else
	{
		if (!ui->actionFixed_Pipeline->isChecked())
		{
			ui->centralWidget->setTCMaskMode(None);
		}
	}
}
