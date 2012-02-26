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

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "MaterialDock.h"
#include "TransformDock.h"
#include "ImportDialog.h"
#include "ExportDialog.h"
#include "TextureDialog.h"
#include "UVEditor.h"

#include <fstream>

#include <QFileInfo>
#include <QFileDialog>
#include <QDir>

#include <QtDebug>
#include <QVariant>

#include "Pie.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
	m_ui(new Ui::MainWindow),
	m_importDialog(new ImportDialog(this)),
	m_exportDialog(NULL),
	m_materialDock(new MaterialDock(this)),
	m_transformDock(new TransformDock(this)),
	m_textureDialog(new TextureDialog(this)),
	m_UVEditor(new UVEditor(this)),
	m_settings(new QSettings(this)),
	m_shaderSignalMapper(new QSignalMapper(this))
{
	m_ui->setupUi(this);

	resize(QSettings().value("Window/size", size()).toSize());
	move(QSettings().value("Window/position", pos()).toPoint());
	restoreState(QSettings().value("Window/state", QByteArray()).toByteArray());

	m_pathImport = m_settings->value(WMIT_SETTINGS_IMPORTVAL, QDir::currentPath()).toString();
	m_pathExport = m_settings->value(WMIT_SETTINGS_EXPORTVAL, QDir::currentPath()).toString();

	m_materialDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	m_materialDock->hide();

	m_transformDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	m_transformDock->hide();

	m_UVEditor->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	m_UVEditor->hide();

	addDockWidget(Qt::RightDockWidgetArea, m_materialDock, Qt::Horizontal);
	addDockWidget(Qt::RightDockWidgetArea, m_transformDock, Qt::Horizontal);
	addDockWidget(Qt::LeftDockWidgetArea, m_UVEditor, Qt::Horizontal);

	m_ui->actionOpen->setIcon(QIcon::fromTheme("document-open", style()->standardIcon(QStyle::SP_DirOpenIcon)));
	m_ui->menuOpenRecent->setIcon(QIcon::fromTheme("document-open-recent"));
	m_ui->actionClearRecentFiles->setIcon(QIcon::fromTheme("edit-clear-list"));
	m_ui->actionSave->setIcon(QIcon::fromTheme("document-save", style()->standardIcon(QStyle::SP_DialogSaveButton)));
	m_ui->actionSaveAs->setIcon(QIcon::fromTheme("document-save-as"));
	m_ui->actionClose->setIcon(QIcon::fromTheme("window-close"));
	m_ui->actionExit->setIcon(QIcon::fromTheme("application-exit", style()->standardIcon(QStyle::SP_DialogCloseButton)));
	m_ui->actionAboutApplication->setIcon(QIcon::fromTheme("help-about"));

	connect(m_ui->centralWidget, SIGNAL(viewerInitialized()), this, SLOT(viewerInitialized()));
	connect(m_ui->menuFile, SIGNAL(aboutToShow()), this, SLOT(updateRecentFilesMenu()));
	connect(m_ui->actionOpen, SIGNAL(triggered()), this, SLOT(actionOpen()));
	connect(m_ui->menuOpenRecent, SIGNAL(triggered(QAction*)), this, SLOT(actionOpenRecent(QAction*)));
	connect(m_ui->actionClearRecentFiles, SIGNAL(triggered()), this, SLOT(actionClearRecentFiles()));
	connect(m_ui->actionSave, SIGNAL(triggered()), this, SLOT(actionSave()));
	connect(m_ui->actionSaveAs, SIGNAL(triggered()), this, SLOT(actionSaveAs()));
	connect(m_ui->actionClose, SIGNAL(triggered()), this, SLOT(actionClose()));
	connect(m_ui->actionMaterial, SIGNAL(toggled(bool)), m_materialDock, SLOT(setVisible(bool)));
	connect(m_ui->actionTransform, SIGNAL(toggled(bool)), m_transformDock, SLOT(setVisible(bool)));
	connect(m_ui->actionUVEditor, SIGNAL(toggled(bool)), m_UVEditor, SLOT(setVisible(bool)));
	connect(m_ui->actionSetupTextures, SIGNAL(triggered()), this, SLOT(actionSetupTextures()));
	connect(m_ui->actionAppendModel, SIGNAL(triggered()), this, SLOT(actionAppendModel()));
	connect(m_ui->actionShowAxes, SIGNAL(toggled(bool)), m_ui->centralWidget, SLOT(setAxisIsDrawn(bool)));
	connect(m_ui->actionShowGrid, SIGNAL(toggled(bool)), m_ui->centralWidget, SLOT(setGridIsDrawn(bool)));
	connect(m_ui->actionShowLightSource, SIGNAL(toggled(bool)), m_ui->centralWidget, SLOT(setDrawLightSource(bool)));
	connect(m_ui->actionAboutQt, SIGNAL(triggered()), QApplication::instance(), SLOT(aboutQt()));

	// transformations
	connect(m_transformDock, SIGNAL(scaleXYZChanged(double)), this, SLOT(scaleXYZChanged(double)));
	connect(m_transformDock, SIGNAL(scaleXChanged(double)), this, SLOT(scaleXChanged(double)));
	connect(m_transformDock, SIGNAL(scaleYChanged(double)), this, SLOT(scaleYChanged(double)));
	connect(m_transformDock, SIGNAL(scaleZChanged(double)), this, SLOT(scaleZChanged(double)));
	connect(m_transformDock, SIGNAL(reverseWindings(int)), this, SLOT(reverseWindings(int)));
	connect(m_transformDock, SIGNAL(applyTransformations()), &m_model, SLOT(applyTransformations()));
	connect(m_transformDock, SIGNAL(changeActiveMesh(int)), &m_model, SLOT(setActiveMesh(int)));
	connect(m_transformDock, SIGNAL(removeMesh(int)), this, SLOT(removeMesh(int)));
	connect(m_transformDock, SIGNAL(mirrorAxis(int)), this, SLOT(mirrorAxis(int)));
	connect(&m_model, SIGNAL(meshCountChanged(int,QStringList)), m_transformDock, SLOT(setMeshCount(int,QStringList)));

	clear();

	// disable wip-parts
	m_ui->actionSave->setDisabled(true);
}

