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

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	importDialog(new ImportDialog(this)),
	exportDialog(NULL),
	transformDock(new TransformDock(this)),
	m_textureDialog(new TextureDialog(this)),
	m_UVEditor(new UVEditor(this)),
	m_settings(new QSettings(this)),
	m_shaderSignalMapper(new QSignalMapper(this))
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
	connect(ui->actionAboutQt, SIGNAL(triggered()), QApplication::instance(), SLOT(aboutQt()));

	// transformations
	connect(transformDock, SIGNAL(scaleXYZChanged(double)), this, SLOT(_on_scaleXYZChanged(double)));
	connect(transformDock, SIGNAL(scaleXChanged(double)), this, SLOT(_on_scaleXChanged(double)));
	connect(transformDock, SIGNAL(scaleYChanged(double)), this, SLOT(_on_scaleYChanged(double)));
	connect(transformDock, SIGNAL(scaleZChanged(double)), this, SLOT(_on_scaleZChanged(double)));
	connect(transformDock, SIGNAL(reverseWindings(int)), this, SLOT(_on_reverseWindings(int)));
	connect(transformDock, SIGNAL(applyTransformations()), &m_model, SLOT(applyTransformations()));
	connect(&m_model, SIGNAL(meshCountChanged(int,QStringList)), transformDock, SLOT(setMeshCount(int,QStringList)));
	connect(transformDock, SIGNAL(setActiveMeshIdx(int)), &m_model, SLOT(setActiveMesh(int)));
	connect(transformDock, SIGNAL(removeMeshIdx(int)), this, SLOT(_on_removeMesh(int)));
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
	m_model.clear();
	m_currentFile.clear();

	setWindowTitle(WMIT_APPNAME);

	ui->actionClose->setDisabled(true);
	ui->actionSaveAs->setDisabled(true);
	ui->actionSetupTextures->setDisabled(true);
	ui->actionAppendModel->setDisabled(true);
}

bool MainWindow::guessModelTypeFromFilename(const QString& fname, wmit_filetype_t& type)
{
	QString ext = fname.right(fname.size() - fname.lastIndexOf('.') - 1);

	if (ext.compare(QString("wzm"), Qt::CaseInsensitive) == 0)
	{
		type = WMIT_FT_WZM;
	}
	else if (ext.compare(QString("obj"), Qt::CaseInsensitive) == 0)
	{
		type = WMIT_FT_OBJ;
	}
	else if (ext.compare(QString("pie"), Qt::CaseInsensitive) == 0)
	{
		type = WMIT_FT_PIE;
	}
	else
	{
		return false;
	}

	return true;
}

bool MainWindow::saveModel(const QString &file, const WZM &model, const wmit_filetype_t &type)
{
	std::ofstream out;
	out.open(file.toLocal8Bit().constData());

	switch (type)
	{
	case WMIT_FT_WZM:
		model.write(out);
		break;
	case WMIT_FT_OBJ:
		model.exportToOBJ(out);
		break;
	case WMIT_FT_PIE:
	default:
		Pie3Model p3 = model;
		p3.write(out);
	}

	out.close();

	return true;
}

