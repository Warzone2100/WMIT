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
#include "TransformDock.hpp"
#include "ImportDialog.hpp"
#include "ExportDialog.hpp"
#include "TextureDialog.h"
#include "UVEditor.hpp"

#include <fstream>

#include <QFileInfo>
#include <QFileDialog>
#include <QDir>

#include <QtDebug>
#include <QVariant>

#include "Pie.hpp"
#include "wmit.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	importDialog(new ImportDialog(this)),
	exportDialog(NULL),
	transformDock(new TransformDock(this)),
	m_textureDialog(new TextureDialog(this)),
	m_UVEditor(new UVEditor(this)),
	m_settings(new QSettings(this))
{
	ui->setupUi(this);

	m_pathImport = m_settings->value(WMIT_SETTINGS_IMPORTVAL, QDir::currentPath()).toString();
	m_pathExport = m_settings->value(WMIT_SETTINGS_EXPORTVAL, QDir::currentPath()).toString();

	transformDock->setAllowedAreas(Qt::RightDockWidgetArea);
	transformDock->hide();
	this->addDockWidget(Qt::RightDockWidgetArea, transformDock, Qt::Horizontal);

	m_UVEditor->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);
	m_UVEditor->hide();
	this->addDockWidget(Qt::LeftDockWidgetArea, m_UVEditor, Qt::Horizontal);

	connect(ui->centralWidget, SIGNAL(viewerInitialized()), this, SLOT(_on_viewerInitialized()));

	// transformations
	connect(transformDock, SIGNAL(scaleXYZChanged(double)), this, SLOT(_on_scaleXYZChanged(double)));
	connect(transformDock, SIGNAL(scaleXChanged(double)), this, SLOT(_on_scaleXChanged(double)));
	connect(transformDock, SIGNAL(scaleYChanged(double)), this, SLOT(_on_scaleYChanged(double)));
	connect(transformDock, SIGNAL(scaleZChanged(double)), this, SLOT(_on_scaleZChanged(double)));
	connect(transformDock, SIGNAL(reverseWindings(int)), this, SLOT(_on_reverseWindings(int)));
	connect(transformDock, SIGNAL(applyTransformations()), &model, SLOT(applyTransformations()));
	connect(&model, SIGNAL(meshCountChanged(int,QStringList)), transformDock, SLOT(setMeshCount(int,QStringList)));
	connect(transformDock, SIGNAL(setActiveMeshIdx(int)), &model, SLOT(setActiveMesh(int)));
	connect(transformDock, SIGNAL(mirrorAxis(int)), this, SLOT(_on_mirrorAxis(int)));

	clear();

	// disable wip-parts
	ui->actionSave->setDisabled(true);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::clear()
{
	model.clear();
	m_currentFile.clear();

	setWindowTitle(WMIT_APPNAME);

	ui->actionClose->setDisabled(true);
	ui->actionSetupTextures->setDisabled(true);
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

void MainWindow::loadModel(const QString& file)
{
	QFileInfo modelFileNfo(file);
	bool read_success = false;
	std::ifstream f;
	wmit_filetype_t type;

	if (!modelFileNfo.exists())
	{
		return;
	}

	if (modelFileNfo.completeSuffix().compare(QString("wzm"), Qt::CaseInsensitive) == 0)
	{
		type = WMIT_FT_WZM;
	}
	else if (modelFileNfo.completeSuffix().compare(QString("3ds"), Qt::CaseInsensitive) == 0)
	{
		type = WMIT_FT_3DS;
	}
	else if (modelFileNfo.completeSuffix().compare(QString("obj"), Qt::CaseInsensitive) == 0)
	{
		type = WMIT_FT_OBJ;
	}
	else
	{
		type = WMIT_FT_PIE;
	}

	f.open(modelFileNfo.absoluteFilePath().toLocal8Bit(), std::ios::in | std::ios::binary);

	switch (type)
	{
	case WMIT_FT_WZM:
		read_success = model.read(f);
		break;
	case WMIT_FT_3DS:
		read_success = model.importFrom3DS(std::string(modelFileNfo.absoluteFilePath().toLocal8Bit().constData()));
		break;
	case WMIT_FT_OBJ:
		read_success = model.importFromOBJ(f);
		break;
	case WMIT_FT_PIE:
	default:
		int pieversion = pieVersion(f);
		if (pieversion <= 2)
		{
			Pie2Model p2;
			read_success = p2.read(f);
			if (read_success)
				model = WZM(Pie3Model(p2));
		}
		else // 3 or higher
		{
			Pie3Model p3;
			read_success = p3.read(f);
			if (read_success)
				model = WZM(p3);
		}
	}

	// Bail out on error, clear app state
	if (!read_success)
	{
		clear();
		return;
	}

	m_currentFile = modelFileNfo.absoluteFilePath();

	setWindowTitle(QString("%1 - WMIT").arg(modelFileNfo.baseName()));
	ui->actionClose->setEnabled(true);
	ui->actionSetupTextures->setEnabled(true);

	if (!fireTextureDialog(true))
	{
		clear();
		return;
	}
}

bool MainWindow::fireTextureDialog(const bool reinit)
{
	QMap<wzm_texture_type_t, QString> texmap;

	if (reinit)
	{
		model.getTexturesMap(texmap);
		m_textureDialog->setTexturesMap(texmap);
		m_textureDialog->createTextureIcons(m_pathImport, m_currentFile);
	}

	if (m_textureDialog->exec() == QDialog::Accepted)
	{
		QMap<wzm_texture_type_t, QString>::const_iterator it;

		model.clearTextureNames();
		model.clearGLRenderTextures();

		m_textureDialog->getTexturesFilepath(texmap);
		for (it = texmap.begin(); it != texmap.end(); ++it)
		{
			if (!it.value().isEmpty())
			{
				QFileInfo texFileNfo(it.value());
				model.loadGLRenderTexture(it.key(), texFileNfo.filePath());
				model.setTextureName(it.key(), texFileNfo.fileName().toStdString());
			}
		}

		return true;
	}

	return false;
}

void MainWindow::on_actionOpen_triggered()
{
	QString filePath;
	QFileDialog* fileDialog = new QFileDialog(this,
						  tr("Select File to open"),
						  m_pathImport,
						  tr("All Compatible (*.pie *.3ds *.obj);;"
						     /// "WZM models (*.wzm);;"
						     "PIE models (*.pie);;"
						     "3DS files (*.3ds);;"
						     "OBJ files (*.obj)"));
	fileDialog->setFileMode(QFileDialog::ExistingFile);
	fileDialog->exec();

	if (fileDialog->result() == QDialog::Accepted)
	{
		filePath = fileDialog->selectedFiles().first();

		// refresh import working dir
		m_pathImport = fileDialog->directory().absolutePath();
		m_settings->setValue(WMIT_SETTINGS_IMPORTVAL, m_pathImport);
	}
	delete fileDialog;
	fileDialog = 0;

	if (!filePath.isEmpty())
		loadModel(filePath);
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

	wmit_filetype_t type;

	fDialog->setFileMode(QFileDialog::AnyFile);
	fDialog->setAcceptMode(QFileDialog::AcceptSave);
	fDialog->setFilter("PIE models (*.pie);;"
			   "WZM models (*.wzm);;"
			   "3DS files (*.3ds);;"
			   "OBJ files (*.obj)");
	fDialog->setWindowTitle(tr("Choose output file"));
	fDialog->setDefaultSuffix("pie");
	fDialog->setDirectory(m_pathExport);
	fDialog->exec();

	if (fDialog->result() != QDialog::Accepted)
	{
		return;
	}

	// refresh export working dir
	m_pathExport = fDialog->directory().absolutePath();
	m_settings->setValue(WMIT_SETTINGS_EXPORTVAL, m_pathExport);

	nfo.setFile(fDialog->selectedFiles().first());

	if (nfo.completeSuffix().compare(QString("wzm"), Qt::CaseInsensitive) == 0)
	{
		type = WMIT_FT_WZM;
	}
	else if (nfo.completeSuffix().compare(QString("3ds"), Qt::CaseInsensitive) == 0)
	{
		type = WMIT_FT_3DS;
	}
	else if (nfo.completeSuffix().compare(QString("obj"), Qt::CaseInsensitive) == 0)
	{
		type = WMIT_FT_OBJ;
	}
	else
	{
		type = WMIT_FT_PIE;
	}

/* Disabled till ready
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
*/

	if (type == WMIT_FT_WZM)
	{
		out.open(nfo.absoluteFilePath().toLocal8Bit().constData());
		model.write(out);
	}
	else if(type == WMIT_FT_3DS)
	{
		model.exportTo3DS(std::string(nfo.absoluteFilePath().toLocal8Bit().constData()));
	}
	else if(type == WMIT_FT_OBJ)
	{
		out.open(nfo.absoluteFilePath().toLocal8Bit().constData());
		model.exportToOBJ(out);
	}
	else //if(type == WMIT_FT_PIE)
	{
		out.open(nfo.absoluteFilePath().toLocal8Bit().constData());
		Pie3Model p3 = model;
		p3.write(out);
	}
}

void MainWindow::_on_viewerInitialized()
{
	ui->centralWidget->addToRenderList(&model);

	// Only do it AFTER model was set to new render context
	if (ui->centralWidget->loadShader(WZ_SHADER_PIE3, WMIT_SHADER_PIE3_DEFPATH_VERT, WMIT_SHADER_PIE3_DEFPATH_FRAG))
	{
		model.setActiveShader(WZ_SHADER_PIE3);
	}

	// FIXME: check for frag file too
	QFileInfo finfo("./pie3.vert");
	if (finfo.exists() && ui->centralWidget->loadShader(WZ_SHADER_PIE3_USER, "./pie3.vert", "./pie3.frag"))
	{
		model.setActiveShader(WZ_SHADER_PIE3_USER);
	}
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

void MainWindow::_on_reverseWindings(int mesh)
{
	model.reverseWinding(mesh);
	ui->centralWidget->updateGL();
}

void MainWindow::_on_mirrorAxis(int axis)
{
	model.slotMirrorAxis(axis);
	ui->centralWidget->updateGL();
}

void MainWindow::on_actionClose_triggered()
{
	clear();
}

void MainWindow::on_actionTransform_triggered()
{
	transformDock->isVisible() ? transformDock->hide() : transformDock->show();
}

void MainWindow::on_actionSetupTextures_triggered()
{
	fireTextureDialog();
}