MainWindow::~MainWindow()
{
	delete m_ui;
}

void MainWindow::clear()
{
	m_model.clear();
	m_currentFile.clear();

	setWindowTitle(WMIT_APPNAME);

	m_ui->actionClose->setDisabled(true);
	m_ui->actionSaveAs->setDisabled(true);
	m_ui->actionSetupTextures->setDisabled(true);
	m_ui->actionAppendModel->setDisabled(true);
}

bool MainWindow::openFile(const QString &filePath)
{
	if (filePath.isEmpty())
	{
		return false;
	}

	WZM tmpmodel;

	if (loadModel(filePath, tmpmodel))
	{
		QFileInfo modelFileNfo(filePath);
		m_model = tmpmodel;
		m_currentFile = modelFileNfo.absoluteFilePath();

		setWindowTitle(QString("%1 - WMIT").arg(modelFileNfo.baseName()));
		m_ui->actionClose->setEnabled(true);
		m_ui->actionSaveAs->setEnabled(true);
		m_ui->actionSetupTextures->setEnabled(true);
		m_ui->actionAppendModel->setEnabled(true);

		if (!fireTextureDialog(true))
		{
			clear();
			return false;
		}
	}

	return true;
}

bool MainWindow::guessModelTypeFromFilename(const QString& fname, wmit_filetype_t& type)
{
	const QString ext = fname.right(fname.size() - fname.lastIndexOf('.') - 1);

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
	case WMIT_FT_PIE2:
	default:
		Pie3Model p3 = model;
		if (type == WMIT_FT_PIE2)
		{
			Pie2Model p2 = p3;
			p2.write(out);
		}
		else
		{
			p3.write(out);
		}
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
	case WMIT_FT_PIE2:
	default:
		Pie3Model p3 = model;
		if (type == WMIT_FT_PIE2)
		{
			Pie2Model p2 = p3;
			p2.write(out);
		}
		else
		{
			p3.write(out);
		}
	}

	out.close();

	return true;
}

void MainWindow::changeEvent(QEvent *event)
{
	QMainWindow::changeEvent(event);

	switch (event->type())
	{
	case QEvent::LanguageChange:
		m_ui->retranslateUi(this);
		break;
	default:
		break;
	}
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	QSettings settings;
	settings.setValue("Window/size", size());
	settings.setValue("Window/position", pos());
	settings.setValue("Window/state", saveState());

	event->accept();
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
	case WMIT_FT_PIE2:
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
		m_model.clearTextureNames();
		m_model.clearGLRenderTextures();

		m_textureDialog->getTexturesFilepath(texmap);
		for (QMap<wzm_texture_type_t, QString>::const_iterator it = texmap.begin(); it != texmap.end(); ++it)
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

void MainWindow::actionOpen()
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

		QFileInfo fileInfo(filePath);
		QStringList recentFiles = QSettings().value("recentFiles").toStringList();
		recentFiles.removeAll(fileInfo.absoluteFilePath());
		recentFiles.prepend(fileInfo.absoluteFilePath());
		recentFiles = recentFiles.mid(0, 10);

		QSettings().setValue("recentFiles", recentFiles);
	}

	delete fileDialog;
	fileDialog = 0;

	if (!filePath.isEmpty())
	{
		openFile(filePath);
		// else popup on fail?
	}
}

void MainWindow::actionOpenRecent(QAction *action)
{
	if (!action->data().toString().isEmpty())
	{
		openFile(action->data().toString());
	}
}