bool MainWindow::saveModel(const QString &file, const QWZM &model, const wmit_filetype_t &type)
{
	std::ofstream out;
	out.open(file.toLocal8Bit().constData());

	switch (type)
	{
	case WMIT_FT_WZM:
		model.write(out);
		break;
	case WMIT_FT_OBJ:
		model.exportToOBJ(out);
		break;
	case WMIT_FT_PIE:
	default:
		Pie3Model p3 = model;
		p3.write(out);
	}

	out.close();

	return true;
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

bool MainWindow::loadModel(const QString& file, WZM& model)
{
	wmit_filetype_t type;

	if (!guessModelTypeFromFilename(file, type))
	{
		return false;
	}

	bool read_success = false;
	std::ifstream f;

	f.open(file.toLocal8Bit(), std::ios::in | std::ios::binary);

	switch (type)
	{
	case WMIT_FT_WZM:
		read_success = model.read(f);
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

	f.close();

	return read_success;
}

bool MainWindow::fireTextureDialog(const bool reinit)
{
	QMap<wzm_texture_type_t, QString> texmap;

	if (reinit)
	{
		m_model.getTexturesMap(texmap);
		m_textureDialog->setTexturesMap(texmap);
		m_textureDialog->createTextureIcons(m_pathImport, m_currentFile);
	}

	if (m_textureDialog->exec() == QDialog::Accepted)
	{
		QMap<wzm_texture_type_t, QString>::const_iterator it;

		m_model.clearTextureNames();
		m_model.clearGLRenderTextures();

		m_textureDialog->getTexturesFilepath(texmap);
		for (it = texmap.begin(); it != texmap.end(); ++it)
		{
			if (!it.value().isEmpty())
			{
				QFileInfo texFileNfo(it.value());
				m_model.loadGLRenderTexture(it.key(), texFileNfo.filePath());
				m_model.setTextureName(it.key(), texFileNfo.fileName().toStdString());
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
						  tr("All Compatible (*.wzm *.pie *.obj);;"
						     "WZM models (*.wzm);;"
						     "PIE models (*.pie);;"
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
	{
		WZM tmpmodel;

		if (loadModel(filePath, tmpmodel))
		{
			QFileInfo modelFileNfo(filePath);
			m_model = tmpmodel;
			m_currentFile = modelFileNfo.absoluteFilePath();

			setWindowTitle(QString("%1 - WMIT").arg(modelFileNfo.baseName()));
			ui->actionClose->setEnabled(true);
			ui->actionSaveAs->setEnabled(true);
			ui->actionSetupTextures->setEnabled(true);
			ui->actionAppendModel->setEnabled(true);

			if (!fireTextureDialog(true))
			{
				clear();
				return;
			}
		}
		// else inf popup on fail?
	}
}

void MainWindow::on_actionUVEditor_toggled(bool show)
{
	show? m_UVEditor->show() : m_UVEditor->hide();
}

void MainWindow::on_actionSave_triggered()
{
//todo
}

void MainWindow::on_actionSaveAs_triggered()
{
	QFileDialog* fDialog = new QFileDialog();

	fDialog->setFileMode(QFileDialog::AnyFile);
	fDialog->setAcceptMode(QFileDialog::AcceptSave);
	fDialog->setFilter("PIE models (*.pie);;"
			   "WZM models (*.wzm);;"
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

	wmit_filetype_t type;
	if (!guessModelTypeFromFilename(fDialog->selectedFiles().first(), type))
	{
		return;
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

	saveModel(fDialog->selectedFiles().first(), m_model, type);
}

void MainWindow::_on_viewerInitialized()
{
	ui->centralWidget->addToRenderList(&m_model);

	QActionGroup* shaderGroup = new QActionGroup(this);

	for (int i = WZ_SHADER__FIRST; i < WZ_SHADER__LAST; ++i)
	{
		QString shadername = QWZM::shaderTypeToString(static_cast<wz_shader_type_t>(i));

		QAction* shaderAct = new QAction(shadername, this);

		shaderAct->setCheckable(true);
		shaderAct->setEnabled(false);

		//FIXME: more automated way required
		{
			QString pathvert, pathfrag;
			switch (static_cast<wz_shader_type_t>(i))
			{
			case WZ_SHADER_NONE:
				shaderAct->setEnabled(true);
				break;
			case WZ_SHADER_PIE3:
				pathvert = WMIT_SHADER_PIE3_DEFPATH_VERT;
				pathfrag = WMIT_SHADER_PIE3_DEFPATH_FRAG;
				break;
			case WZ_SHADER_PIE3_USER:
				pathvert = WMIT_SHADER_PIE3_USERFILE_VERT;
				pathfrag = WMIT_SHADER_PIE3_USERFILE_FRAG;
				break;
			default:
				break;
			}

			QFileInfo finfo(pathvert);
			if (finfo.exists())
			{
				finfo.setFile(pathfrag);
				if (finfo.exists())
				{
					if (ui->centralWidget->loadShader(static_cast<wz_shader_type_t>(i), pathvert, pathfrag))
					{
						shaderAct->setEnabled(true);
					}
				}
			}
		}

		m_shaderSignalMapper->setMapping(shaderAct, i);
		connect(shaderAct, SIGNAL(triggered()), m_shaderSignalMapper, SLOT(map()));

		shaderAct->setActionGroup(shaderGroup);
	}

	connect(m_shaderSignalMapper, SIGNAL(mapped(int)),
		     this, SLOT(_on_shaderActionTriggered(int)));

	QMenu* rendererMenu = new QMenu(this);
	rendererMenu->addActions(shaderGroup->actions());

	for (int i = shaderGroup->actions().size() - 1; i >= 0; --i)
	{
		if (shaderGroup->actions().at(i)->isEnabled())
		{
			shaderGroup->actions().at(i)->activate(QAction::Trigger);
			break;
		}
	}

	ui->actionRenderer->setMenu(rendererMenu);

	connect(ui->actionShowModelCenter, SIGNAL(triggered(bool)),
		&m_model, SLOT(setDrawCenterPointFlag(bool)));
	connect(ui->actionShowNormals, SIGNAL(triggered(bool)),
		&m_model, SLOT(setDrawNormalsFlag(bool)));
}

void MainWindow::_on_shaderActionTriggered(int type)
{
	if (static_cast<wz_shader_type_t>(type) != WZ_SHADER_NONE)
	{
		m_model.setActiveShader(static_cast<wz_shader_type_t>(type));
	}
	else
	{
		m_model.disableShaders();
	}
}

void MainWindow::_on_scaleXYZChanged(double val)
{
	m_model.setScaleXYZ(val);
	ui->centralWidget->updateGL();
}

void MainWindow::_on_scaleXChanged(double val)
{
	m_model.setScaleX(val);
	ui->centralWidget->updateGL();
}

void MainWindow::_on_scaleYChanged(double val)
{
	m_model.setScaleY(val);
	ui->centralWidget->updateGL();
}

void MainWindow::_on_scaleZChanged(double val)
{
	m_model.setScaleZ(val);
	ui->centralWidget->updateGL();
}

void MainWindow::_on_reverseWindings(int mesh)
{
	m_model.reverseWinding(mesh);
	ui->centralWidget->updateGL();
}

void MainWindow::_on_mirrorAxis(int axis)
{
	m_model.slotMirrorAxis(axis);
	ui->centralWidget->updateGL();
}

void MainWindow::_on_removeMesh(int mesh)
{
	if (mesh < 0 || mesh > m_model.meshes())
		return;

	m_model.rmMesh(mesh);
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

void MainWindow::on_actionAppendModel_triggered()
{
	QString filePath;
	QFileDialog* fileDialog = new QFileDialog(this,
						  tr("Select file to append"),
						  m_pathImport,
						  tr("All Compatible (*.wzm *.pie *.obj);;"
						     "WZM models (*.wzm);;"
						     "PIE models (*.pie);;"
						     "OBJ files (*.obj)"));
	fileDialog->setFileMode(QFileDialog::ExistingFile);
	fileDialog->exec();

	if (fileDialog->result() == QDialog::Accepted)
	{
		filePath = fileDialog->selectedFiles().first();
	}
	delete fileDialog;
	fileDialog = 0;

	if (!filePath.isEmpty())
	{
		WZM newmodel;

		if (loadModel(filePath, newmodel))
		{
			for (int i = 0; i < newmodel.meshes(); ++i)
			{
				m_model.addMesh(newmodel.getMesh(i));
			}
		}
	}
}
