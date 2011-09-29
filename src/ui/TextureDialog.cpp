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

#include "TextureDialog.h"
#include "ui_TextureDialog.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QDir>

TextureDialog::TextureDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::TextureDialog)
{
	ui->setupUi(this);

	ui->lwTextures->setViewMode(QListView::IconMode);
	ui->lwTextures->setIconSize(QSize(128, 128));
	ui->lwTextures->setMovement(QListView::Static);
	ui->lwTextures->setFlow(QListView::LeftToRight);
	ui->lwTextures->setSpacing(10);

	ui->lwTextures->setMaximumWidth(350);

	connect(ui->lwTextures, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
		this, SLOT(iconDoubleClicked(QListWidgetItem*)));
}

TextureDialog::~TextureDialog()
{
	delete ui;
}

QString TextureDialog::selectTextureFile()
{
	static QString lastDir = m_work_dir;
	QString texture;
	QFileDialog* fileDialog = new QFileDialog(0,
						  tr("Select texture file to use"),
						  lastDir,
						  tr("WZ Compatible (*.png);;WMIT Compatible (*.bmp *.jpg *.jpeg *.png)"));
	fileDialog->setFileMode(QFileDialog::ExistingFile);
	fileDialog->exec();

	if (fileDialog->result() == QDialog::Accepted)
	{
		lastDir = fileDialog->directory().absolutePath();
		texture = fileDialog->selectedFiles().first();
	}

	delete fileDialog;
	return texture;
}

void TextureDialog::addTextureIcon(wzm_texture_type_t type)
{
	QString texpath = findTexture(type);
	if (!texpath.isEmpty())
	{
		if (m_icons.find(type) == m_icons.end())
		{
			m_icons[type] = new QListWidgetItem(ui->lwTextures);
		}
		QListWidgetItem *newicn = m_icons[type];

		newicn->setData(Qt::UserRole, QVariant(static_cast<int>(type)));
		newicn->setData(Qt::UserRole + 1, texpath);

		QPixmap img(texpath);
		QString ttp = "Path: " + texpath + "\nWidth: " + QString::number(img.width())+ ", height: " + QString::number(img.height());
		newicn->setToolTip(ttp);
		newicn->setIcon(QIcon(img));
		newicn->setText(QString::fromStdString(WZM::texTypeToString(type)));
		newicn->setTextAlignment(Qt::AlignHCenter);
		newicn->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	}
}

void TextureDialog::createTextureIcons(const QString& workdir, const QString& modelname)
{
	ui->lwTextures->clear();
	m_icons.clear();

	m_work_dir = workdir;
	m_model_filepath = modelname;

	if (workdir.isEmpty() || modelname.isEmpty() || !m_texnames.size())
		return;

	QMap<wzm_texture_type_t, QString>::const_iterator it;
	for (it = m_texnames.begin(); it != m_texnames.end(); ++it)
	{
		addTextureIcon(it.key());
	}

	ui->lwTextures->setCurrentRow(0);
}

void TextureDialog::setSearchDirs(const QStringList &list)
{
	scanForTexturesInDirs(list);
}

void TextureDialog::setTexturesMap(const QMap<wzm_texture_type_t, QString> &texnames)
{
	m_texnames = texnames;
}

void TextureDialog::getTexturesFilepath(QMap<wzm_texture_type_t, QString> &files) const
{
	files.clear();

	for (int i = 0; i < ui->lwTextures->count(); ++i)
	{
		QListWidgetItem *itm = ui->lwTextures->item(i);
		if (itm)
		{
			files.insert(static_cast<wzm_texture_type_t>(itm->data(Qt::UserRole).toInt()),
				     itm->data(Qt::UserRole + 1).toString());
		}
	}
}

QString TextureDialog::findTexture(wzm_texture_type_t type) const
{
	QFileInfo modelnfo(m_model_filepath);
	QString fileTexName = m_texnames.value(type);

	if (!fileTexName.isEmpty())
	{
		// Pre-configured search
		foreach(QString filePath, m_predefined_textures)
		{
			if (fileTexName.compare(QFileInfo(filePath).fileName(), Qt::CaseSensitive) == 0)
			{
				return filePath;
			}
		}

		// Local search
		QFileInfo nfo(modelnfo.path() + "/" + fileTexName);
		if (nfo.exists())
		{
			return nfo.filePath();
		}
	}

	// Wild guess for Diffuse type
	if (type == WZM_TEX_DIFFUSE)
	{
		QFileInfo nfo(modelnfo.absolutePath() + "/" + modelnfo.completeBaseName() + ".png");
		if (nfo.exists())
			return nfo.filePath();
	}
	return QString();
}

void TextureDialog::scanForTexturesInDirs(const QStringList& dirs)
{
	m_predefined_textures.clear();
	foreach (QString path, dirs)
	{
		QDir dir(path, "*.png", QDir::Type, QDir::Files);
		QStringList textures = dir.entryList();
		foreach (QString texture, textures)
		{
			m_predefined_textures.append(dir.absoluteFilePath(texture));
		}
	}
}

void TextureDialog::iconDoubleClicked(QListWidgetItem *icon)
{
	QString newtex = selectTextureFile();
	if (!newtex.isEmpty())
	{
		icon->setData(Qt::UserRole + 1, newtex);

		QPixmap img(newtex);
		QString ttp = "Path: " + newtex + "\nWidth: " + QString::number(img.width())+ ", height: " + QString::number(img.height());
		icon->setToolTip(ttp);
		icon->setIcon(QIcon(img));
	}
}