void MainWindow::actionClearRecentFiles()
{
	QSettings().remove("recentFiles");
}

void MainWindow::actionSave()
{
//todo
}

void MainWindow::actionSaveAs()
{
	QStringList filters;
	filters << "PIE2 models (*.pie)" << "PIE3 models (**.pie)" << "WZM models (*.wzm)" << "OBJ files (*.obj)";

	QList<wmit_filetype_t> types;
	types << WMIT_FT_PIE2 << WMIT_FT_PIE << WMIT_FT_WZM << WMIT_FT_OBJ;

	QFileDialog* fDialog = new QFileDialog();

	fDialog->setFileMode(QFileDialog::AnyFile);
	fDialog->setAcceptMode(QFileDialog::AcceptSave);
	fDialog->setFilter(filters.join(";;"));
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

	if (!filters.contains(fDialog->selectedNameFilter()))
	{
		return;
	}
	
	QFileInfo fileInfo(fDialog->selectedFiles().first());
	QStringList recentFiles = QSettings().value("recentFiles").toStringList();
	recentFiles.removeAll(fileInfo.absoluteFilePath());
	recentFiles.prepend(fileInfo.absoluteFilePath());
	recentFiles = recentFiles.mid(0, 10);

	QSettings().setValue("recentFiles", recentFiles);

/* Disabled till ready
	if (type == PIE)
	{
		m_exportDialog = new PieExportDialog(this);
		m_exportDialog->exec();
	}
	else
	{
		m_exportDialog = new ExportDialog(this);
		m_exportDialog->exec();
	}

	if (m_exportDialog->result() != QDialog::Accepted)
	{
		return;
	}

	if (m_exportDialog->optimisationSelected() == 0)
	{
//		model.optimizeForsyth();
	}
	delete m_exportDialog;
	m_exportDialog = NULL;
*/

	saveModel(fDialog->selectedFiles().first(), m_model, types[filters.indexOf(fDialog->selectedNameFilter())]);
}

void MainWindow::viewerInitialized()
{
	m_ui->centralWidget->addToRenderList(&m_model);

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
					if (m_ui->centralWidget->loadShader(static_cast<wz_shader_type_t>(i), pathvert, pathfrag))
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
		     this, SLOT(shaderAction(int)));

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

	m_ui->actionRenderer->setMenu(rendererMenu);

	connect(m_ui->actionShowModelCenter, SIGNAL(triggered(bool)),
		&m_model, SLOT(setDrawCenterPointFlag(bool)));
	connect(m_ui->actionShowNormals, SIGNAL(triggered(bool)),
		&m_model, SLOT(setDrawNormalsFlag(bool)));
}

void MainWindow::shaderAction(int type)
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

void MainWindow::scaleXYZChanged(double val)
{
	m_model.setScaleXYZ(val);
	m_ui->centralWidget->updateGL();
}

void MainWindow::scaleXChanged(double val)
{
	m_model.setScaleX(val);
	m_ui->centralWidget->updateGL();
}

void MainWindow::scaleYChanged(double val)
{
	m_model.setScaleY(val);
	m_ui->centralWidget->updateGL();
}

void MainWindow::scaleZChanged(double val)
{
	m_model.setScaleZ(val);
	m_ui->centralWidget->updateGL();
}

void MainWindow::reverseWindings(int mesh)
{
	m_model.reverseWinding(mesh);
	m_ui->centralWidget->updateGL();
}

void MainWindow::mirrorAxis(int axis)
{
	m_model.slotMirrorAxis(axis);
	m_ui->centralWidget->updateGL();
}

void MainWindow::removeMesh(int mesh)
{
	if (mesh < 0 || mesh > m_model.meshes())
		return;

	m_model.rmMesh(mesh);
	m_ui->centralWidget->updateGL();
}

void MainWindow::actionClose()
{
	clear();
}

void MainWindow::actionSetupTextures()
{
	fireTextureDialog();
}

void MainWindow::actionAppendModel()
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

void MainWindow::actionTakeScreenshot()
{
	m_ui->centralWidget->saveSnapshot(false);
}

void MainWindow::updateRecentFilesMenu()
{
	QStringList recentFiles = QSettings().value("recentFiles").toStringList();

	for (int i = 0; i < 10; ++i)
	{
		if (i < recentFiles.count())
		{
			QFileInfo fileInfo(recentFiles.at(i));

			m_ui->menuOpenRecent->actions().at(i)->setText(QString("%1. %2 (%3)").arg(i + 1).arg(fileInfo.fileName()).arg(recentFiles.at(i)));
			m_ui->menuOpenRecent->actions().at(i)->setData(recentFiles.at(i));
			m_ui->menuOpenRecent->actions().at(i)->setVisible(true);
		}
		else
		{
			m_ui->menuOpenRecent->actions().at(i)->setVisible(false);
		}
	}

	m_ui->menuOpenRecent->setEnabled(recentFiles.count());
}
